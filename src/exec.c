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
#include <signal.h>

struct lljobs *liste_jobs = NULL;

/* LA fonction d'execution de philsh,
 * elle prend en paramêtre une file d'instruction
 * (structure définie dans file_instruction.h)
 * elle retourne la valeur de retour de la dernière
 * instruction */
/* {{{ exec_file_instruction() */
int exec_file(file_instruction *liste)
{
   char *ptr, *path;
   pid_t pid;
   int bg = 0, fd, ret, p[2], status;
   const struct builtin *p_builtin;
   extern const struct builtin builtin_command[];
   extern lljobs *liste_jobs;
   /* On ne sais jamais */
   if(liste == NULL)
      return 0;
   /* On teste si la commande est une builtin
    * a lancer dans le même processus.
    * Typiquement cd */
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
   /* *p est le dernier caractère du dernier argument de la liste
    * On veut voir si *p == &, pour savoir s'il faut lancer la commande
    * en background...
    * TODO : Pourquoi ne pas l'intergrer dans liste->red_type ?
    */
   ptr = &liste->argv[liste->argc-1][strlen(liste->argv[liste->argc-1])-1];
   if(*ptr == '&')
   {
      bg = 1;
      /* machin & != machin& */
      if(ptr == liste->argv[liste->argc-1])
      {
	 liste->argc--; /* TODO : fuite memoire ici ? */
	 liste->argv[liste->argc] = NULL;
      }
      else
	 *ptr = '\0';
   }
   /* Duplication du processus */
   if(-1 == pipe(p))
      perror("pipe()");
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
      /* Tests de redirections
       * TODO : meilleur affichage des érreurs possibles
       */
      if(liste->flags & RED_CREAT)
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
      else if (liste->flags & RED_ADD)
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
      if(liste->flags & PIPE)
      {
	 close(p[0]);
	 close(1);
	 dup(p[1]);
      }
      /* On recupère le builtin restantes,
       * C'est a dire celles qui ne sont pas SAME_PROCESS
       * (cf debut de la fonction)
       */
      p_builtin = builtin_command;
      while(p_builtin->name != NULL)
      {
	 if(!strcmp(liste->argv[0], p_builtin->name))
	    exit (p_builtin->p(liste->argc, liste->argv));
	 p_builtin++;
      }
      /* Sinon, on cherche le chemin vers le premier argument de la liste dans $PATH
       * S'il n'y est pas, on renvoie 127 et s'il y est on remplace le processus
       * courant avec execv (cf man 3 exec) */
      if(NULL == (path = which_cmd (liste->argv[0])))
	 fprintf(stderr,"Philsh: command not found : %s\n", liste->argv[0]);
      else
	 execv(path, liste->argv);
      exit(127);
   }
   else if(pid != -1)
   {
      /* Processus père, il se charge de gérer la liste des
       * jobs, execute la prochaine instruction
       * ou retourne la valeur de retour
       * TODO : Lire man signal :) */
      if(!bg)
      {
	 signal(SIGINT, SIG_IGN);
	 while(!WaitForChild(pid, &ret)); /*  cf un peu plus bas */
	 signal(SIGINT, HandleInterrupt);
      }
      else
      {
	 /* On l'ajoute à la liste des jobs, et on affiche un joli message */
	 liste_jobs = add_job(liste_jobs, liste->argv[0], pid);
	 printf("[\033[36m%d\033[37m] : \033[34m%s\033[37m\n", pid, liste->argv[0]);
      }
      if(liste->flags & PIPE)
      {
	 waitpid(pid, &status, WNOHANG);
	 close(p[1]);
	 close(0);
	 dup(p[0]);
      }
      /* S'il reste des instructions, on les executes sinon on
       * renvoie la valeur de retour */
      if(liste->next != NULL)
	 return exec_file(liste->next);
      else
	 return ret;
   }
   else
      perror("fork()");
   /* Le programme n'est jamais censé arriver jusque là, cela signifirais
    * que pid == -1, donc une grosse érreur fatale, du coup on quitte
    * brutalement, le return 0 c'est juste pour eviter les érreurs de gcc :) */
   abort();
   return 0;
}

/* }}} */

/* Affiche simplement l'aide */
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

/* Affiche la liste des jobs en cours */
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

/* La fonction pour quitter philsh */
int exit_philsh(int argc, char **argv)
{
   int ret = 0,i;
   extern alias_ll *liste_alias;
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

/* Cette fonction attend un changement d'état d'un des
 * processus fils.
 * Si le fils est terminé, on stoque sa valeur
 * de retour dans ret...
 */
/* {{{ WaitForChild() && HandleInterrupt() */
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
   philsh();
}
/* }}} */

/* Ce sont des fonction de modifications de listes chainées
 * toutes simples */
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
