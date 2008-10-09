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
file_instruction *creat_liste_instruction(char *saisie)
{
   char *p, *q, *r; /* Des pointeurs temporaires */
   int gui = 0; /* La variable qui permet de savoir si on est entre 2 guillemets */
   enum _redirection_type red_type; /* cf file_instruction.h */
   file_instruction *liste = NULL; /* La file qu'on va créer puis retourner */
   /* Une saisie NULL est un affront à cette fonction */
   if(saisie == NULL)
      return NULL;
   /* Dans un premier temps, on va separer les instructions
    * Je vous conseille de bien voir comment est construite la structure
    * file_instruction
    * Exemple ls -l /tmp >/dev/null; env >> tmp;echo "prout"
    * Se sépare en
    * ls -l /tmp RED_CREAT---> /dev/null
    * env RED_ADD---> tmp
    * echo "prout"
    * Ensuite On applique add_instruction()
    */

   /* En gros p parcoure la chaine et q est le debut de la sous chaine
    * que l'on veut extraire */
   q = saisie;
   for(p = saisie; *p != '\0'; p++)
   {
      if(*p == '"'||*p == '\'')
      {
	 gui = (gui) ? 0 : 1; /* Permet de passer gui a 0 s'il vaut 1 et inversement */
	 continue;
      }
      if(*p == ';'&&p != q)
      {
	 /* Un ; veut dire nouvelle instruction, on a pas trouvé de > avant
	  * donc il n'y a pas de redirections */
	 *p = '\0';
	 liste = add_instruction(liste, q, NONE, NULL);
	 q = ++p;
	 continue;
      }
      if(*p == '>'&&(!gui))
      {
	 *p = '\0';
	 if(*(++p) == '>')
	 {
	    p++;
	    red_type = RED_ADD; /* Redirection >> */
	 }
	 else
	    red_type = RED_CREAT; /* Redirection > */
	 /* Là ça devient vraiment complexe, car il faut extraire le nom du fichier
	  * dans lequel on veut rediriger. Mais il faut pourvoir faire tous les cas
	  * possibles    >fichier ou >       fichier     ;
	  * Bref, faut bien gérer les espaces... */
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
   /* Ajoute la dernière instruction */
   liste = add_instruction(liste, q, NONE, NULL);
   return liste;
}

/* Remplis une instruction et la colle en bout de file, c'est
 * ici qu'on affecte l'alias s'il existe */
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

/* Libère totalement la liste */
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

/* Fonction de traduction d'argc, argv en une liste d'instruction
 * (avec une seule entrée donc), pour l'instant elle est utilisée seulement
 * une fois, mais elle pourrait être utile par la suite */
file_instruction *Translate(int argc, char **argv)
{
   file_instruction *new = malloc(sizeof(file_instruction));
   new->argc = argc;
   new->argv = argv;
   new->file = NULL;
   new->red_type = NONE;
   return new;
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

/* Cette fonction retourne le nombre de mots d'une saisie
 * en prenant compte des " et '
 * Puis elle met la longeur du plus long
 * mot dans lenght, pour nous permettre de faire un malloc
 * efficace ensuite */
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

/* Cette fonction alloue argv selon le nombre de mot récuperé par
 * compter_mots(), puis alloue chaque argv[i] d'une taille de buf_size
 * qu'on a aussi récuperé par compter_mots et qui correspond a la
 * longeur du mot le plus long. Ainsi on est sur d'utiliser juste
 * ce qu'il faut en memoire */
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



