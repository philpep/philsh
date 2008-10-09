#ifndef _INTERNAL_HEADER
#define _INTERNAL_HEADER

#include "err.h"

char * get_current_dir(void);
int internal_cd(int argc, char **argv);
int internal_pwd(int argc, char **argv);
int internal_which(int argc, char **argv);
int which(char **argv, int a, int ret);
char *which_cmd(char *str);
int internal_uname(int argc, char **argv);
int internal_setenv(char *str);
int internal_env(int argc, char **argv);
int whoami(int argc, char **argv);

/* Pour (*builtin).process */
enum Fork
{
   NEW_PROCESS, SAME_PROCESS
};

/* Structure des built-in commandes */
typedef struct builtin builtin;
struct builtin
{
	char *name; /* Le nom de la commande */
	int (*p)(int, char **); /* Pointeur sur la fonction à appeler */
	enum Fork process; /* SAME_PROCESS ou NEW_PROCESS */
};

extern const struct builtin builtin_command[];


#endif /* _INTERNAL_HEADER */
