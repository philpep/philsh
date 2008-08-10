#ifndef INTERNAL_HEADER
#define INTERNAL_HEADER


char * get_current_dir(void);
int internal_cd(int argc, char **argv);
int internal_pwd(int argc, char **argv);
int internal_which(int argc, char **argv);
int which_cmd(char *str);
int internal_uname(int argc, char **argv);
int internal_setenv(char *str);
int internal_env(void);
int internal_ls(int argc, char **argv);

#endif
