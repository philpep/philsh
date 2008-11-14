#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "complete.h"


/* Completion sur les fichiers */
char *file_complete(char *str)
{
   char *p = strrchr(str, ' ');
   char *q[MAX_COMPLETION], *path;
   DIR *rep;
   struct dirent *file;
   size_t match = 0, i, j;
   unsigned char type;
   if(p == NULL)
      return NULL;
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
