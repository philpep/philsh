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
   size_t comands = 0;
   strcpy(path, getenv("PATH"));
   dir_name = strtok(path, ":");
   while(dir_name != NULL)
   {
      if((dir = opendir(dir_name)))
      {
	 while((file = readdir(dir)))
	    comands++;
	 closedir(dir);
      }
      dir_name = strtok(NULL, ":");
   }
   command_names = malloc(sizeof(char *) * comands);
   comands = 0;
   strcpy(path, getenv("PATH"));
   dir_name = strtok(path, ":");
   while(dir_name != NULL)
   {
      if((dir = opendir(dir_name)))
      {
	 while((file = readdir(dir)))
	 {
	    if(file->d_name[0] != '.')
	    {
	       command_names[comands] = malloc(sizeof(char) * (1+strlen(file->d_name)));
	       strcpy(command_names[comands++], file->d_name);
	    }
	 }
	 closedir(dir);
      }
      dir_name = strtok(NULL, ":");
   }
   command_names[comands] = NULL;
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
   /* Si la commande est cd : completion uniquement
    * sur les dossiers */
   /*
      if(!strncmp(str, "cd ", 3))
      flags |= DIR_ONLY;
      */
   /* S'il n'y a pas d'espaces, c'est qu'on cherche
    * une commande */
   if(p == NULL)
      return comand_complete(str, flags, prompt);
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
      if(ll != NULL && ll->next == NULL)
      {
	 lenght = strlen(p);
	 p = malloc(sizeof(char) * (2+strlen(ll->name+lenght)));
	 strcpy(p, ll->name+lenght);
	 if(ll->type == DT_DIR)
	    strcat(p, "/");
	 closedir(rep);
	 return p;
      }
      else if(ll != NULL && ll->next != NULL)
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
		  return p;
	       }
	       p_ll = p_ll->next;
	    }
	 }
      }
      closedir(rep);
   }
   return NULL;
}
/* }}} */


/* Completion des commandes */
/* {{{ comand_complete() */
char *comand_complete(char *str, unsigned int flags, char *prompt)
{
   char *p = str, *q[MAX_COMPLETION];
   size_t match = 0, i = 0, j;
   if(p == NULL || p[0] == '\0')
      return NULL;
   while(*p == ' ')
      p++;
   while(command_names[i] != NULL)
   {
      if (match > MAX_COMPLETION)
	 break;
      if(!strncmp(p, command_names[i], strlen(p)))
	 q[match++] = command_names[i];
      i++;
   }
   if(match == 1)
   {
      match = strlen(p);
      p = malloc(sizeof(char) * (3+strlen(q[0]+match)));
      strcpy(p, q[0]+match);
      strcat(p, " ");
      return p;
   }
   else if(match != 0)
   {
      if((flags & VERBOSE))
      {
	 printf("\n");
	 for(i = 0; i < match; i++)
	    printf("%s\t", q[i]);
	 printf("\n");
	 printf("%s%s", prompt, str);
      }
      for(j = 0; j < strlen(q[0])-strlen(p); j++)
      {
	 for(i = 0; i < match; i++)
	 {
	    if(q[i][j+strlen(p)] != q[0][j+strlen(p)])
	    {
	       match = strlen(p);
	       p = malloc(sizeof(char) * (strlen(q[0]+match)-j+2));
	       strncpy(p, q[0]+match, j);
	       p[j] = '\0';
	       return p;
	    }
	 }
      }
   }
   return NULL;
}
/* }}} */

/* Gestion de la liste chainée des correspondances */
file_completion *add_file_completion(char *name, unsigned char type, file_completion *liste)
{
   file_completion *new = malloc(sizeof(file_completion));
   struct stat file_stat;
   new->name = name;
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
