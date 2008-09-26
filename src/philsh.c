/* 
 * philsh.c
 *
 * Copyright (C) 2008 Philippe Pepiot <philippe.pepiot@gmail.com>
 * See LICENCE file for licence of this piece of software.
 *
 */

/* on inclu les headers */
#include "headers.h"

/* le chemin de l'executable par exemple /bin/echo */
char *chemin;

/* Structure des builtin commandes,
 * char *name;
 * int (*p)(int, char**);
 */
const struct builtin builtin_command[] =
{
   {"pwd", internal_pwd},
   {"which", internal_which},
   {"uname", internal_uname},
   {"env", internal_env},
   {"cd", internal_cd},
   {"whoami", whoami},
   {"alias", alias},
   {NULL, NULL}
};

/* Cette fonction initialise l'environnement
 * Si $HOME et $PATH n'existent pas, il faut
 * les créer... En effet, si on lance le programme
 * comme ceci : env -i philsh, tous les getenv()
 * retournent NULL */
void init_env(void)
{
   /* L'uid sous lequel tourne philsh */
   uid_t uid = getuid();
   struct passwd *user;
   /* On recupère l'entrée correspondante
    * dans /etc/passwd */
   user = getpwuid(uid);
   /* On met tout ça dans l'environnement */
   setenv("HOME", user->pw_dir, 0);
   /* Si le PATH n'est pas fixé, il est impossible de le deviner
    * Dans ce cas on utilise un PATH classique UNIX */
   if (getenv("PATH") == NULL)
      putenv("PATH=/usr/local/bin:/usr/bin:/bin:/opt/bin:/sbin");
   return;
}

/* Cette fonction decoupe la saisie suivant les
 * espaces ou les guillemets, elle alloue le tout dans argv
 * Elle renvoie le dernier entier alloué (on s'en sert pour bien
 * liberer la memoire ensuite) */
int parse_saisie(char *saisie, size_t buf_size, char **argv)
{
   char *p;
   p = saisie;
   char *buffer;
   int gui = 0;
   size_t i = 0, j = 0;
   buffer = malloc(sizeof(char) * buf_size+2);
   assert(buffer != NULL);
   while(*p == ' ')
      p++;
   for(;*p != '\0'; p++)
   {
      assert(i < buf_size+2);
      if(*p == '"')
      {
	 gui = (gui) ? 0 : 1;
	 continue;
      }
      if((*p == ' ')&&(!gui))
      {
	 buffer[i] = '\0';
	 argv[j] = malloc(sizeof(char) * (1+strlen(buffer)));
	 assert(argv[j] != NULL);
	 strcpy(argv[j], buffer);
#ifdef DEBUG
	 printf("argv[%d] = '%s'\n", j, argv[j]);
#endif
	 i = 0;
	 j++;
	 while(*p == ' ')
	    p++;
	 p--;
	 continue;
      }
      buffer[i] = *p;
      i++;
   }
   buffer[i] = '\0';
   argv[j] = malloc(sizeof(char) * (1+strlen(buffer)));
   assert(argv[j] != NULL);
   strcpy(argv[j], buffer);
#ifdef DEBUG
   printf("argv[%d] = '%s'\n", j, argv[j]);
#endif
   free(buffer);
   return j;
}

/* Cette fonction compte les mots dans saisie
 * en prenant en compte les espaces et les guillemets,
 * elle en profite pour rentrer la taille du mot le plus
 * long dans lenght (pour fixer la taille du buffer ensuite...)
 */
int compter_mots(char *saisie, size_t *lenght)
{
   char *p = strchr(saisie, '\0');
   int words = 0;
   size_t lenght_current = 0;
   size_t lenght_max = 0;
   int gui = 0;
   p--;
   while(*p == ' ')
   {
      *p = '\0';
      p--;
   }
   p = saisie;
   while(*p == ' ')
      p++;
   if(*p != '\0')
      words++;
   for(;*p != '\0';p++)
   {
      if(*p == '"')
      {
	 gui = (gui) ? 0 : 1;
	 continue;
      }
      if((*p == ' ')&&(!gui))
      {
	 if (lenght_current > lenght_max)
	    lenght_max = lenght_current;
	 lenght_current = 0;
	 words++;
	 while(*p == ' ')
	    p++;
	 p--;
	 continue;
      }
      lenght_current++;
   }
   if (lenght_current > lenght_max)
      lenght_max = lenght_current;
   *lenght = lenght_max;
   return words;
}


