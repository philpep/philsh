#ifndef SHELL_HEADERS
#define SHELL_HEADERS

int make_argv(char *str, int i);
int exec_cmd(void);
char *get_prompt(void);
void HandleInterrupt(int sig);
int WaitForChild(pid_t pid);


#define SIZE 256

#endif
