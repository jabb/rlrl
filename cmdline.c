
#include "cmdline.h"

#include <stdlib.h>

struct word {
	char *str;
	struct word *next;
};

int cmdline_parse(struct cmdline_args *args, const char *str)
{
	return 0;
}

void cmdline_free(struct cmdline_args *args)
{
	int i;
	for (i = 0; i < args->argc; ++i)
		free(args->argv[i]);
	free(args);
}