int exec_saisie(char *saisie)
{
   int i;
   int argc;
   char **argv;
   size_t buf_size;
   char *p, *buffer = NULL;
   char c;
   extern alias_ll *liste_alias;
   alias_ll *alias;
   alias = liste_alias;
   p = saisie;
   argc = compter_mots(saisie, &buf_size);
   /* Si la saisie est vide de sens */
   if (argc == 0)
      return 0;
   while (*p == ' ')
      p++;
   while(alias != NULL)
   {
      if (!strncmp(alias->name, p, strlen(alias->name)))
      {
	 c = *(p+strlen(alias->name));
	 if ((c != '\0')&&(c != ' '))
	 {
	    alias = alias->next;
	    continue;
	 }
	 buffer = malloc(sizeof(char) * (1+strlen(alias->cmd)+strlen(p+strlen(alias->name))));
	 sprintf(buffer, "%s%s", alias->cmd, p+strlen(alias->name));
	 break;
      }
      alias = alias->next;
   }
   if(buffer == NULL)
      buffer = saisie;
   argc = compter_mots(buffer, &buf_size);
   /* C'est honteux, mais en attendant le support des
    * caractères speciaux, on utilise system()
    * Histoire que le shell donne l'impression
    * de bien marcher
    * NOTE : L'idée honteuse n'est pas de moi,
    * en lisant le code de sash (app-shell/sash sur gentoo)
    * je me suis rendu compte qu'il utilisait system() a tout-vat,
    * du coup je me dis pourquoi pas ? C'est bien sûr temporaire !
    */
   if(strchr(buffer, '|')||strchr(buffer, '*')||strchr(buffer, '>')||strchr(buffer, '<')||strstr(buffer, "&&")||strchr(buffer, '`')||strchr(buffer, '$')||strchr(buffer, '\\'))
   {
      i = system(buffer);
      if (buffer != saisie)
	 free(buffer);
      return i;
   }
   argv = malloc (sizeof(char *) * (argc+1));
   argc = parse_saisie(buffer, buf_size, argv);
   if (buffer != saisie)
      free(buffer);
   argc++;
   argv[argc] = NULL;
   exec_cmd(argc, argv);
   for (i = 0; i < argc; i++)
      free(argv[i]);
   free(argv);
   return 0;
}

/* Ici commence l'aventure */
int main (int argc, char **argv)
{
   /* On teste les options avec lesquelles
    * philsh est appellé */
   options_philsh(argc, argv);
   /* On initialise l'environnement */
   init_env();
   char *config_file;
   char *saisie = NULL;
   char *prompt;
   /* Initialisation de la config */
   uid_t uid = getuid();
   struct passwd *user = getpwuid(uid);
   config_file = malloc(sizeof(char) * (15+strlen(user->pw_dir)));
   sprintf(config_file, "%s/%s", user->pw_dir, ".philshrc");
   init_config(config_file);
   /* La super boucle du phil shell */
   for (;;)
   {
      prompt = set_prompt();
      /* La fonction readline est magique */
      saisie = readline(prompt);
      free(prompt);
      if (saisie && *saisie)
	 add_history(saisie);
      exec_saisie(saisie);
      free(saisie);
   }
   return 0;
}


int options_philsh(int argc, char **argv)
{
   const char *optstring = "hvc";
   static struct option philsh_option[] =
   {
      {"help", 0, NULL, 'h'},
      {"version", 0, NULL, 'v'},
      {"execute", 0, NULL, 'c'},
      {0, 0, 0, 0}
   };
   char opt;
   while (EOF != (opt = getopt_long(argc, argv, optstring, philsh_option, NULL)))
   {
      if (opt == 'v')
      {
	 printf("Philsh version : "PHILSH_VERSION".\n"
	       " Compilation settings :\n"
	       "  - Flags : "PHILSH_COMPILE_FLAGS"\n"
	       "  - Linked Libs : "PHILSH_LINKED_LIBS"\n"
	       "  - On "PHILSH_COMPILE_MACHINE" by "PHILSH_COMPILE_BY"\n"
	       " Main contact : "PHILSH_MAIL"\n");
	 exit(0);
      }
      else if(opt == 'h')
      {
	 afficher_aide();
	 exit(0);
      }
      else if(opt == 'c')
      {
	 exec_cmd(argc-2, argv+2);
	 exit(0);
      }
   }
   return 0;
}

