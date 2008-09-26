#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include "internal.h"

/* Commande qui va chercher le repertoire
 * absolu dans lequel on est.
 * NE PAS OUBLIER DE LIBERER LA MEMOIRE */
/* TODO : gestion des liens symboliques */
char * get_current_dir(void)
{
   char *p = get_current_dir_name();
   setenv("PWD", p, 1);
   return p;
}

/* TODO : gestion des liens symboliques,
 * mais garder une option qui permette d'avoir
 * le chemin absolu */
int internal_pwd(int argc, char **argv)
{
   if (argc > 1)
   {
      fprintf(stderr, "Philsh : Option inconue : %s\n", argv[1]);
      return -1;
   }
   char *current_dir;
   current_dir = get_current_dir();
   assert(current_dir != NULL);
   printf("%s\n", current_dir);
   free(current_dir);
   return 0;
}

