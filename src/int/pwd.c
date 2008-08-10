#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include "internal.h"

int setenv (const char *name, const char *value, int overwrite);
/* Commande qui va chercher le repertoire
 * absolu dans lequel on est.
 * NE PAS OUBLIER DE LIBERER LA MEMOIRE */
/* TODO : gestion des liens symboliques */
char * get_current_dir(void)
{
	int taille = 128;
	char * buffer = NULL;
	for(;;)
	{
		buffer = malloc (sizeof(char) * taille);
		assert(buffer != NULL);
		if (getcwd(buffer, taille) == NULL)
		{
			free(buffer);
			taille<<=1;
		}
		else
		{
			if (setenv("PWD", buffer, 1) != 0)
				fprintf(stderr, "Philsh : Impossible de fixer la valeur de PWD à %s\n", buffer);
			return buffer;
		}
	}
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
	printf("%s\n", get_current_dir());
	return 0;
}

