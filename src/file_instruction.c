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

/* Cette fonction prend une chaine brute et la transforme
 * en une liste d'instructions
 * Elle est assez complexe, mais elle fonctionne plutôt bien */
/* {{{ creat_liste_instruction() */
file_instruction *creat_liste_instruction(char *saisie)
{
   char *p, *q, *r;
   int gui = 0;
   unsigned int flags;
   file_instruction *liste = NULL;
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
      if(*p == ';'&&p != q&&!gui)
      {
	 *p = '\0';
	 flags = NORED | NOPIPE | NOCOND;
	 liste = add_instruction(liste, q, NULL, flags);
	 q = ++p;
	 continue;
      }
      if(*p == '>'&&(!gui))
      {
	 *p = '\0';
	 if(*(++p) == '>')
	 {
	    p++;
	    flags = RED_ADD | NOPIPE | NOCOND;
	 }
	 else
	    flags = RED_CREAT | NOPIPE | NOCOND;
	 while(*p == ' ')
	    p++;
	 r = p;
	 while(*r != '\0'&&*r != ' '&&*r != ';'&&*r != '|' &&*r != '&')
	    r++;
	 if(*r == '\0')
	    return liste = add_instruction(liste, q, p, flags);
	 if(*r == '|')
	 {
	    flags &= ~NOPIPE;
	    flags |= PIPE;
	 }
	 if(*r == '&')
	 {
	    flags &= ~NOCOND;
	    flags |= AND;
	 }
	 *r = '\0';
	 r++;
	 while(*r == ' ')
	    r++;
	 if(*r == '|')
	 {
	    if(*(r+1) == '|')
	    {
	       flags &= ~NOCOND;
	       flags |= OR;
	       r++;
	    }
	    else
	    {
	       flags &= ~NOPIPE;
	       flags |= PIPE;
	    }
	 }
	 if(*r == '&')
	 {
	    if(*(r+1) == '&')
	    {
	       flags &= ~NOCOND;
	       flags |= AND;
	       r++;
	    }
	    else
	    {
	       flags |= BG;
	       flags &= ~AND;
	    }
	 }
	 liste = add_instruction(liste, q, p, flags);
	 p = ++r;
	 while(*p == ' '||*p == ';')
	    p++;
	 q = p;
	 continue;
      }
      if(*p == '|'&&(!gui))
      {
	 *p = '\0';
	 if(*(++p) == '|')
	 {
	    p++;
	    flags = NORED | NOPIPE | OR;
	 }
	 else
	    flags = NORED | PIPE | NOCOND;
	 liste = add_instruction(liste, q, NULL, flags);
	 while(*p == ' ')
	    p++;
	 q = p;
	 continue;
      }
      if(*p == '&' && (!gui))
      {
	 *p = '\0';
	 if(*(++p) == '&')
	 {
	    p++;
	    flags = NORED | NOPIPE | AND;
	 }
	 else
	    flags = NORED | NOPIPE | NOCOND | BG;
	 liste = add_instruction(liste, q, NULL, flags);
	 while(*p == ' ')
	    p++;
	 q = p;
	 continue;
      }
   }
   liste = add_instruction(liste, q, NULL, NOPIPE | NORED | NOCOND);
   return liste;
}
/* }}} */

/* Remplis une instruction et la colle en bout de file, c'est
 * ici qu'on affecte l'alias s'il existe */
/* {{{ add_instruction() */
file_instruction *add_instruction(file_instruction *liste, char *saisie, char *file, unsigned int flags)
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
   /* On detecte si la commande est un path
    * (ie : ne pas la chercher dans $PATH) */
   if('/' == new->argv[0][0] || '.' == new->argv[0][0])
      new->flags = flags | LOCAL_CMD;
   else
      new->flags = flags;
   /************/
   p = liste;
   if(p == NULL)
      return liste = new;
   while(p->next != NULL)
      p = p->next;
   p->next = new;
   return liste;
}
/* }}} */

/* Libère totalement la liste */
/* {{{ free_file_instruction() */
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
/* }}} */

/* Affiche la liste sur stdin pour debug */
#ifdef DEBUG
/* {{{ afficher_liste_instruction() */
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
      printf("Redirection = ");
      if(p->flags & RED_ADD)
	 printf("RED_ADD >> '%s' ", p->file);
      if(p->flags & RED_CREAT)
	 printf("RED_CREAT > '%s' ", p->file);
      if(p->flags & NORED)
	 printf("NORED ");
      if(p->flags & PIPE)
	 printf("PIPE ");
      if(p->flags & NOPIPE)
	 printf("NOPIPE ");
      if(p->flags & AND)
	 printf("AND ");
      if(p->flags & OR)
	 printf("OR ");
      if(p->flags & NOCOND)
	 printf("NOCOND ");
      if(p->flags & LOCAL_CMD)
	 printf("LOCAL_CMD ");
      if(p->flags & BG)
	 printf("BG ");
      printf("\n");
      p = p->next;
   }
   return;
}
/* }}} */
#endif

/* Cette fonction retourne le nombre de mots d'une saisie
 * en prenant compte des " et '
 * Puis elle met la longeur du plus long
 * mot dans lenght, pour nous permettre de faire un malloc
 * efficace ensuite */
/* {{{ compter_mots() */
int compter_mots(char *saisie, size_t *lenght)
{
   char *p = strchr(saisie, '\0');
   int words = 0, gui = 0;
   size_t lenght_current = 0,lenght_max = 0;
   p--;
   /* On virre tous les espaces de la fin */
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
/* }}} */

/* Cette fonction alloue argv selon le nombre de mot récuperé par
 * compter_mots(), puis alloue chaque argv[i] d'une taille de buf_size
 * qu'on a aussi récuperé par compter_mots et qui correspond a la
 * longeur du mot le plus long. Ainsi on est sur d'utiliser juste
 * ce qu'il faut en memoire */
/* {{{ parse_saisie() */
int parse_saisie(char *saisie, size_t buf_size, char **argv)
{
   char *p, *buffer;
   int gui = 0;
   size_t i = 0, j = 0;
   p = saisie;
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
/* }}} */



