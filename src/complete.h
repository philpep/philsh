#ifndef _COMPLETE_HEARDER
#define _COMPLETE_HEARDER

char **command_names;
char *file_complete(char *str);
char *comand_complete(char *str);
void init_command_name(void);
/* au maximum 100 solutions de completions */
#define MAX_COMPLETION 100

#endif /* _COMPLETE_HEARDER */
