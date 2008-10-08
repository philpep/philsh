#ifndef _ALIAS_HEADER
#define _ALIAS_HEADER

#include "err.h"

/* La structure des alias, c'est
 * une liste chainée.
 * alias machin=truc */
struct alias_ll
{
   char *name; /* Le nom de l'alias, machin */
   char *cmd; /* La commande de l'alias, truc */
   struct alias_ll *next; /* Le prochain alias */
};
/* alias_ll == struct alias_ll */
typedef struct alias_ll alias_ll;
/* Déclaration des fonctions */
int alias(int argc, char **argv);
int unalias(int argc, char **argv);
alias_ll *add_alias(alias_ll *liste, char *name, char *cmd);
int del_alias(char *name);
char *search_alias(alias_ll *liste, char *name);
alias_ll *liste_alias;

#endif /* _ALIAS_HEADER */
