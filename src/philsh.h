#ifndef _PHILSH_HEADERS
#define _PHILSH_HEADERS

#define DEBUG
int options_philsh(int argc, char **argv);
void afficher_aide(void);
void init_env(void);
void philsh(void);
void mode_raw(int activer);

#define SIZE_SAISIE 512

/* Definitions des touches renvoyées par getchar() */
#define PHILSH_KEY_TAB 9
#define PHILSH_KEY_CTRLD 4
#define PHILSH_KEY_CTRLC 3
#define PHILSH_KEY_CTRLZ 26
#define PHILSH_KEY_ENTER 13
#define PHILSH_KEY_BACK 127
#define PHILSH_KEY_SPECIALS 27
#define PHILSH_KEY_UP 65
#define PHILSH_KEY_DOWN 66

/* efface la saisie de l'ecran */
#define clear_saisie() while(i != 0) \
{ \
   printf("\b \b"); \
   i--; \
} \

typedef struct _philsh_history
{
   char *cmd;
   struct _philsh_history *next;
   struct _philsh_history *prev;

} philsh_history;

philsh_history *add_to_history(char *cmd, philsh_history *liste);
void free_history(philsh_history *liste);
void print_history(philsh_history *liste);




#endif /* _PHILSH_HEADER */
