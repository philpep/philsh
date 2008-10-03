#ifndef PHILSH_HEADERS
#define PHILSH_HEADERS

int options_philsh(int argc, char **argv);
int exec_saisie(char *saisie);
void afficher_aide(void);
void init_env(void);
int compter_mots(char *saisie, size_t *lenght);
int parse_saisie(char *saisie, size_t buf_size, char **argv);

/* Déclarations externes */
int setenv(const char *name, const char *value, int overwrite);
int putenv(char *string);


#endif /* PHILSH_HEADER */
