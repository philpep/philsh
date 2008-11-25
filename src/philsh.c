/*
 * Copyright (C) 2008 Philippe Pepiot <philippe.pepiot@gmail.com>
 * philsh is under BSD licence, see LICENCE file for more informations.
 *
 */ 

/* on inclu les headers */
#include "headers.h"

/* Structure des builtin commandes,
 * char *name;
 * int (*p)(int, char**);
 */
/* {{{ variables globales */
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

/* Cette fonction initialise l'environnement
 * Si $HOME et $PATH n'existent pas, il faut
 * les créer... En effet, si on lance le programme
 * comme ceci : env -i philsh, tous les getenv()
 * retournent NULL */
/* TODO : tester si l'environnement est
 * accessible */
/* {{{ init_env */
void init_env(void)
{
   /* L'uid sous lequel tourne philsh */
   uid_t uid = getuid();
   struct passwd *user;
   struct utsname host;
   uname(&host);
   /* On recupère l'entrée correspondante
    * dans /etc/passwd */
   user = getpwuid(uid);
   /* On met tout ça dans l'environnement */
   setenv("HOME", user->pw_dir, 0);
   /* Si le PATH n'est pas fixé, il est impossible de le deviner
    * Dans ce cas on utilise un PATH classique UNIX */
   if (NULL == getenv("PATH"))
   {
      /* PATH optimised for FreeBSD */
      if(!strcmp(host.sysname, "FreeBSD"))
	 setenv("PATH", "/usr/local/bin:/usr/local/sbin:/usr/games/bin:/usr/bin:/usr/sbin:/bin:/sbin:/usr/games", 0);
      else
	 /* Ce PATH devrait marcher de façon minimale
	  * pour la plupart des OS basés sur UNIX */
	 setenv("PATH", "/usr/local/bin:/usr/bin:/bin:/opt/bin:/sbin", 0);
      fprintf(stderr,"Philsh: WARNING, PATH variable don't exist\nSo we will use this one : %s\n", getenv("PATH"));
   }
   return;
}
/* }}} */

/* mode_raw permet de switcher entre
 * le mode raw (1) et le mode cooked (0) */
/* {{{ mode_raw */
void mode_raw(int activer)
{
   static struct termios cooked;
   static int raw_actif = 0;

   if (raw_actif == activer)
      return;

   if (activer)
   {
      struct termios raw;

      tcgetattr(STDIN_FILENO, &cooked);

      raw = cooked;
      cfmakeraw(&raw);
      tcsetattr(STDIN_FILENO, TCSANOW, &raw);
   }
   else
      tcsetattr(STDIN_FILENO, TCSANOW, &cooked);

   raw_actif = activer;
}
/* }}} */

