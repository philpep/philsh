#ifndef _READCONFIG_HEADER
#define _READCONFIG_HEADER

#define DEFAULT_CONFIG 1
#define CUSTOM_CONFIG 0

int init_config(char *config_file);
int parse_file(char *ptr);
int source(int argc, char **argv);

#endif /* _READCONFIG_HEADER */
