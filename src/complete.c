#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "complete.h"

/* Completion sur les fichiers
 * TODO : ecrire le debut si plusieurs
 * fichier marchent */
char *file_complete(char *str)
{
   char *p = strrchr(str, ' ');
   char *q;
   DIR *rep;
   struct dirent *file;
   size_t match = 0;
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
	 {
	    match++;
	    q = file->d_name;
	 }
      }
      if(match == 1)
      {
	 match = strlen(p);
	 p = malloc(sizeof(char) * (1+strlen(q+match)));
	 strcpy(p, q+match);
	 closedir(rep);
	 return p;
      }
      closedir(rep);
   }
   return NULL;
}
