#ifndef ALIAS_HEADER
#define ALIAS_HEADER


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
alias_ll *add_alias(alias_ll *liste, char *name, char *cmd);
char *search_alias(alias_ll *liste, char *name);
alias_ll *liste_alias;

#endif /* ALIAS_HEADER */
