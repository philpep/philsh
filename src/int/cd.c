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
		chemin = malloc(sizeof(char) * (1+strlen(argv[1])+strlen(user->pw_dir)));
		assert(chemin != NULL);
		sprintf(chemin, "%s%s", user->pw_name, argv[1]+1);
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

