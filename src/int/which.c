#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include "../philsh_build.h"
#include "internal.h"


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

int internal_which(int argc, char **argv)
{
	if (argc == 1)
	{
		printf("Usage : which [OPTIONS] Nom_de_la_commande\nTapez which --help pour l'aide\n");
		return 0;
	}
	const char *optstring = "ha";
	static struct option which_option[] =
	{
		{"help", 0, NULL, 'h'},
		{"all", 0, NULL, 'a'},
		{0, 0, 0, 0}
	};
	int a = 0;
	char opt;
	while(EOF != (opt = (char)getopt_long(argc, argv, optstring, which_option, NULL)))
	{
		if ((opt == 'h')||(opt == '?'))
		{
			printf("Which %s: version pour philsh\n\
\n\n\
which [OPTIONS] nom_de_la_commande\n\
Affiche le path de la commande si elle existe, which s'arrète dès qu'il à trouvé un résultat sauf si utilisé avec l'option --all\n\
\n\
             -a, --all                Afficher tous les résultats\n\
	     -h, --help               Afficher l'aide memoire\n\
Signaler un bug à %s\n", PHILSH_VERSION, PHILSH_MAIL);
			return 0;
		}
		if (opt == 'a')
		{
			if(argc < 3)
			{
				printf("Usage : which -a command\n");
				return 0;
			}
			a = 1;
		}
	}
	if (a)
		return which(argv+2, a, 1);
	else
		return which(argv+1, a, 1);
}

int which(char **argv, int a, int ret)
{
	if(argv[0] == NULL)
		return ret;
	if(getenv("PATH") == NULL)
	{
		fprintf(stderr, "La variable d'environement PATH n'existe pas !\nImpossible de trouver %s\n", argv[0]);
		return -1;
	}
	int bingo = 0;
	char *command = argv[0];
	char *path;
	char *nom_dossier;
	size_t path_len = 1+strlen(getenv("PATH"));
	path = malloc(sizeof(char) * path_len);
	strcpy(path, getenv("PATH"));
	DIR *dossier;
	struct dirent *contenu_dossier;
	const struct builtin *p_builtin;
	p_builtin = builtin_command;
	while (p_builtin->name != NULL)
	{
	   if (!strcmp(p_builtin->name, command))
	   {
	      printf("%s : built-in command\n", command);
	      if (!a)
	      {
		 free(path);
		 return which(argv+1, a, 0);
	      }
	      bingo = 1;
	      break;
	   }
	   p_builtin++;
	}

	nom_dossier = strtok(path, ":");
	/* La boucle qui cherche l'executable dans le PATH */
	while (nom_dossier != NULL)
	{
		if ((dossier = opendir(nom_dossier)) != NULL)
		{
			while ((contenu_dossier = readdir(dossier)) != NULL)
				if (strcmp(contenu_dossier->d_name, command) == 0)
				{
					printf("%s/%s\n", nom_dossier, command);
					if(!a)
					{
						closedir(dossier);
						free(path);
						return which(argv+1, a, 0);
					}
					bingo = 1;
				}
			closedir(dossier);
		}
		nom_dossier = strtok(NULL, ":");
	}
	/* Si on est arrivé jusqu'ici, c'est qu'on a rien trouvé */
	if(!bingo)
		fprintf(stderr, "%s non trouvé\n", command);
	free(path);
	return which(argv+1, a, 1);
}
