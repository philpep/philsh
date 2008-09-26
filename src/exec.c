/*
 * exec.c
 * Copyright (C) 2008 Philippe Pepiot <philippe.pepiot@gmail.com>
 * See LICENCE file for licence of this piece of software
 *
 */

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
#include "int/alias.h"

extern char *chemin;
struct lljobs *liste_jobs = NULL;

extern const struct builtin builtin_command[];
extern alias_ll *liste_alias;

int exec_cmd_external(char **argv)
{
   pid_t pid;
   pid = fork();
   switch (errno)
   {
      case EAGAIN:
	 perror("Philsh: pas assez de memoire pour créer le fork\n");
	 exit(EAGAIN);
	 break;
      case ENOMEM:
	 perror("Philsh: Le noyau n'a pas assez de memoire");
	 exit(ENOMEM);
	 break;
   }
   if (pid == 0)
   {
      if (which_cmd(argv[0]) != 0)
      {
	 fprintf(stderr, "Philsh: %s commande non trouvée\n", argv[0]);
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

int exec_cmd(int argc, char **argv)
{
   char *p;
   char *chemin_cmd_locale = NULL;
   int bg = 0, i, err;
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
   /* Si la commande est exit : quitter le shell */
   if (!strcmp(argv[0], "exit"))
   {
      if (liste_jobs != NULL)
      {
	 fprintf(stderr, "Philsh: vous avez des jobs en cours\n");
	 afficher_liste_jobs(liste_jobs);
	 return 1;
      }
      for(i = 0; i < argc; i++)
	 free(argv[i]);
      free(argv);
      exit(0);
   }
   if (!strcmp(argv[0], "alias"))
      return alias(argc, argv);
   /* Si la commande est cd, on ne l'execute pas
    * dans le fork, sinon le changement de repertoire
    * s'effectue dans le processus fils :) */
   if (!strcmp(argv[0], "cd"))
      return internal_cd(argc, argv);
   else if (!strcmp(argv[0], "help"))
   {
      afficher_aide();
      return 0;
   }
   /* Meme remaque */
   else if (strchr(argv[0], '='))
      return internal_setenv(argv[0]);
   /* On crée le fork */
   pid_t pid = fork();
   err = errno;
   if (pid == 0)
   {
      /* Pocessus fils, si la commande est interne à philsh
       * elle est executé
       * si la commande est externe, elle est executé
       * par execvp. */
      /* S'il faut lancer en backgroud */
      const struct builtin *p_builtin;
      if(bg)
      {
	 int null = open("/dev/null", O_RDONLY);
	 if (null == -1)
	    exit(1);
	 dup2(null, 0);
	 signal(SIGINT, SIG_IGN);
      }
      if(!strcmp(argv[0], "jobs"))
      {
	 afficher_liste_jobs(liste_jobs);
	 exit(0);
      }
      /* Si on à affaire à un ./ ou un exec */
      else if ( (argv[0][0] == '.')&&(argv[0][1] == '/') )
      {
	 chemin = get_current_dir();
	 chemin_cmd_locale = malloc (sizeof(char) * (strlen(chemin)+strlen(argv[0])));
	 assert(chemin_cmd_locale != NULL);
	 sprintf(chemin_cmd_locale, "%s/%s", chemin, argv[0]+2);
	 execv(chemin_cmd_locale, argv);
      }
      p_builtin = builtin_command;
      while(p_builtin->name != NULL)
      {
	 if(!strcmp(argv[0], p_builtin->name))
	    exit (p_builtin->p(argc, argv));
	 p_builtin++;
      }
      if (argv[0][0] == '_')
	 argv[0]++;
      else if (argv[0][0] == '/')
      {
	 int file = open(argv[0], O_RDONLY, S_IXUSR | S_IXGRP);
	 extern char **environ;
	 if ((errno == EACCES)||(file == -1))
	 {
	    fprintf(stderr, "Philsh: %s permission refusée\n", argv[0]);
	    exit(EACCES);
	 }
	 fexecve(file, argv, environ);
      }
      /* Si la commande est externe */
      if (which_cmd (argv[0]) != 0)
	 fprintf(stderr, "Philsh : %s commande inconnue\n", argv[0]);
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
      while(!WaitForChild(pid));
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
   return 0;
}

int WaitForChild(pid_t pid)
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