void afficher_aide(void)
{

   printf("PHILSH version %s\n\
	 philsh [OPTION]...\n\
	 -c, --execute commande (arguments)   Executer la commande puis quitter\n\
	 -h, --help                           afficher l'aide\n\
	 -v, --version                        afficher le nom et la version du logiciel\n\
	 \n\
	 <[HISTORIQUE]>\n\
	 Philsh est un shell minimaliste, écrit par Philippe Pepiot,\n\
	 étudiant en 3ème année de Mathématiques à Toulouse,\n\
	 qui à décidé d'apprendre le C sur internet et avec les pages de manuel.\n\
	 Philippe avait besoin de mettre en pratique ses maigres connaissances en C\n\
	 donc il s'est lancé dans le projet de faire un shell inutile, mais marrant.\n\
	 Le but premier est de coder, n'importe quoi pourvu que ce soit (marrant|instructif)\n\
	 Plus tard, il montre son code à rhaamo qui lui confectionne un systeme de compilation\n\
	 plus performant avec cmake. rhaamo lui apprend aussi les joie de git, et lui\n\
	 donne un depot git (git://tux-atome.fr/philsh.git)\n\
	 \n\
	 <[FONCTIONALITÉES]>\n\
	 Philsh fonctionne comme n'importe quel shell (zsh|bash|sh), mais avec quelques\n\
	 problèmes au niveau de l'interprétation des caractères suivants : '$>|* , le \" est\n\
	 partièlement géré.\n\
	 En gros ça veut dire que les redirections (>, <, >> et <<) ne fonctionnent pas encore\n\
	 et que les pipes (|) non plus. Les conditions (&& et ||) ne sont pas gérées non plus.\n\
	 Le remplacement des variables d'environnements ($) par leurs valeurs et la gestion de (*)\n\
	 ne marchent pas mais c'est un des objectifs prioritaires de philsh.\n\
	 Par contre depuis la version 0.2, la mise en backgroud des procéssus avec & fonctionne\n\
	 et la gestion des signaux fonctionne partiellement\n\
	 Philsh comporte des built-in commandes : %s ; ls n'y est pas encore (il était présent\n\
	       dans la version 0.1 mais le code n'était pas de moi donc ça compte pas :-)\n\
	 Il n'y a pas non plus de boucles possible (if, then, else, fi, for, do, done, while)\n\
	 Philsh à pour but la complexité, donc tout (le maximum possible) est alloué dynamiquement\n\
	 et philsh est surement le shell le plus léger en memoire (c'est la classe de dire ça ^^)\n\
	 \n\
	 <[BUGS]> pardon <[FONCTIONALITÉES SPÉCIALES]>\n\
	 Plus philsh commence à grossir, plus il y à de bugs et plus les defauts de conception\n\
	 se font sentirs. Je pense à une reécriture complete du code pour plus de lisibilité\n\
	 et pour plus de flexibilitée des fonctions...\n\
	 Sur certains systèmes, philsh n'arrive pas à s'executer (segment fault), donc si vous\n\
	 ètes dans ce cas, contactez moi (addresse plus bas) pour que je comprenne ou se trouve le problème car sur\n\
	 d'autres systèmes, tout marche bien...\n\
	 \033[36mLa commande cd comporte le bug suivant : quand vous vous déplacez sur un lien\n\
	 symbolique, il croit que vous ètes dans le path absolu, du coup un petit cd .. et vous\n\
	 vous retrouvez dans le repertoire parent du path absolu et non dans le path de départ\n\
	 \n\
	 Sur les petits terminaux, si la commande est trop longue, elle ecrase le prompt puis\n\
	 le début de la commande, je pense que c'est un réglage à faire dans readline...\n\
	 \n\
	 La suspenssion d'un processus avec ctrl-z ne marche pas, c'est philsh qui est suspendu...\n\033[37m\
	 \n\
	 \n\
	 <[AIDER PHILSH]>\n\
	 La programmation de philsh est ouverte à tous, faites vous connaitre et rhaamo pourra\n\
	 décider de vous donner accées au modifications sur le depot git.\n\
	 Pour avoir la dernière version de philsh : git clone git://tux-atome.fr/philsh.git\n\
	 \n\
	 \n\
	 <[CONTACT]>\n\
	 Vous pouvez contacter les auteurs par mail :\n\
	 Philippe Pepiot alias philpep %s\n\
	 rhaamo <markocpc@gmail.com>\n\
	 <%s>\n\
	 \n\n\n", PHILSH_VERSION, PHILSH_BUILTIN, PHILSH_MAIL, PHILSH_HTTP);
   return;
}

