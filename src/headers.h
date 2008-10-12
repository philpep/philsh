#ifndef _HEADER
#define _HEADERS

#define _BSD_SOURCE /* for setenv() definition */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <assert.h>
#include <getopt.h>

#include "philsh.h"
#include "philsh_build.h"
#include "prompt.h"
#include "int/internal.h"
#include "int/alias.h"
#include "readconfig.h"
#include "file_instruction.h"
#include "exec.h"

#endif /* _HEADER */
