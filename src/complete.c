/*
 * Copyright (C) 2008 Philippe Pepiot <philippe.pepiot@gmail.com>
 * philsh is under BSD licence, see LICENCE file for more informations.
 *
 */ 

#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include "complete.h"

/* Initialisation de la liste des commandes
 * une sorte de source profile */
/* {{{ init_command_name() */
void init_command_name(void)
{
   char *path = malloc(sizeof(char) * (1+strlen(getenv("PATH"))));
   char *dir_name;
   DIR *dir;
   struct dirent *file;
   if(command_completion)
      free_file_completion(command_completion);
   command_completion = NULL;
   strcpy(path, getenv("PATH"));
   dir_name = strtok(path, ":");
   while(dir_name != NULL)
   {
      if((dir = opendir(dir_name)))
      {
	 while((file = readdir(dir)))
	    command_completion = add_file_completion(file->d_name, file->d_type, command_completion);
	 closedir(dir);
      }
      dir_name = strtok(NULL, ":");
   }
   return;
}
/* }}} */

/* Completion sur les fichiers */
/* {{{ file_complete() */
char *file_complete(char *str, unsigned int flags, char *prompt)
{
   char *p = strrchr(str, ' '), *path, *q;
   DIR *rep;
   struct dirent *file;
   size_t lenght, j;
   unsigned char type;
   struct stat file_stat;
   file_completion *ll = NULL, *p_ll = NULL;
   /* S'il n'y a pas d'espaces, c'est qu'on cherche
    * une commande */
   if(p == NULL)
   {
      p = str;
      /* Si on cherche une commande dans $PATH */
      if(*p != '/' && *p != '.')
	 return comand_complete(str, prompt);
      /* Sinon on continu sur la completion de fichier */
   }
   /* On grille les espaces */
   while(*p == ' ')
      p++;
   /* S'il n'y a pas de debut de chaine */
   if(*p == '\0' && (flags & NOVERBOSE))
      return NULL;
   /* On cherche le repertoire qu'il faut completer */
   if(NULL != (q = strrchr(p, '/')))
   {
      /* Si /foo<tab> */
      if(q == p)
      {
	 path = malloc(sizeof(char) * 2);
	 strcpy(path, "/");
      }
      /* Sinon foo/bar<tab> */
      else
      {
	 path = malloc(sizeof(char) * (1+q-p));
	 strncpy(path, p, q-p);
	 path[q-p] = '\0';
      }
      p = q+1;
      rep = opendir(path);
      free(path);
   }
   /* Sinon on cherche dans
    * le repertoire courrant */
   else
      rep = opendir(".");
   if(rep != NULL)
   {
      while((file = readdir (rep)))
      {
	 /* Si le debut de chaine correspond
	  * à un fichier */
	 if(!strncmp(p, file->d_name, strlen(p)))
	 {
	    /* Ne prendre les fichier cachés que si
	     * necessaire... */
	    if('.' != file->d_name[0] || '.' == p[0])
	    {
	       type = file->d_type;
	       if(flags & DIR_ONLY)
	       {
		  stat(file->d_name, &file_stat);
		  if(S_ISDIR(file_stat.st_mode))
		     ll = add_file_completion(file->d_name, file->d_type, ll);
	       }
	       else
		  ll = add_file_completion(file->d_name, file->d_type, ll);
	    }
	 }
      }
      if(ll == NULL)
	 return NULL;
      else if(ll->next == NULL)
      {
	 lenght = strlen(p);
	 p = malloc(sizeof(char) * (2+strlen(ll->name+lenght)));
	 strcpy(p, ll->name+lenght);
	 if(ll->type == DT_DIR)
	    strcat(p, "/");
	 closedir(rep);
	 free(ll->name);
	 free(ll);
	 return p;
      }
      else
      {
	 if(!(flags & VERBOSE))
	 {
	    printf("\n");
	    p_ll = ll;
	    while(p_ll != NULL)
	    {
	       if(p_ll->type == DT_DIR)
		  printf("\033[34m%s\t\033[37m", p_ll->name);
	       else
		  printf("%s\t", p_ll->name);
	       p_ll = p_ll->next;
	    }
	    printf("\n");
	    printf("%s%s", prompt, str);
	 }
	 for(j = 0; j < strlen(ll->name)-strlen(p); j++)
	 {
	    p_ll = ll;
	    while(p_ll != NULL)
	    {
	       if(p_ll->name[j+strlen(p)] != ll->name[j+strlen(p)])
	       {
		  lenght = strlen(p);
		  p = malloc(sizeof(char) * (strlen(p_ll->name+lenght)-j+2));
		  strncpy(p, ll->name+lenght, j);
		  p[j] = '\0';
		  closedir(rep);
		  free_file_completion(ll);
		  return p;
	       }
	       p_ll = p_ll->next;
	    }
	 }
      }
      closedir(rep);
   }
   free_file_completion(ll);
   return NULL;
}
/* }}} */


/* Completion des commandes */
/* {{{ comand_complete() */
char *comand_complete(char *str, char *prompt)
{
   file_completion *ll = NULL, *p_ll;
   char *p  = str;
   size_t i;
   if(p == NULL || p[0] == '\0')
      return NULL;
   while(*p == ' ')
      p++;
   p_ll = command_completion;
   while(p_ll != NULL)
   {
      if(!strncmp(p, p_ll->name, strlen(p)))
	 ll = add_file_completion(p_ll->name, p_ll->type, ll);
      p_ll = p_ll->next;
   }
   if(ll == NULL)
   {
      free_file_completion(ll);
      return NULL;
   }
   else if(ll->next == NULL)
   {
      /* Un seul resultat */
      i = strlen(p);
      p = malloc(sizeof(char) * (3+strlen(ll->name+i)));
      strcpy(p, ll->name+i);
      strcat(p, " ");
      return p;
   }
   else
   {
      /* Plusieurs resultats */
      /* Afficher les resultats */
      printf("\n");
      p_ll = ll;
      while(p_ll != NULL)
      {
	 printf("\033[36m%s\t\033[37m", p_ll->name);
	 p_ll = p_ll->next;
      }
      printf("\n");
      printf("%s%s", prompt, str);
      /* TODO : completion */
   }
   free_file_completion(ll);
   return NULL;
}
/* }}} */

/* Gestion de la liste chainée des correspondances */
/* {{{ fonctions de traitement de la liste */
file_completion *add_file_completion(char *name, unsigned char type, file_completion *liste)
{
   file_completion *new = NULL, *p_ll = liste;
   struct stat file_stat;
   /* On regarde si l'ellement n'est pas
    * déjà dans la liste */
   if(liste != NULL)
   {
      while(p_ll != NULL)
      {
	 if(!strcmp(name, p_ll->name))
	    return liste;
	 p_ll = p_ll->next;
      }
   }
   new = malloc(sizeof(file_completion));
   /* Il faut copier sinon l'information se
    * perd quand on ferme le repertoire */
   new->name = malloc(sizeof(char) * (1+strlen(name)));
   strcpy(new->name, name);
   if(type == DT_LNK)
   {
      stat(name, &file_stat);
      if(S_ISDIR(file_stat.st_mode))
	 new->type = DT_DIR;
   }
   else
      new->type = type;
   new->next = liste;
   return new;
}

void free_file_completion(file_completion *liste)
{
   file_completion *p;
   if(liste == NULL)
      return;
   else
   {
      p = liste->next;
      free(liste->name);
      free(liste);
      return free_file_completion(p);
   }
}

/* }}} */
