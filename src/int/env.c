#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include "internal.h"

int setenv (const char *name, const char *value, int overwrite);
int exec_cmd_external(char **argv);

/* Cette fonction doit absolument recevoir une chaine du type
 * VAR=VALEUR */
int internal_setenv(char *str)
{
   int ret;
   char *p = strchr(str, '=');
   assert(p != NULL);
   *p = '\0';
   ret = setenv(str, p+1, 1);
   if (errno == EINVAL)
      fprintf(stderr, "Philsh: '%s' ne doit pas contenir de caractère '='", str);
   return ret;
}

/* TODO : c'est une ébauche */
int internal_env(int argc, char **argv)
{
   if (argc < 2)
   {
      extern char **environ;
      int i = 0;
      char *p;
      char *str;
      for (i = 0; environ[i] != NULL; i++)
      {
	 p = strchr(environ[i], '=');
	 assert(p != NULL);
	 str = malloc(sizeof(char) * (p-environ[i]+1));
	 assert(str != NULL);
	 memcpy(str, environ[i], p-environ[i]);
	 str[p-environ[i]] = '\0';
	 printf("\033[34m%s\033[37m%s\n", str, p);
	 free(str);
      }
      return 0;
   }
   return exec_cmd_external(argv+1);
}

