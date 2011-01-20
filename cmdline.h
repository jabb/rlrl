
#ifndef CMDLINE_H
#define CMDLINE_H

struct cmdline_args {
	int argc;
	char *argv[];
};

int cmdline_parse(struct cmdline_args *args, const char *str);
void cmdline_free(struct cmdline_args *args);

#endif
