/* 
 * The Phil shell (philsh)
 * 
 * Contact : Philippe Pepiot <philippe.pepiot@gmail.com>
 *
 *
 */


/* on inclu les headers */
#include "headers.h"

/* Déclaration des variables globales (à shell.c) */
/* La saisie de l'utilisateur */
char * saisie;
/* les arguments de la saisie */
char **cmd_argv;
/* le nombre d'arguments dans la saisie */
int cmd_argc;
/* Le prompt */
char *prompt;
/* le chemin de l'executable par exemple /bin/echo */
char *chemin;

/* Coupe saisie suivant les espaces
 * et rentre les arguments dans
 * cmd_argv */
/* TODO : Gerer les cas ou il y a des '"' */
void make_argv(void)
{

	int i = 0;
	char *p = strtok(saisie, " ");
	while ((p != NULL))
	{
		cmd_argv[i] = malloc (sizeof(char) * (1+strlen(p)));
		assert(cmd_argv[i] != NULL);
		strcpy(cmd_argv[i], p);
#ifdef DEBUG
		printf("argv[%d] = '%s'\n", i, cmd_argv[i]);
#endif
		i++;
		p = strtok(NULL, " ");
	}
	return;
}


int main (void)
{
	int i;
	char*p;
	/* Le retour du processus fils */
	int valeur_retour;
	/* La super boucle du phil shell */
	for (;;)
	{
		cmd_argc = 0;
		prompt = get_prompt();
		/* La fonction readline est magique */
		saisie = readline(prompt);
		/* On compte le nombres de mots dans saisie */
		for (p = saisie; *p != '\0'; p++)
	        {
			if ((*p != ' ')&&((*(p+1) == '\0')||(*(p+1) == ' ')))
				cmd_argc++;
		}
#ifdef DEBUG
		printf("saisie = '%s'\ncmd_argc = '%d'\n", saisie, cmd_argc);
#endif

		/* Si la saisie est non nulle */
		if (cmd_argc != 0)
		{
			cmd_argv = malloc (sizeof(char *) * (cmd_argc+1));
			make_argv();
			cmd_argv[cmd_argc] = NULL;
			valeur_retour = exec_cmd();
			/* On libère la memoire */
			for (i = 0; i < cmd_argc; free(cmd_argv[i]), i++);
			free(cmd_argv);
		}
		free(saisie);
	}
	return 0;
}

/* La fonction qui execute la commande
 * en créant un processus fils
 * Elle renvoie la valeur de retour
 * du processus fils */
int exec_cmd(void)
{
	int valeur_retour;
	int processus_fils;
	char *chemin_cmd_locale = NULL;
	/* Si la commande est exit : quitter le shell */
	if (!strcmp(cmd_argv[0], "exit"))
		exit(0);
	/* Si la commande est cd, on ne l'execute pas
	 * dans le fork, sinon le changement de repertoire
	 * s'effectue dans le processus fils :) */
	if (!strcmp(cmd_argv[0], "cd"))
		return internal_cd(cmd_argc, cmd_argv);
	/* Meme remaque */
	else if (strchr(cmd_argv[0], '='))
		return internal_setenv(cmd_argv[0]);
	/* On crée le fork */
	pid_t pid = fork();
	if (pid > 0)
	{
		/* Processus père :
		 * il attend que le processus fils
		 * se termine */
		processus_fils = wait(NULL);
#ifdef DEBUG
		printf("Fin du processus de PID = %d\n", processus_fils);
#endif
	}
	else if (pid == 0)
	{
		/* Pocessus fils, si la commande est interne à philsh
		 * elle est executé
		 * si la commande est externe, elle est executé
		 * par execvp.
		 * Le processus fils doit absolument se terminer
		 * sinon, on crée des processus infiniment :) */
		if (!strcmp(cmd_argv[0], "pwd"))
			valeur_retour = internal_pwd(cmd_argc, cmd_argv);
		else if (!strcmp(cmd_argv[0], "which"))
			valeur_retour = internal_which(cmd_argc, cmd_argv);
		else if (!strcmp(cmd_argv[0], "ls"))
			valeur_retour = internal_ls(cmd_argc, cmd_argv);
		else if (!strcmp(cmd_argv[0], "uname"))
			valeur_retour = internal_uname(cmd_argc, cmd_argv);
		else if (!strcmp(cmd_argv[0], "env"))
			valeur_retour = internal_env();
		/* Si on à affaire à un ./ ou un exec */
		else if ((cmd_argv[0][0] == '.')&&(cmd_argv[0][1] == '/'))
		{
			chemin = get_current_dir();
			chemin_cmd_locale = malloc (sizeof(char) * (strlen(chemin)+strlen(cmd_argv[0])));
			assert(chemin_cmd_locale != NULL);
			sprintf(chemin_cmd_locale, "%s/%s", chemin, cmd_argv[0]+2);
			valeur_retour = execv (chemin_cmd_locale, cmd_argv);
			/* Est ce utile  ? Le execv termine le processus... */
			free(chemin_cmd_locale);
			free(chemin);
		}
		else
		{
			/* Si la commande est externe */
			/* TODO : recuperer la valeur de retour du execv */
			if (which_cmd (cmd_argv[0]) != 0)
				fprintf(stderr, "Philsh : %s commande inconue\n", cmd_argv[0]);
			else
				execv(chemin, cmd_argv);
		}
#ifdef DEBUG
		printf("Valeur retour dans le fork = '%d'\n", valeur_retour);
#endif
		exit(valeur_retour);
	}
	/* Si le fork() à planté on quitte philsh brutalement */
	else
		abort();
	return valeur_retour;
}
