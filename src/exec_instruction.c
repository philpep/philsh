/*
 * src/exec_instruction.c
 * Copyright (C) 2008 Philippe Pepiot <philippe.pepiot@gmail.com>
 * philsh is under BSD licence, see LICENCE file for more informations.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "file_instruction.h"
#include "int/internal.h"
#include "exec.h"

int exec_file_instruction(file_instruction *liste)
{
   char *p;
   pid_t pid;
   int bg = 0, fd, ret;
   extern char *chemin;
   const struct builtin *p_builtin;
   extern const struct builtin builtin_command[];
   extern lljobs *liste_jobs;
   if(liste == NULL)
      return 0;
   p_builtin = builtin_command;
   while(p_builtin->name != NULL)
   {
      if(!strcmp(liste->argv[0], p_builtin->name)&&p_builtin->process == SAME_PROCESS)
      {
	 p_builtin->p(liste->argc, liste->argv);
	 return exec_file_instruction(liste->next);
      }
      p_builtin++;
   }
   if(liste == NULL)
      return -1;
   p = &liste->argv[liste->argc-1][strlen(liste->argv[liste->argc-1])-1];
   if(*p == '&')
   {
      bg = 1;
      /* machin & != machin& */
      if(p == liste->argv[liste->argc-1])
      {
	 liste->argc--; /* TODO : fuite memoire ici ? */
	 liste->argv[liste->argc] = NULL;
      }
      else
	 *p = '\0';
   }
   pid = fork();
   if(pid == 0)
   {
      /* FILS */
      if(bg)
      {
	 fd = open("/dev/null", O_RDONLY);
	 if(fd == -1)
	    exit(1);
	 dup2(fd, 0);
	 signal(SIGINT, SIG_IGN);
      }
      if(liste->red_type == RED_CREAT)
      {
	 fd = creat(liste->file, S_IRWXU);
	 if(fd == -1)
	 {
	    fprintf(stderr, "Philsh: Impossible d'ouvrir le fichier %s\n", liste->file);
	    exit(1);
	 }
	 close(1);
	 dup(fd);
      }
      else if (liste->red_type == RED_ADD)
      {
	 fd = open(liste->file, O_WRONLY | O_CREAT | O_APPEND, 0666);
	 if(fd == -1)
	 {
	    fprintf(stderr, "Philsh: Impossible d'ouvrir le fichier %s\n", liste->file);
	    exit(1);
	 }
	 close(1);
	 dup (fd);
      }
      /* On recupère le builtin restantes */
      p_builtin = builtin_command;
      while(p_builtin->name != NULL)
      {
	 if(!strcmp(liste->argv[0], p_builtin->name))
	    exit (p_builtin->p(liste->argc, liste->argv));
	 p_builtin++;
      }
      if(which_cmd (liste->argv[0]) != 0)
	 fprintf(stderr,"Philsh: command not found : %s\n", liste->argv[0]);
      else
	 execv(chemin, liste->argv);
      exit(127);
   }
   else if(pid != -1)
   {
      if(!bg)
      {
	 signal(SIGINT, SIG_IGN);
	 while(!WaitForChild(pid, &ret));
	 signal(SIGINT, HandleInterrupt);
      }
      else
      {
	 /* On l'ajoute à la liste des jobs */
	 liste_jobs = add_job(liste_jobs, liste->argv[0], pid);
	 printf("[\033[36m%d\033[37m] : \033[34m%s\033[37m\n", pid, liste->argv[0]);
      }
      if(liste->next != NULL)
	 return exec_file_instruction(liste->next);
      else
	 return ret;
   }
   return 0;
}
