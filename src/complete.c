#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "complete.h"

/* Completion sur les fichiers */
char *file_complete(char *str)
{
   char *p = strrchr(str, ' ');
   char *q[100];
   DIR *rep;
   struct dirent *file;
   size_t match = 0, i, j;
   if(p == NULL)
      return NULL;
   while(*p == ' ')
      p++;
   if(*p == '\0')
      return NULL;
   rep = opendir(".");
   if(rep != NULL)
   {
      while((file = readdir (rep)))
      {
	 if(!strncmp(p, file->d_name, strlen(p)))
	    q[match++] = file->d_name;
      }
      if(match == 1)
      {
	 match = strlen(p);
	 p = malloc(sizeof(char) * (1+strlen(q[0]+match)));
	 strcpy(p, q[0]+match);
	 closedir(rep);
	 return p;
      }
      else if(match > 1)
      {
	 for(j = 0; j < strlen(q[0])-strlen(p); j++)
	 {
	    for(i = 0; i < match; i++)
	    {
	       if(q[i][j+strlen(p)] != q[0][j+strlen(p)])
	       {
		  match = strlen(p);
		  p = malloc(sizeof(char) * (strlen(q[0]+match)-j+1));
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
