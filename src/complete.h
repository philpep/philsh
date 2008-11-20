#ifndef _COMPLETE_HEARDER
#define _COMPLETE_HEARDER

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

file_completion *command_completion;
void init_command_name(void);
char *file_complete(char *str, unsigned int flags, char *prompt);
char *comand_complete(char *str, char *prompt);

file_completion *add_file_completion(char *name, unsigned char type, file_completion *liste);
void free_file_completion(file_completion *liste);
#endif /* _COMPLETE_HEARDER */
