#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>


/* Cette fonction est comme la commande which
 * ATTENTION : cette fonction detruit completement
 * le PATH, il faut la lancer dans un processus
 * séparé */
int internal_which(int argc, char **argv)
{
  int nb = 0;
	char *contenu_path = getenv("PATH");
	char *nom_dossier = strtok(contenu_path, ":");
	DIR *dossier;
	struct dirent *contenu_dossier;
        /* Si la commande est interne a philsh */
        if ( !strcmp(argv[1], "cd") || !strcmp(argv[1], "which") \
            || !strcmp(argv[1], "uname") || !strcmp(argv[1], "pwd") \
	    || !strcmp(argv[1], "env") )
        {
          printf("[0] %s : build-in command\n", argv[1]);
          nb = 1;
        }
	/* Si il y a trop d'arguments */
	if (argc != 2)
	{
		fprintf(stderr, "Philsh : Utilisation %s [executable]\n", argv[0]);
		return 1;
	}
	/* La boucle qui cherche l'executable dans le PATH
	 * La fonction retoure 0 dès qu'elle a trouvé quelque chose */
	while (nom_dossier != NULL)
	{
		if ((dossier = opendir(nom_dossier)) != NULL)
		{
			while ((contenu_dossier = readdir(dossier)) != NULL)
				if (strcmp(contenu_dossier->d_name, argv[1]) == 0)
				{
					printf("[%d] %s/%s\n", nb, nom_dossier, argv[1]);
					return 0;
				}
			if (closedir(dossier) == -1)
			{
				perror("Fermeture du repertoire");
				return 2;
			}
		}
		nom_dossier = strtok(NULL, ":");
	}
	/* Si on est arrivé jusqu'ici, c'est qu'on a rien trouvé */
	printf("%s non trouvé\n", argv[1]);
	return 1;
}


/* Cette fonction ecris le chemin de l'executable str dans chemin
 * Elle renvoie chemin ou NULL (echec) */
int which_cmd(char *str)
{
	extern char *chemin;
	/* On copie le PATH dans une variable locale
	 * pour ne pas modifier le vrai PATH */
	char *contenu_path = NULL;
	contenu_path = malloc (sizeof(char) * (1+strlen(getenv("PATH"))));
	assert(contenu_path != NULL);
	strcpy(contenu_path, getenv("PATH"));
	char *nom_dossier = strtok(contenu_path, ":");
	DIR *dossier;
	struct dirent *contenu_dossier;
	/* La boucle qui cherche l'executable dans le PATH
	 * La fonction retoure le chemin dès qu'elle a trouvé quelque chose */
	while (nom_dossier != NULL)
	{
		if ((dossier = opendir(nom_dossier)) != NULL)
		{
			while ((contenu_dossier = readdir(dossier)) != NULL)
				if (strcmp(contenu_dossier->d_name, str) == 0)
				{
					chemin = malloc (sizeof(char) * (strlen(nom_dossier)+strlen(str)+2));
					assert(chemin != NULL);
					sprintf(chemin, "%s/%s", nom_dossier, str);
					free(contenu_path);
					return 0;
				}
			if (closedir(dossier) == -1)
			{
				perror("Fermeture du repertoire");
				return 1;
			}
		}
		nom_dossier = strtok(NULL, ":");
	}
	/* Si on est arrivé jusqu'ici, c'est qu'on a rien trouvé */
	chemin = NULL;
	free(contenu_path);
	return 1;
}
