#ifndef _PHILSH_HEADERS
#define _PHILSH_HEADERS

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



#endif /* _PHILSH_HEADER */
