/*
 * Copyright (C) 2008 Philippe Pepiot <philippe.pepiot@gmail.com>
 * philsh is under BSD licence, see LICENCE file for more informations.
 *
 */ 

/* on inclu les headers */
#include "headers.h"

/* {{{ variables globales */
/* le chemin de l'executable par exemple /bin/echo */
char *chemin;

/* Structure des builtin commandes,
 * char *name;
 * int (*p)(int, char**);
 */
const struct builtin builtin_command[] =
{
   {"pwd", internal_pwd, SAME_PROCESS},
   {"which", internal_which, NEW_PROCESS},
   {"uname", internal_uname, NEW_PROCESS},
   {"env", internal_env, NEW_PROCESS},
   {"cd", internal_cd, SAME_PROCESS},
   {"whoami", whoami, SAME_PROCESS},
   {"alias", alias, SAME_PROCESS},
   {"unalias", unalias, SAME_PROCESS},
   {"source", source, SAME_PROCESS},
   {"exit", exit_philsh, SAME_PROCESS},
   {"jobs", jobs, SAME_PROCESS},
   {"help", help, SAME_PROCESS},
   {NULL, NULL, NEW_PROCESS}
};
/* }}} */

/* {{{ init_env */
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
      setenv("PATH", "/usr/local/bin:/usr/bin:/bin:/opt/bin:/sbin", 0);
   return;
}
/* }}} */

/* {{{ main() */
/* Ici commence l'aventure */
int main (int argc, char **argv)
{
   char *config_file, *saisie = NULL, *prompt;
   uid_t uid = getuid();
   struct passwd *user = getpwuid(uid);
   file_instruction *liste_instruction = NULL;
   int ret = 0;
   /* On teste les options avec lesquelles
    * philsh est appellé */
   options_philsh(argc, argv);
   /* On initialise l'environnement */
   init_env();
   /* Initialisation de la config */
   init_config("/etc/philsh/philshrc");
   config_file = malloc(sizeof(char) * (15+strlen(user->pw_dir)));
   sprintf(config_file, "%s/%s", user->pw_dir, ".philshrc");
   init_config(config_file);
   free(config_file);
   /* La super boucle du phil shell */
   for (;;)
   {
      prompt = set_prompt(ret);
      /* La fonction readline est magique */
      saisie = readline(prompt);
      free(prompt);
      if (saisie && *saisie)
	 add_history(saisie);
      /* ret = exec_saisie(saisie); */
      liste_instruction = creat_liste_instruction(saisie);
      ret = exec_file(liste_instruction);
#ifdef DEBUG
      afficher_liste_instruction(liste_instruction);
#endif
      free_file_instruction(liste_instruction);
      free(saisie);
   }
   return 0;
}
/* }}} */

/* {{{ options_philsh */
int options_philsh(int argc, char **argv)
{
   file_instruction *cmd;
   int ret;
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
	 if(argc < 3)
	    exit(0);
	 cmd = Translate(argc-2, argv+2);
	 ret = exec_file(cmd);
	 free(cmd);
	 exit(ret);
      }
   }
   return 0;
}

/* }}} */

/* {{{ afficher_aide */

void afficher_aide(void)
{

   printf("PHILSH version "PHILSH_VERSION"\n\
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
	 Philsh comporte des built-in commandes : "PHILSH_BUILTIN" ; ls n'y est pas encore (il était présent\n\
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
	 Philippe Pepiot alias philpep "PHILSH_MAIL"\n\
	 rhaamo <markocpc@gmail.com>\n\
	 <"PHILSH_HTTP">\n\
	 \n\n\n");
   return;
}

/* }}} */
/* vim:fdm=marker:
 */
