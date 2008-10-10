/*
 * Copyright (C) 2008 Philippe Pepiot <philippe.pepiot@gmail.com>
 * philsh is under BSD licence, see LICENCE file for more informations.
 *
 */ 

/* on inclu les headers */
#include "headers.h"

/* {{{ variables globales */

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
   rl_catch_signals = 0;
   rl_set_signals();
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
/* vim:fdm=marker:
 */
