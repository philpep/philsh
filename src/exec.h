#ifndef _EXEC_HEADERS
#define _EXEC_HEADERS

void HandleInterrupt(int sig);
int WaitForChild (pid_t pid, int *ret);
int exit_philsh(int argc, char **argv);
int jobs(int argc, char **argv);
int help(int argc, char **argv);

/* Liste chainée qui contient tous les jobs en background */
typedef struct lljobs lljobs;
struct lljobs
{
   char *name;
   pid_t pid;
   struct lljobs *next;
};
lljobs *add_job(lljobs *liste, char *name, pid_t pid);
int afficher_liste_jobs(lljobs *liste);
lljobs *del_job(lljobs *liste, pid_t pid);

int exec_file(file_instruction *liste);

#endif /* _EXEC_HEADERS */


