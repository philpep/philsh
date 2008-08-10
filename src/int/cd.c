#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "internal.h"

/* Fonction simple */
/* TODO : gestion des liens symboliques */
int internal_cd(int argc, char **argv)
{
	if ((argc == 1)||(!strcmp(argv[1], "~")))
	{
		chdir(getenv("HOME"));
		return 0;
	}
	else
	{
		if ( chdir(argv[1]) == -1)
			fprintf(stderr, "Philsh : %s n'est pas un repertoire\n", argv[1]);
		return -1;
	}
}

