/*
 * Copyright (C) 2008 Philippe Pepiot <philippe.pepiot@gmail.com>
 * philsh is under BSD licence, see LICENCE file for more informations.
 *
 */ 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> /* pour getuid() */
#include <pwd.h>
#include <sys/utsname.h>
#include <sys/types.h> /* pour uid_t */
#include "int/internal.h"
#include "prompt.h"

char *set_prompt(int ret)
{
   /* Déclarations */
   char *prompt, *pwd;
   struct utsname machine;
   struct passwd *user;
   uid_t uid;
   size_t prompt_size;
   /* Recuperation des informations sur l'utilisateur
    * et sur le systeme */
   uid = getuid();
   user = getpwuid(uid);
   pwd = get_current_dir();
   uname(&machine);
   prompt_size = 60 + strlen(user->pw_name) + \
		 strlen(machine.nodename) + strlen(pwd);
   prompt = malloc(sizeof(char) * prompt_size);
   if(ret != 0)
      sprintf(prompt, "[\033[34m%d\033[36m!\033[37m][\033[31m%s\033[37m@\033[35m%s\033[37m: \033[34m", ret, user->pw_name, machine.nodename);
   else
      sprintf(prompt, "[\033[31m%s\033[37m@\033[35m%s\033[37m: \033[34m", user->pw_name, machine.nodename);
   /* Permet de remplacer $HOME par ~ */
   if(!strncmp(pwd, user->pw_dir, strlen(user->pw_dir)))
   {
      strcat(prompt, "~");
      strcat(prompt, pwd+strlen(user->pw_dir));
   }
   else
      strcat(prompt, pwd);
   strcat(prompt, "\033[37m $>");
   free(pwd);
   return prompt;
}
