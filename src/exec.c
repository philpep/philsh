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
#include "exec.h"
#include "int/internal.h"
#include "philsh.h" /* Pour options_philsh() */
#include "int/alias.h" /* Pour alias() unalias() */
#include "readconfig.h" /* Pour source() */

extern char *chemin;
struct lljobs *liste_jobs = NULL;

extern const struct builtin builtin_command[];
extern alias_ll *liste_alias;
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

/* {{{ exec_cmd_external() */
int exec_cmd_external(char **argv)
{
   pid_t pid;
   pid = fork();
   switch (errno)
   {
      case EAGAIN:
	 perror("philsh: cannot allocate sufficient memory to create fork\n");
	 exit(EAGAIN);
	 break;
      case ENOMEM:
	 perror("philsh: failed to allocate the necessary kernel Structure\n");
	 exit(ENOMEM);
	 break;
   }
   if (pid == 0)
   {
      if (which_cmd(argv[0]) != 0)
      {
	 fprintf(stderr, "philsh: command not found : %s\n", argv[0]);
	 exit(1);
      }
      return execv(chemin, argv);
   }
   else if (pid != -1)
      wait(NULL);
   else if (pid == -1)
      return -1;
   return -1;
}
/* }}} */

/* {{{ exec_cmd() */
/* LA fonction d'execution de philsh
 * TODO : simplifier, c'est un bordel
 * monstrueux !
 * Cette fonction renvoie le code de sortie
 * de l'application lancée (par un moyen on ne
 * peut plus douteux :)
 */
int exec_cmd(int argc, char **argv)
{
   int bg = 0, err, ret, file, null;
   char *p, *chemin_cmd_locale = NULL;
   extern alias_ll *liste_alias;
   pid_t pid;
   const struct builtin *p_builtin;
   extern char **environ;
   p_builtin = builtin_command;
   while(p_builtin->name != NULL)
   {
      if((!strcmp(argv[0], p_builtin->name))&&(p_builtin->process == SAME_PROCESS))
	 return p_builtin->p(argc, argv);
      p_builtin++;
   }
   /* bg = 1 : commande en backgroud */
   p = &argv[argc-1][strlen(argv[argc-1])-1];
   if(*p == '&')
   {
      bg = 1;
      /* Il faut distinguer
       * truc machin& et
       * truc machin &
       */
      if (p == argv[argc-1])
      {
	 argc--;
	 argv[argc] = NULL;
      }
      else
	 *p = '\0';
   }
   if (strchr(argv[0], '='))
      return internal_setenv(argv[0]);
   /* On crée le fork */
   pid = fork();
   err = errno;
   if (pid == 0)
   {
      /* Pocessus fils, si la commande est interne à philsh
       * elle est executé
       * si la commande est externe, elle est executé
       * par execvp. */
      /* S'il faut lancer en backgroud */
      if(bg)
      {
	 null = open("/dev/null", O_RDONLY);
	 if (null == -1)
	    exit(1);
	 dup2(null, 0);
	 signal(SIGINT, SIG_IGN);
      }
      p_builtin = builtin_command;
      while(p_builtin->name != NULL)
      {
	 if(!strcmp(argv[0], p_builtin->name))
	    exit (p_builtin->p(argc, argv));
	 p_builtin++;
      }
      /* Si on à affaire à un ./  */
      if ( (argv[0][0] == '.')&&(argv[0][1] == '/') )
      {
	 chemin = get_current_dir();
	 chemin_cmd_locale = malloc (sizeof(char) * (strlen(chemin)+strlen(argv[0])));
	 assert(chemin_cmd_locale != NULL);
	 sprintf(chemin_cmd_locale, "%s/%s", chemin, argv[0]+2);
	 execv(chemin_cmd_locale, argv);
      }
      if (argv[0][0] == '_')
	 argv[0]++;
      else if (argv[0][0] == '/')
      {
	 file = open(argv[0], O_RDONLY, S_IXUSR | S_IXGRP);
	 if ((errno == EACCES)||(file == -1))
	 {
	    fprintf(stderr, "philsh: cannot execute %s : Permission denied\n", argv[0]);
	    exit(EACCES);
	 }
	 fexecve(file, argv, environ);
      }
      /* Si la commande est externe */
      if (which_cmd (argv[0]) != 0)
	 fprintf(stderr, "philsh : command not found : %s\n", argv[0]);
      else
	 execv(chemin, argv);
      exit(127);
   }
   else if (pid != -1 && !bg)
   {
      /* Processus père
       * Il attent que le processus fils
       * se termine */
      signal(SIGINT, SIG_IGN);
      while(!WaitForChild(pid, &ret));
      signal(SIGINT, HandleInterrupt);
   }
   else if(pid != -1 && bg)
   {
      /* On l'ajoute à la liste des jobs */
      liste_jobs = add_job(liste_jobs, argv[0], pid);
      printf("[\033[36m%d\033[37m] : \033[34m%s\033[37m\n", pid, argv[0]);
   }
   else
   {
      switch (err)
      {
	 case EAGAIN:
	    fprintf(stderr, "Philsh : ne peut pas allouer asser de memoire pour créer un nouveau processus fils\n");
	    break;
	 case ENOMEM:
	    fprintf(stderr, "Philsh : Impossible de faire un fork(), le noyau n'a plus assez de memoire\n");
	    break;
      }
      return -1;
   }
   return ret;
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
