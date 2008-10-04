#ifndef INTERNAL_HEADER
#define INTERNAL_HEADER

#include "err.h"

char * get_current_dir(void);
int internal_cd(int argc, char **argv);
int internal_pwd(int argc, char **argv);
int internal_which(int argc, char **argv);
int which(char **argv, int a, int ret);
int which_cmd(char *str);
int internal_uname(int argc, char **argv);
int internal_setenv(char *str);
int internal_env(int argc, char **argv);
int whoami(int argc, char **argv);
/* Déclarations externes */
int setenv (const char *name, const char *value, int overwrite);
char *get_current_dir_name(void);

/* Structure des built-in commandes */
typedef struct builtin builtin;
struct builtin
{
	char *name; /* Le nom de la commande */
	int (*p)(int, char **); /* Pointeur sur la fonction à appeler */
	int fork; /* 0 s'il faut lancer dans un nouveau processus */
};

extern const struct builtin builtin_command[];


#endif
