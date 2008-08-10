#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>

/* TODO : utiliser getopts */
int internal_uname(int argc, char **argv)
{
	struct utsname localhost;
	int i;
	uname(&localhost);
	if (argc == 1)
		printf("%s", localhost.sysname);
	else
		for (i = 0; i < argc; i++)
		{
			if (!strcmp(argv[i], "--kernel-name"))
				printf("%s", localhost.sysname);
			if (!strcmp(argv[i], "--nodename"))
				printf("%s", localhost.nodename);
			if (!strcmp(argv[i], "--release"))
				printf("%s", localhost.release);
			if (!strcmp(argv[i], "--kernel-version"))
				printf("%s", localhost.version);
			if (!strcmp(argv[i], "--machine"))
				printf("%s", localhost.machine);
		}
	printf("\n");
	return 0;
}
