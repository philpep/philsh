/*
 * prompt.c
 * Copyright (C) 2008 Philippe Pepiot <philippe.pepiot@gmail.com>
 * See LICENCE file for licence of this piece of software
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

char *set_prompt(void)
{
   /* Déclarations */
   char *prompt;
   struct utsname machine;
   struct passwd *user;
   uid_t uid;
   char *pwd;
   size_t prompt_size;
   /* Recuperation des informations sur l'utilisateur
    * et sur le systeme */
   uid = getuid();
   user = getpwuid(uid);
   pwd = get_current_dir();
   uname(&machine);
   prompt_size = 39 + strlen(user->pw_name) + \
		 strlen(machine.nodename) + strlen(pwd);
   prompt = malloc(sizeof(char) * prompt_size);
   /* Permet de remplacer $HOME par ~ */
   if(!strncmp(pwd, user->pw_dir, strlen(user->pw_dir)))
      sprintf(prompt, "[\033[31m%s\033[37m@\033[35m%s\033[37m: \033[34m~%s\033[37m] $>", user->pw_name, machine.nodename, pwd+strlen(user->pw_dir));
   else
      sprintf(prompt, "[\033[31m%s\033[37m@\033[35m%s\033[37m: \033[34m%s\033[37m] $>", user->pw_name, machine.nodename, pwd);
   free(pwd);
   return prompt;
}
