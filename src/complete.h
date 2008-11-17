#ifndef _COMPLETE_HEARDER
#define _COMPLETE_HEARDER

char **command_names;
char *file_complete(char *str, unsigned int flags, char *prompt);
char *comand_complete(char *str, unsigned int flags, char *prompt);
void init_command_name(void);
/* au maximum 100 solutions de completions */
#define MAX_COMPLETION 100

#define VERBOSE 0
#define NOVERBOSE 2
#define DIR_ONLY 4

typedef struct _file_completion {
   char *name;
   unsigned char type;
   struct _file_completion *next;
} file_completion;

file_completion *add_file_completion(char *name, unsigned char type, file_completion *liste);
#endif /* _COMPLETE_HEARDER */
