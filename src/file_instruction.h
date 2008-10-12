#ifndef _FILE_INSTRUCTION_HEADER
#define _FILE_INSTRUCTION_HEADER


#define RED_ADD 0
#define RED_CREAT 2
#define NORED 4
#define PIPE 8
#define NOPIPE 16
#define AND 32
#define OR 64
#define NOCOND 128

typedef struct _file_instruction
{
   int argc;
   char **argv;
   unsigned int flags;
   char *file;
   struct _file_instruction *next;
} file_instruction;


file_instruction *creat_liste_instruction(char *saisie);
file_instruction *add_instruction(file_instruction *liste, char *saisie, char *file, unsigned int flags);
void free_file_instruction(file_instruction *liste);
#ifdef DEBUG
void afficher_liste_instruction(file_instruction *liste);
#endif
int parse_saisie(char *saisie, size_t buf_size, char **argv);
int compter_mots(char *saisie, size_t *lenght);

#endif /* _FILE_INSTRUCTION_HEADER */