/* La boucle philsh() */
/* {{{ philsh() */
void philsh(void)
{
   char *prompt, *saisie, c, d = -1, *completion;
   size_t i;
   int ret = 0, f[2];
   unsigned int flags = NOVERBOSE;
   file_instruction *liste_instruction = NULL;
   philsh_history *history = NULL, *p_history = NULL;
   saisie = malloc(sizeof(char) * SIZE_SAISIE);
   for(;;)
   {
      i = 0;
      /* On affiche le prompt */
      prompt = set_prompt(ret);
      printf("%s", prompt);
      fflush(stdin);
      /* On recupère la saisie */
      while(i < SIZE_SAISIE)
      {
	 mode_raw(1);
	 c = getchar();
	 mode_raw(0);
	 if(c == PHILSH_KEY_TAB && d == PHILSH_KEY_TAB)
	    flags = VERBOSE;
	 else
	 {
	    d = c;
	    flags = NOVERBOSE;
	 }

	 /* fin de saisie ou ctrl+c
	  * ou ctrl+z */
	 if(c == PHILSH_KEY_ENTER || c == PHILSH_KEY_CTRLZ || c == PHILSH_KEY_CTRLC)
	 {
	    printf("\n");
	    break;
	 }
	 /* ctrl+d --> exit */
	 if(c == PHILSH_KEY_CTRLD)
	    exit(1);
	 /* TAB */
	 if (c == PHILSH_KEY_TAB)
	 {
	    saisie[i] = '\0';
	    completion = try_complete(saisie, prompt);
	    if(completion != NULL)
	    {
	       if(strlen(completion) < SIZE_SAISIE-i)
	       {
		  strcat(saisie, completion);
		  printf("%s", completion);
		  i += strlen(completion);
	       }
	       free(completion);
	    }
	    continue;
	 }
	 /* BACK */
	 if(c == PHILSH_KEY_BACK)
	 {
	    if(i == 0)
	       continue;
	    printf("\b \b");
	    i--;
	    continue;
	 }
	 /* Special keys */
	 if(c == PHILSH_KEY_SPECIALS)
	 {
	    getchar();
	    switch (getchar())
	    {
	       case PHILSH_KEY_UP:
		  if(p_history != NULL)
		  {
		     clear_saisie();
		     strcpy(saisie, p_history->cmd);
		     printf("%s", saisie);
		     p_history = p_history->next;
		     i = strlen(saisie);
		     continue;
		  }
		  break;
	       case PHILSH_KEY_DOWN:
		  /* TODO */
		  break;
	       default:
		  break;
	    }
	    continue;
	 }
	 saisie[i++] = c;
	 printf("%c", c);
      }
      mode_raw(0);
      saisie[i] = '\0';
      fflush(stdin);
      f[0] = dup(1);
      f[1] = dup(0);
      history = add_to_history(saisie, history);
      p_history = history;
      if(!strcmp(saisie, "history"))
      {
	 print_history(history);
	 continue;
      }
      liste_instruction = creat_liste_instruction(saisie);
      ret = exec_file(liste_instruction);
#ifdef DEBUG
      afficher_liste_instruction(liste_instruction);
#endif
      close(1);
      dup(f[0]);
      close(0);
      dup(f[1]);
      free_file_instruction(liste_instruction);
   }
   free(saisie);
   free_history(history);
   return;
}
/* }}} */

/* Ici commence l'aventure */
/* {{{ main() */
int main (int argc, char **argv)
{
   char *config_file;
   uid_t uid = getuid();
   struct passwd *user = getpwuid(uid);
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
   init_command_name();
   philsh();
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
	 cmd = creat_liste_instruction(argv[2]);
	 ret = exec_file(cmd);
#ifdef DEBUG
	 afficher_liste_instruction(cmd);
#endif
	 free_file_instruction(cmd);
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
	 man philsh pour voir la page de manuel\n\
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
	 <[CONTACT]>\n\
	 Vous pouvez contacter les auteurs par mail :\n\
	 Philippe Pepiot alias philpep "PHILSH_MAIL"\n\
	 rhaamo <markocpc@gmail.com>\n\
	 <"PHILSH_HTTP">\n\
	 \n\n\n");
   return;
}

/* }}} */


/* {{{ Gestion de la liste doublement chainnée de
 * l'historique */
philsh_history *add_to_history(char *cmd, philsh_history *liste)
{
   philsh_history *new;
   if(cmd == NULL || cmd[0] == '\0')
      return liste;
   new = malloc(sizeof(philsh_history));
   new->cmd = malloc(sizeof(char) * (1+strlen(cmd)));
   strcpy(new->cmd, cmd);
   new->next = liste;
   if(liste != NULL)
      liste->prev = new;
   new->prev = NULL;
   return new;
}

void free_history(philsh_history *liste)
{
   philsh_history *p, *q;
   p = liste;
   while(p != NULL)
   {
      free(p->cmd);
      q = p;
      p = p->next;
      free(q);
   }
   return;
}

void print_history(philsh_history *liste)
{
   philsh_history *p = liste;
   size_t i = 0;
   while(p != NULL)
   {
      printf("%d : %s\n", ++i, p->cmd);
      p = p->next;
   }
   return;
}
/* }}} */
/* vim:fdm=marker:
*/
