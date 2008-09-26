#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <pwd.h>
#include <sys/types.h>
#include "internal.h"


/* Fonction simple */
/* TODO : gestion des liens symboliques */
int internal_cd(int argc, char **argv)
{
   uid_t uid = getuid();
   struct passwd *user = getpwuid(uid);
   char *name;
   char *p;
   char *chemin;
   char *oldpwd;
   int err;
   int ret;
   if (argc == 1)
   {
      chemin = malloc(sizeof(char) * (1+strlen(user->pw_dir)));
      assert(chemin != NULL);
      strcpy(chemin, user->pw_dir);
   }
   else if ((argc == 2)&&(argv[1][0] == '~'))
   {
      if((argv[1][1] == '\0')||(argv[1][1] == '/'))
      {
	 chemin = malloc(sizeof(char) * (strlen(user->pw_dir)+strlen(argv[1])));
	 assert(chemin != NULL);
	 sprintf(chemin, "%s%s", user->pw_dir, argv[1]+1);
      }
      else
      {
	 p = strchr(argv[1], '/');
	 if (p == NULL)
	 {
	    name = malloc(sizeof(char) * (1+strlen(argv[1]+1)));
	    assert(name != NULL);
	    strcpy(name, argv[1]+1);
	 }
	 else
	 {
	    name = malloc(sizeof(char) * (p-argv[1]));
	    memcpy(name, argv[1]+1, p-argv[1]-1);
	    name[p-argv[1]-1] = '\0';
	 }
	 if(NULL == (user = getpwnam(name)))
	 {
	    fprintf(stderr, "Philsh: aucun utilisateur de ce nom : %s\n", name);
	    free(name);
	    return -1;
	 }
	 free(name);
	 if (p == NULL)
	 {
	    chemin = malloc(sizeof(char) * (1+strlen(user->pw_dir)));
	    assert(chemin != NULL);
	    strcpy(chemin, user->pw_dir);
	 }
	 else
	 {
	    chemin = malloc(sizeof(char) * (1+strlen(user->pw_dir)+strlen(p)));
	    assert(chemin != NULL);
	    sprintf(chemin, "%s%s", user->pw_dir, p);
	 }
      }
   }
   else
   {
      chemin = malloc(sizeof(char) * (1+strlen(argv[1])));
      assert(chemin != NULL);
      strcpy(chemin, argv[1]);
   }
   oldpwd = get_current_dir();
   setenv("OLDPWD", oldpwd, 1);
   free(oldpwd);
   ret = chdir(chemin);
   err = errno;
   switch (err)
   {
      case EACCES:
	 fprintf(stderr, "Philsh: %s permission non accordée\n", chemin);
	 break;
      case EIO:
	 fprintf(stderr, "Philsh: %s erreur d'entrée/sortie\n", chemin);
	 break;
      case ENAMETOOLONG:
	 fprintf(stderr, "Philsh: %s path trop long\n", chemin);
	 break;
      case ENOENT:
	 fprintf(stderr, "Philsh: aucun fichier ou dossier de ce type : %s\n", chemin);
	 break;
      case EFAULT:
	 fprintf(stderr, "Philsh: %s pointe en dehors de l'espace d'adressage accessible\n", chemin);
	 break;
   }
   free(chemin);
   return ret;
}

