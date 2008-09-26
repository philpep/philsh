#ifndef READCONFIG_HEADER
#define READCONFIG_HEADER

#define DEFAULT_CONFIG 1
#define CUSTOM_CONFIG 0

int init_config(char *config_file);
int parse_file(char *ptr);

#endif
