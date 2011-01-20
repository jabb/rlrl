
#ifndef CMDLINE_H
#define CMDLINE_H

struct cmdline_args {
	int argc;
	char *argv[];
};

struct cmdline_args *cmdline_parse(const char *str);
void cmdline_free(struct cmdline_args *args);

#endif
