#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/utsname.h>
#include "int/internal.h"

char* get_prompt(void)
{
	extern char *prompt;
	char *prompt_tmp = NULL; 
	char *p = NULL;
	/* On recupère les variables
	 * pour l'affichage du prompt */
	/* Le hostname */
	struct utsname localhost;
	uname(&localhost);
	/* Le login de l'utilisateur */
	/* TODO : utiliser pwd.h pour ça */
	char *user = getenv("USER");
	/* Le dossier courant */
	char *current_dir = get_current_dir();
	/* Le HOME de l'utilisateur */
	char *home = getenv("HOME");
	/* Cette partie remplace HOME par ~ dans current_dir */
	size_t i = 0;
	for(i = 0; i < strlen(home); i++)
	{
		if (current_dir[i] != home[i])
			break;
		if (i == strlen(home)-1)
			sprintf(current_dir, "~%s", current_dir+i+1);
	}
	/* TODO : utiliser PS1
	 * Puis l'allouer en fonction */
	if (getenv("PHILSH_PROMPT") == NULL)
	{
		prompt = malloc (sizeof(char) * (strlen(user)+strlen(current_dir) + \
					strlen(home)+39));
		assert(prompt != NULL);
		sprintf(prompt, "[\033[31m%s\033[37m@\033[35m%s\033[37m: \033[34m%s\033[37m] $> ", \
				user, localhost.nodename, current_dir);
		/* cf src/int/pwd.c */
		free(current_dir);
		return prompt;
	}
	else
	{
		i = strlen(getenv("PHILSH_PROMPT")) + strlen(user) + \
		    strlen(localhost.nodename) + strlen(current_dir);
		/******************/
		prompt_tmp = malloc (sizeof(char) * i);
		assert(prompt_tmp != NULL);
		strcpy(prompt_tmp, getenv("PHILSH_PROMPT"));
		/*******************/
		prompt = malloc (sizeof(char) * i);
		assert(prompt != NULL);
		/*******************/
		while ((p = strchr(prompt_tmp, '%')) != NULL)
		{
			*p = '\0';
			if (*(p+1) == 'U')
				sprintf(prompt, "%s%s%s", prompt_tmp, user, p+2);
			else if (*(p+1) == 'D')
				sprintf(prompt, "%s%s%s", prompt_tmp, current_dir, p+2);
			else if (*(p+1) == 'H')
				sprintf(prompt, "%s%s%s", prompt_tmp, localhost.nodename, p+2);
			strcpy(prompt_tmp, prompt);
		}
		strcpy(prompt, prompt_tmp);
		free(current_dir);
		free(prompt_tmp);
		return prompt;
	}
	return NULL;
}
