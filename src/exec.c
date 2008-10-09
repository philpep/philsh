/*
 * Copyright (C) 2008 Philippe Pepiot <philippe.pepiot@gmail.com>
 * philsh is under BSD licence, see LICENCE file for more informations.
 *
 */ 
#define _GNU_SOURCE /* for fexecve definition */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h> /* for open() */
#include <sys/time.h> /* for fexecve */
#include "file_instruction.h"
#include "int/internal.h"
#include "philsh.h" /* Pour options_philsh() */
#include "int/alias.h" /* Pour alias() unalias() */
#include "readconfig.h" /* Pour source() */
#include "exec.h"

struct lljobs *liste_jobs = NULL;

extern alias_ll *liste_alias;

/* {{{ exec_file_instruction() */
int exec_file(file_instruction *liste)
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
	 ret = p_builtin->p(liste->argc, liste->argv);
	 if (liste->next != NULL)
	    	 return exec_file(liste->next);
	 else
	    return ret;
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
	 return exec_file(liste->next);
      else
	 return ret;
   }
   return 0;
}

/* }}} */

/* {{{ builtin exit && jobs && help */
int help(int argc, char **argv)
{
   if(argc != 1)
   {
      fprintf(stderr,"Philsh: Usage: %s : Print help\n", argv[0]);
      return ERR_ARG;
   }
   afficher_aide();
   return 0;
}

int jobs(int argc, char **argv)
{
   if(argc != 1)
   {
      fprintf(stderr,"Philsh : Usage: %s : Print list of runnings jobs\n", argv[0]);
      return ERR_ARG;
   }
   afficher_liste_jobs(liste_jobs);
   return 0;
}


int exit_philsh(int argc, char **argv)
{
   int ret = 0,i;
   if(argc > 2)
   {
      fprintf(stderr,"Philsh : Usage: exit [EXIT_CODE]");
      return ERR_ARG;
   }
   if(argc == 2)
      ret = atoi(argv[1]);
   if(liste_jobs != NULL)
   {
      fprintf(stderr,"You have runnings jobs\n");
      afficher_liste_jobs(liste_jobs);
      return ERR_EXEC;
   }
   /* On libère la memoire et on
    * quitte */
   while(liste_alias != NULL)
      del_alias(liste_alias->name);
   for(i = 0; i < argc; i++)
      free(argv[i]);
   free(argv);
   exit(ret);
}
/* }}} */

/* {{{ WaitForChild() && HandleInterrupt() */
/* Cette fonction attend un changement d'état d'un
 * processus fils.
 * Si le fils est terminé, on stoque sa valeur
 * de retour dans ret...
 */
int WaitForChild(pid_t pid, int *ret)
{
   pid_t cpid;
   int st;
   for (;;)
   {
      /* Attendre la mort d'un fils. */
      cpid = wait(&st);
      if (cpid == -1)
      {
	 if (errno == ECHILD)
	    break;
	 else if (errno == EINTR) 
	    continue;
	 else
	    exit(1);
      }
      if (WIFEXITED(st) || WIFSIGNALED(st))
      {
	 if (cpid == pid) /* Le fils attendu est mort */
	 {
#ifdef VERBOSE
	    printf("\033[31mFin du processus de pid : %d, code de retour : %d\n\033[37m", pid, WEXITSTATUS(st));
#endif
	    *ret = WEXITSTATUS(st);
	    return 1;
	 }
	 else
	 {
	    liste_jobs = del_job(liste_jobs, cpid);
#ifdef VERBOSE
	    printf("[\033[36m%d\033[37m]: Terminé, code de retour : %d\n", pid, WEXITSTATUS(st));
#endif
	 }
      }
      else if (WIFSTOPPED(st))
      {
#ifdef VERBOSE
	 printf("\033[31m\t\tStoppé: pid=%d\n\033[37m", cpid);
#endif
      }
   }
   return 0;
}


void HandleInterrupt(int sig)
{
   int sig2;
   sig2 = sig;
   printf("\n");
   /* Reinstaller la routine. */
   signal(SIGINT, HandleInterrupt);
}
/* }}} */

/* {{{ jobs functions */
lljobs *add_job(lljobs *liste, char *name, pid_t pid)
{
   struct lljobs *new = malloc(sizeof(lljobs));
   assert(new != NULL);
   new->name = malloc(sizeof(char) * (1+strlen(name)));
   assert(new->name != NULL);
   strcpy(new->name, name);
   new->pid = pid;
   new->next = liste;
   return new;
}

lljobs *del_job(lljobs *liste, pid_t pid)
{
   if (liste == NULL)
      return liste;
   struct lljobs *p;
   struct lljobs *prec;
   p = liste;
   prec = liste;
   while(p != NULL)
   {
      if (p->pid == pid)
      {
	 if (p == liste)
	 {
	    p = liste->next;
	    free(liste->name);
	    free(liste);
	    return p;
	 }
	 prec->next = p->next; 
	 free(p->name);
	 free(p);
	 return liste;
      }
      prec = p;
      p = p->next;
   }
   return liste;
}


int afficher_liste_jobs(lljobs *liste)
{
   if (liste == NULL)
   {
      printf("Pas de jobs en cours\n");
      return 0;
   }
   struct lljobs *tmp;
   tmp = liste;
   while(tmp != NULL)
   {
      printf("[\033[36m%d\033[37m] : \033[34m%s\033[37m\n", tmp->pid, tmp->name);
      tmp = tmp->next;
   }
   return 0;
}

/* }}} */


/* vim:fdm=marker:
*/
