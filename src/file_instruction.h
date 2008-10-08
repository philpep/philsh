#ifndef _FILE_INSTRUCTION_HEADER
#define _FILE_INSTRUCTION_HEADER

enum _redirection_type
{
   RED_ADD, RED_CREAT, NONE
};

typedef struct _file_instruction
{
   int argc;
   char **argv;
   struct _file_instruction *next;
   enum _redirection_type red_type;
   char *file;
} file_instruction;


file_instruction *creat_liste_instruction(char *saisie);
file_instruction *add_instruction(file_instruction *liste, char *saisie, enum _redirection_type type, char *file);
void free_file_instruction(file_instruction *liste);
#ifdef DEBUG
void afficher_liste_instruction(file_instruction *liste);
#endif
int parse_saisie(char *saisie, size_t buf_size, char **argv);
int compter_mots(char *saisie, size_t *lenght);

#endif /* _FILE_INSTRUCTION_HEADER */
