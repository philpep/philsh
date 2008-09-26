/* Affiche simplement le nom de l'utilisateur... */
#include <stdio.h>
#include <unistd.h>
#include <pwd.h>
#include <getopt.h>
#include <sys/types.h>
#include "../philsh_build.h"

int whoami(int argc, char **argv)
{
   const char *optstrings = "hv";
   static struct option whoami_options[] =
   {
      {"help", 0, NULL, 'h'},
      {"version", 0, NULL, 'v'},
      {0, 0, 0, 0}
   };
   char c;
   while(EOF != (c = getopt_long(argc, argv, optstrings, whoami_options, NULL)))
   {
      if ((c == '?')||(c == 'h'))
      {
	 printf("whoami (-h|-v)\n\n\n\
	       Affiche simplement le login de l'utilisateur...\n\
	       --help, -h     : Affiche cette aide\n\
	       --version, -v  : Affiche la version du logiciel\n\
	       Rapporter des bugs à %s\n", PHILSH_MAIL);
	 return 0;
      }
      if (c == 'v')
      {
	 printf("whoami, version %s pour Philsh\nRapporter des bugs à %s\n", PHILSH_VERSION, PHILSH_MAIL);
	 return 0;
      }
   }
   uid_t uid;
   struct passwd *user;
   uid = getuid();
   if(NULL == (user = getpwuid(uid)))
   {
      fprintf(stderr, "Philsh : Impossible d'obtenir le nom de l'utilisateur\n");
      return -1;
   }
   printf("%s\n", user->pw_name);
   return 0;
}
