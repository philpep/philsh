#ifndef EXEC_HEADERS
#define EXEC_HEADERS

int exec_cmd_external(char **argv);
int exec_cmd(int argc, char **argv);
void HandleInterrupt(int sig);
int WaitForChild (pid_t pid);

#define VERBOSE
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

#endif
