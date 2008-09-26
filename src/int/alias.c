#include <stdio.h> /* Pour printf, fprintf */
#include <string.h> /* Pour strlen, strchr, strcmp, strcpy */
#include <stdlib.h> /* Pour malloc, NULL, free */
#include "alias.h"


/* Fonction qui est appelée lors de la création d'un alias */
int alias(int argc, char **argv)
{
   char *p;
   alias_ll *tmp;
   tmp = liste_alias;
   /* Manque d'arguments */
   if (argc != 2)
      return fprintf(stderr, "Philsh : Usage : alias [-p|name=\"command\"\n");
   /* L'option -p permet d'afficher les alias de la session en cours */
   if (!strcmp(argv[1], "-p"))
   {
      while(tmp != NULL)
      {
	 printf("'%s' aliased to '%s'\n", tmp->name, tmp->cmd);
	 tmp = tmp->next;
      }
      return 0;
   }
   /* Si la commande n'est pas de la forme alias machin=truc */
   if (NULL == (p = strchr(argv[1], '=')))
      return fprintf(stderr, "Philsh : Usage : alias [-p|name=\"command\"\n");
   /* On coupe la chaine au =, ainsi argv[1] = machin et p+1 = truc */
   *p = '\0';
   /* On teste si l'alias n'existe pas déjà */
   while(tmp != NULL)
   {
      if (!strcmp(tmp->name, argv[1]))
      {
	 /* Si l'alias existe, on le modifie à sa nouvelle valeur */
	 free(tmp->cmd);
	 tmp->cmd = malloc(sizeof(char) * (1+strlen(p+1)));
	 strcpy(tmp->cmd, p+1);
	 return 0;
      }
      tmp = tmp->next;
   }
   /* L'alias n'existe pas ---> on le crée */
   liste_alias = add_alias(liste_alias, argv[1], p+1);
   return 0;
}



/* Cette fonction crée l'alias name=cmd, il le rajoute
 * a la liste chainée des alias ... */
alias_ll *add_alias(alias_ll *liste, char *name, char *cmd)
{
   alias_ll *new = malloc(sizeof(alias_ll));
   new->name = malloc(sizeof(char) * (1+strlen(name)));
   new->cmd = malloc(sizeof(char) * (1+strlen(cmd)));
   strcpy(new->name, name);
   strcpy(new->cmd, cmd);
   new->next = liste;
   return new;
}

/* Cette fonction cherche un alias par son nom
 * et renvoie la commande.
 * f(machin) = truc */
char *search_alias(alias_ll *liste, char *name)
{
   alias_ll *tmp;
   tmp = liste;
   /* On parcoure simplement la liste */
   while(tmp != NULL)
   {
      if (!strcmp(tmp->name, name))
	 return tmp->cmd;
      tmp = tmp->next;
   }
   /* On à rien trouvé... */
   return NULL;
}
