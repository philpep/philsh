#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "complete.h"


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
	    command_names[comands] = malloc(sizeof(char) * (1+strlen(file->d_name)));
	    strcpy(command_names[comands++], file->d_name);
	 }
         closedir(dir);
      }
      dir_name = strtok(NULL, ":");
   }
   command_names[comands] = NULL;
   return;
}

/* Completion sur les fichiers */
/* {{{ file_complete() */
char *file_complete(char *str)
{
   char *p = strrchr(str, ' ');
   char *q[MAX_COMPLETION], *path;
   DIR *rep;
   struct dirent *file;
   size_t match = 0, i, j;
   unsigned char type;
   if(p == NULL)
      return comand_complete(str);
   while(*p == ' ')
      p++;
   if(*p == '\0')
      return NULL;
   /* path absolu */
   if(*p == '/')
   {
      q[0] = strrchr(p, '/');
      if(q[0] == p)
      {
	 path = malloc(sizeof(char) * 2);
	 strcpy(path, "/");
      }
      else
      {
	 path = malloc(sizeof(char) * (1+q[0]-p));
	 strncpy(path, p, q[0]-p);
	 path[q[0]-p] = '\0';
      }
      p = q[0]+1;
      rep = opendir(path);
      free(path);
   }
   else
      rep = opendir(".");
   if(rep != NULL)
   {
      while((file = readdir (rep)))
      {
	 if(match == MAX_COMPLETION)
	    break;
	 if(!strncmp(p, file->d_name, strlen(p)))
	 {
	    q[match++] = file->d_name;
	    type = file->d_type;
	 }
      }
      if(match == 1)
      {
	 match = strlen(p);
	 p = malloc(sizeof(char) * (2+strlen(q[0]+match)));
	 strcpy(p, q[0]+match);
	 if(type == DT_DIR)
	    strcat(p, "/");
	 closedir(rep);
	 return p;
      }
      else if(match != 0)
      {
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
		  closedir(rep);
		  return p;
	       }
	    }
	 }
      }
      closedir(rep);
   }
   return NULL;
}
/* }}} */


char *comand_complete(char *str)
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
      p = malloc(sizeof(char) * (2+strlen(q[0]+match)));
      strcpy(p, q[0]+match);
      return p;
   }
   else if(match != 0)
   {
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
