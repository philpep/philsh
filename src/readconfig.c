/*
 * readconfig.c
 * Copyright (C) 2008 Philippe Pepiot <philippe.pepiot@gmail.com>
 * See LICENCE for licence of this piece of software
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "philsh.h"
#include "readconfig.h"


typedef unsigned short ushort; 

int init_config(char *config_file)
{
   int fd;
   char *buffer;
   struct stat buf;
   if(-1 == (fd = open(config_file, O_RDONLY)))
   {
      fprintf(stderr, "Philsh: %s n'est pas accésible !\nPhilsh: Utilisation de la configuration par default\n", config_file);
      return DEFAULT_CONFIG;
   }
   if (-1 == stat(config_file, &buf))
   {
      perror("Philsh: impossible d'obtenir des informations sur le fichier de configuration\n");
      close(fd);
      exit(EXIT_FAILURE);
   }
   /* On mappe le fichier en memoire */
   buffer = (char*) mmap(0, (int)buf.st_size, PROT_READ, MAP_SHARED, fd, 0);
   if ((char*)-1 == buffer)
   {
      perror("Philsh: Impossible de mapper le fichier de configuration en mémoire\n");
      close(fd);
      exit(EXIT_FAILURE);
   }
   /* Traitement du fichier de configuration */
   parse_file(buffer);
   /* On libère la memoire */
   munmap(buffer, (int)buf.st_size);
   return CUSTOM_CONFIG;
}


int parse_file(char *ptr)
{
   char *p;
   char *str;
   size_t i = 0;
   p = strchr(ptr, '\n');
   /* Si on est en fin de config */
   if(p == NULL)
      return 0;
   /* Si c'est un commentaire */
   if (ptr[0] == '#')
      return parse_file(p+1);
   /* On grille les espaces eventuels */
   while (ptr[i] == ' ')
      i++;
   /* Si c'est une ligne d'espaces */
   if (ptr[i] == '\n')
      return parse_file(p+1);
   /* On alloue la ligne de config */
   str = malloc(sizeof(char) * (1+p-ptr));
   memcpy(str, ptr, p-ptr);
   str[p-ptr] = '\0';
   exec_saisie(str);
   free(str);
   return parse_file(p+1);
}




