/*
 * src/file_instruction.c
 * Copyright (C) 2008 Philippe Pepiot <philippe.pepiot@gmail.com>
 * philsh is under BSD licence, see LICENCE file for more informations.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "int/alias.h"
#include "file_instruction.h"

file_instruction *creat_liste_instruction(char *saisie)
{
   char *p, *q, *r;
   int gui = 0;
   enum _redirection_type red_type;
   file_instruction *liste = NULL;
   /**************/
   if(saisie == NULL)
      return NULL;
   q = saisie;
   for(p = saisie; *p != '\0'; p++)
   {
      if(*p == '"'||*p == '\'')
      {
	 gui = (gui) ? 0 : 1;
	 continue;
      }
      if(*p == ';'&&p != q)
      {
	 *p = '\0';
	 liste = add_instruction(liste, q, NONE, NULL);
	 q = ++p;
	 continue;
      }
      if(*p == '>'&&(gui))
      {
	 *p = '\0';
	 if(*(++p) == '>')
	 {
	    p++;
	    red_type = RED_ADD;
	 }
	 else
	    red_type = RED_CREAT;
	 while(*p == ' ')
	    p++;
	 r = p;
	 while(*r != '\0'&&*r != ' '&&*r != ';')
	    r++;
	 /* si r est la fin de saisie ---> ++r n'a plus de sens :/ */
	 if(*r == '\0')
	    return liste = add_instruction(liste, q, red_type, p);
	 *r = '\0';
	 liste = add_instruction(liste, q, red_type, p);
	 p = ++r;
	 while(*p == ' '||*p == ';')
	    p++;
	 q = p;
	 continue;
      }
   }
   liste = add_instruction(liste, q, NONE, NULL);
   return liste;
}


file_instruction *add_instruction(file_instruction *liste, char *saisie, enum _redirection_type type, char *file)
{
   int argc;
   size_t buf_size;
   extern alias_ll *liste_alias;
   alias_ll *alias = liste_alias;
   file_instruction *new, *p;
   char *str = saisie, *buffer = NULL, c;
   /*************/
   argc = compter_mots(saisie, &buf_size);
   if(argc == 0)
      return liste;
   new = malloc(sizeof(file_instruction));
   new->red_type = type;
   new->next = NULL;
   /************/
   if(file != NULL)
   {
      new->file = malloc(sizeof(char) * (1+strlen(file)));
      strcpy(new->file, file);
   }
   else
      new->file = NULL;
   /* ALIAS ? */
   while(*str == ' ')
      str++;
   while(alias != NULL)
   {
      if (!strncmp(alias->name, str, strlen(alias->name)))
      {
	 /* c est le caractère juste apres l'alias trouvé
	  * car il faut distinguer
	  * ls et lsblablia */
	 c = *(str+strlen(alias->name));
	 if ((c != '\0')&&(c != ' '))
	 {
	    alias = alias->next;
	    continue;
	 }
	 /* Si c'est un alias on alloue buffer */
	 buffer = malloc(sizeof(char) * (1+strlen(alias->cmd)+strlen(str+strlen(alias->name))));
	 sprintf(buffer, "%s%s", alias->cmd, str+strlen(alias->name));
	 break;
      }
      alias = alias->next;
   }
   if(buffer == NULL)
      buffer = saisie;
   argc = compter_mots(buffer, &buf_size);
   new->argc = argc+1;
   new->argv = malloc(sizeof(char*) * (new->argc));
   new->argc = parse_saisie(buffer, buf_size, new->argv);
   new->argc++;
   new->argv[new->argc] = NULL;
   if(buffer != saisie)
      free(buffer);
   /************/
   p = liste;
   if(p == NULL)
      return liste = new;
   while(p->next != NULL)
      p = p->next;
   p->next = new;
   return liste;
}

void free_file_instruction(file_instruction *liste)
{
   file_instruction *p, *q;
   int i;
   p = liste;
   while(p != NULL)
   {
      q = p;
      p = p->next;
      for(i = 0; i < q->argc; i++)
	 free(q->argv[i]);
      free(q->argv);
      if(q->file != NULL)
	 free(q->file);
      free(q);
   }
   return;
}

#ifdef DEBUG
void afficher_liste_instruction(file_instruction *liste)
{
   int i = 0, j;
   file_instruction *p;
   p = liste;
   while(p != NULL)
   {
      printf("Instruction %d :\n", i++);
      for(j = 0; j < p->argc; j++)
	 printf("\t\targv[%d] = '%s'\n", j, p->argv[j]);
      printf("Redirection : ");
      switch(p->red_type)
      {
	 case NONE:
	    printf("NONE");
	    break;
	 case RED_ADD:
	    printf("RED_ADD >> %s'", p->file);
	    break;
	 case RED_CREAT:
	    printf("RED_CREAT > %s'", p->file);
	    break;
	 default:
	    printf("ERREUR !!!");
	    break;
      }
      printf("\n");
      p = p->next;
   }
   return;
}
#endif



int parse_saisie(char *saisie, size_t buf_size, char **argv)
{
   char *p;
   p = saisie;
   char *buffer;
   int gui = 0;
   size_t i = 0, j = 0;
   buffer = malloc(sizeof(char) * buf_size+2);
   assert(buffer != NULL);
   while(*p == ' ')
      p++;
   for(;*p != '\0'; p++)
   {
      assert(i < buf_size+2);
      if(*p == '"'||*p == '\'')
      {
	 gui = (gui) ? 0 : 1;
	 continue;
      }
      if((*p == ' ')&&(!gui))
      {
	 buffer[i] = '\0';
	 argv[j] = malloc(sizeof(char) * (1+strlen(buffer)));
	 assert(argv[j] != NULL);
	 strcpy(argv[j], buffer);
#ifdef DEBUG
	 printf("argv[%d] = '%s'\n", j, argv[j]);
#endif
	 i = 0;
	 j++;
	 while(*p == ' ')
	    p++;
	 p--;
	 continue;
      }
      buffer[i] = *p;
      i++;
   }
   buffer[i] = '\0';
   argv[j] = malloc(sizeof(char) * (1+strlen(buffer)));
   assert(argv[j] != NULL);
   strcpy(argv[j], buffer);
#ifdef DEBUG
   printf("argv[%d] = '%s'\n", j, argv[j]);
#endif
   free(buffer);
   return j;
}

int compter_mots(char *saisie, size_t *lenght)
{
   char *p = strchr(saisie, '\0');
   int words = 0;
   size_t lenght_current = 0;
   size_t lenght_max = 0;
   int gui = 0;
   p--;
   while(*p == ' ')
   {
      *p = '\0';
      p--;
   }
   p = saisie;
   while(*p == ' ')
      p++;
   if(*p != '\0')
      words++;
   for(;*p != '\0';p++)
   {
      if(*p == '"'||*p == '\'')
      {
	 gui = (gui) ? 0 : 1;
	 continue;
      }
      if((*p == ' ')&&(!gui))
      {
	 if (lenght_current > lenght_max)
	    lenght_max = lenght_current;
	 lenght_current = 0;
	 words++;
	 while(*p == ' ')
	    p++;
	 p--;
	 continue;
      }
      lenght_current++;
   }
   if (lenght_current > lenght_max)
      lenght_max = lenght_current;
   *lenght = lenght_max;
   return words;
}

