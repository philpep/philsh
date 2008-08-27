#ifndef HEADERS
#define HEADERS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <assert.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "shell.h"
#include "int/internal.h"
#include "philsh_build.h"

void config_init(void);


#endif
