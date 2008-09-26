#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <sys/utsname.h>
#include "../philsh_build.h"

int internal_uname(int argc, char **argv)
{
   struct utsname localhost;
   uname(&localhost);
   const char *help_message = "Usage: uname [OPTION]... \n \
			       Afficher certaines informations identifiant le système. \n \
			       Sans OPTION, identique à -s. \n \
			       \n \
			       -a, --all                afficher toutes les informations \n \
			       -s, --kernel-name        afficher le nom du kernel \n \
			       -n, --nodename           afficher le nom du noeud réseau du poste (hostname) \n \
			       -r, --release            afficher la révision de la version du \n \
			       système d'exploitation \n \
			       -v, --kernel-version     afficher la version du kernel \n \
			       -m, --machine            afficher le nom du système d'exploitation \n \
			       -h, --help      afficher l'aide-mémoire \n \
			       -V, --version   afficher le nom et la version du logiciel \n \
			       \n \
			       Signaler un bug à";
   const char *optstring = "hVasnrvm";
   static struct option uname_options[] = 
   {
      {"help", 0, NULL, 'h'},
      {"version", 0, NULL, 'V'},
      {"all", 0, NULL, 'a'},
      {"kernel-name", 0, NULL, 's'},
      {"nodename", 0, NULL, 'n'},
      {"kernel-release", 0, NULL, 'r'},
      {"kernel-version", 0, NULL, 'v'},
      {"machine", 0, NULL, 'm'},
      {0, 0, 0, 0}
   };
   char *s = NULL, *n = NULL, *r = NULL, *v = NULL, *m = NULL;
   char opt;
   while(EOF != (opt = (char)getopt_long(argc, argv, optstring, uname_options, NULL)))
   {
      if((opt == 'h')||(opt == '?'))
      {
	 printf("%s %s\n", help_message, PHILSH_MAIL);
	 return 0;
      }
      else if (opt == 'V')
      {
	 printf("Uname %s, version pour philsh\nSi vous voulez utiliser \
	       le traditionnel uname executez _uname\n\
	       Bugs, commentaires à %s\n", PHILSH_VERSION, PHILSH_MAIL);
	 return 0;
      }
      else if(opt == 'a')
      {
	 s = localhost.sysname;
	 n = localhost.nodename;
	 r = localhost.release;
	 v = localhost.version;
	 m = localhost.machine;
	 break;
      }
      else if(opt == 's')
	 s = localhost.sysname;
      else if(opt == 'n')
	 n = localhost.nodename;
      else if(opt == 'r')
	 r = localhost.release;
      else if(opt == 'v')
	 v = localhost.version;
      else if(opt == 'm')
	 m = localhost.machine;
   }
   if ((s)||((!n)&&(!r)&&(!v)&&(!m)))
      printf("%s ", localhost.sysname);
   if (n)
      printf("%s ", n);
   if (r)
      printf("%s ", r);
   if (v)
      printf("%s ", v);
   if (m)
      printf("%s ", m);
   printf("\n");
   return 0;
}
