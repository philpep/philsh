#ifndef _PHILSH_HEADERS
#define _PHILSH_HEADERS

int options_philsh(int argc, char **argv);
void afficher_aide(void);
void init_env(void);
void philsh(void);
void mode_raw(int activer);

#define SIZE_SAISIE 256

#endif /* _PHILSH_HEADER */
