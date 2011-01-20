
#include "shell.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmdline.h"
#include "term.h"

#define PROMPT		"> "
#define PROMPT_LEN	(strlen(PROMPT))

#define PROMPT_X(sh)	((sh)->x)
#define PROMPT_Y(sh)	((sh)->y + (sh)->h - 1)

#define CMD_NAME_LEN	32

#define BUF_HEIGHT(sh)	((sh)->h - 1)
#define BUF_WIDTH(sh)	((sh)->w)

#define ARG_LEN		32
#define MAX_ARGS	32

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

struct shell {
	int x;
	int y;
	int w;
	int h;
	struct cmd {
		char name[CMD_NAME_LEN];
		shell_cmd *fn;
		struct cmd *next;
	} *cmds;
	int wr_row;
	int wr_col;
	char **buf;

	int wlines; /* Written lines. */
	int entered;
};

/******************************************************************************\
 *	INTERNAL FUNCTIONS
\******************************************************************************/

static void write_ch(struct shell *sh, int c)
{
	int i;
	int tmp;

	if (sh->wr_col >= BUF_WIDTH(sh)) {
		sh->wr_col = 0;
		sh->wr_row++;
		sh->wlines++;
	}

	if (sh->wr_row >= BUF_HEIGHT(sh)) {
		for (i = 0; i < BUF_HEIGHT(sh) - 1; ++i) {
			memmove(sh->buf[i], sh->buf[i + 1], BUF_WIDTH(sh));
		}
		sh->wr_row--;
	}

	switch (c) {
	case '\n':
		for (i = sh->wr_col; i < BUF_WIDTH(sh); ++i)
			sh->buf[sh->wr_row][i] = ' ';
		sh->wr_col = 0;
		sh->wr_row++;
		sh->wlines++;
		break;
	case '\t':
		tmp = 4 - ((sh->wr_col + 1) % 4) + 1;
		for (i = sh->wr_col;
			i < MIN(sh->wr_col + tmp, BUF_WIDTH(sh)); ++i)
			sh->buf[sh->wr_row][i] = ' ';
		sh->wr_col += tmp;
		break;
	default:
		sh->buf[sh->wr_row][sh->wr_col] = c;
		sh->wr_col++;
		break;
	}
}

static void clear_prompt(struct shell *sh)
{
	int x = PROMPT_X(sh);
	int y = PROMPT_Y(sh);
	int i;
	for (i = 0; i < sh->w; ++i) {
		term_cursor_move(x + i, y);
		term_set_char(' ');
	}
}

static void clearbuf(struct shell *sh)
{
	int y;
	int x;
	for (y = 0; y < BUF_HEIGHT(sh); ++y) {
		for (x = 0; x < BUF_WIDTH(sh); ++x) {
			sh->buf[y][x] = ' ';
		}
	}
	sh->wr_row = 0;
	sh->wr_col = 0;
}

/******************************************************************************\
 *	BASIC SHELL FUNCTIONS
\******************************************************************************/

static int cmd_comment(struct shell *sh, int ac, char *av[])
{
	(void)ac;
	(void)av;
	(void)sh;
	return SHELL_SUCCESS;
}

static int cmd_clear(struct shell *sh, int ac, char *av[])
{
	(void)ac;
	(void)av;
	clearbuf(sh);
	return SHELL_SUCCESS;
}

static int cmd_exit(struct shell *sh, int ac, char *av[])
{
	(void)ac;
	(void)av;
	sh->entered = 0;
	return SHELL_SUCCESS;
}

static int cmd_echo(struct shell *sh, int ac, char *av[])
{
	int i;

	for (i = 1; i < ac; ++i) {
		shell_puts(sh, av[i]);
		shell_puts(sh, " ");
	}
	shell_puts(sh, "\n");
	shell_flush(sh);
	return SHELL_SUCCESS;
}

static int cmd_pause(struct shell *sh, int ac, char *av[])
{
	(void)ac;
	(void)av;

	shell_puts(sh, "Press any key to continue...\n");
	shell_flush(sh);
	term_get_char();
	return SHELL_SUCCESS;
}

static int cmd_runfile(struct shell *sh, int ac, char *av[])
{
	char buf[BUFSIZ];
	FILE *fin;

	(void)ac;
	(void)av;

	if (ac != 2) {
		shell_puts(sh, "usage: run <file>\n");
		shell_flush(sh);
		return SHELL_SUCCESS;
	}

	fin = fopen(av[1], "r");
	if (!fin) {
		shell_printf(sh, "error: couldn't open %s\n", av[1]);
		return SHELL_SUCCESS;
	}

	while (fgets(buf, BUFSIZ, fin)) {
		if (strlen(buf) <= 1)
			continue;
		buf[strlen(buf) - 1] = '\0';
		if (shell_exec_linef(sh, "%s", buf) == SHELL_FAILURE) {
			shell_printf(sh, "error: '%s' failed\n", buf);
			break;
		}
	}

	fclose(fin);

	return SHELL_SUCCESS;
}

static int cmd_runcmds(struct shell *sh, int ac, char *av[])
{
	int i;

	for (i = 1; i < ac; ++i) {
		if (shell_exec_linef(sh, "%s", av[i]) == SHELL_FAILURE) {
			shell_printf(sh, "error: '%s' failed\n", av[i]);
		}
	}

	return SHELL_SUCCESS;
}

static int cmd_sh(struct shell *sh, int ac, char *av[])
{
	(void)ac;
	(void)av;

	shell_enter(sh);
	return SHELL_SUCCESS;
}

static int cmd_list(struct shell *sh, int ac, char *av[])
{
	int lines;
	struct cmd *head = sh->cmds;

	if (!head)
		return SHELL_SUCCESS;

	lines = shell_lines_written(sh);
	do {
		shell_printf(sh, "%s\n", head->name);

		if (shell_lines_written(sh) - lines >=
			shell_get_height(sh) - 1) {
			shell_exec_line(sh, "pause");
			lines = shell_lines_written(sh);
		}

		head = head->next;
	} while (head != NULL);

	return SHELL_SUCCESS;
}

/******************************************************************************\
 *	ENTRY FUNCTIONS
\******************************************************************************/

struct shell *shell_create(int x, int y, int w, int h)
{
	int i;
	struct shell *sh = NULL;

	if (w * h <= 0)
		return NULL;

	sh = malloc(sizeof(struct shell));

	if (!sh)
		goto alloc_fail;

	sh->x = x;
	sh->y = y;
	sh->w = w;
	sh->h = h;
	sh->cmds = NULL;

	sh->buf = calloc(BUF_HEIGHT(sh), sizeof(char *));

	if (!sh->buf)
		goto alloc_fail;

	for (i = 0; i < BUF_HEIGHT(sh); ++i) {
		sh->buf[i] = calloc(BUF_WIDTH(sh), sizeof(char));
		if (!sh->buf[i])
			goto array_alloc_fail;
	}

	sh->wlines = 0;
	sh->entered = 0;

	clearbuf(sh);

	goto success;
array_alloc_fail:
	for (i = 0; i < BUF_HEIGHT(sh); ++i)
		free(sh->buf[i]);
	free(sh->buf);
alloc_fail:
	free(sh);
	sh = NULL;
success:
	return sh;
}

void shell_destroy(struct shell *sh)
{
	int i;
	struct cmd *prev = NULL, *head = NULL;

	for (i = 0; i < BUF_HEIGHT(sh); ++i)
		free(sh->buf[i]);
	free(sh->buf);

	prev = NULL;
	head = sh->cmds;

	while (head) {
		prev = head;
		head = head->next;
		free(prev);
	}

	free(sh);
}

int shell_get_width(struct shell *sh)
{
	return BUF_HEIGHT(sh);
}

int shell_get_height(struct shell *sh)
{
	return BUF_WIDTH(sh);
}

int shell_lines_written(struct shell *sh)
{
	return sh->wlines;
}

int shell_add_cmd(struct shell *sh, const char *name, shell_cmd *fn)
{
	struct cmd *iter =NULL;
	struct cmd *c = NULL;

	/* Remove just in case it exists. */
	shell_remove_cmd(sh, name);

	c = malloc(sizeof(struct cmd));
	if (!c)
		return SHELL_FAILURE;

	strncpy(c->name, name, CMD_NAME_LEN);
	c->fn = fn;
	c->next = NULL;

	if (sh->cmds) {
		iter = sh->cmds;

		while (iter->next)
			iter = iter->next;

		iter->next = c;
	}
	else {
		sh->cmds = c;
	}

	return SHELL_SUCCESS;
}

int shell_remove_cmd(struct shell *sh, const char *name)
{
	struct cmd *prev = NULL, *head = sh->cmds;

	if (!head)
		return -1;

	do {
		if (strcmp(name, head->name) == 0) {
			if (prev) {
				prev->next = head->next;
			}
			else {
				sh->cmds = head->next;
			}

			free(head);
			return SHELL_SUCCESS;
		}

		prev = head;
		head = head->next;
	} while (head != NULL);

	return SHELL_FAILURE;
}

int shell_flush(struct shell *sh)
{
	int y;
	int x;
	for (y = 0; y < BUF_HEIGHT(sh); ++y) {
		for (x = 0; x < BUF_WIDTH(sh); ++x) {
			term_cursor_move(sh->x + x, sh->y + y);
			term_set_char(sh->buf[y][x]);
		}
	}
	term_flush();
	return SHELL_SUCCESS;
}

int shell_puts(struct shell *sh, const char *buf)
{
	while (*buf) {
		write_ch(sh, *buf);
		buf++;
	}
	return SHELL_SUCCESS;
}

int shell_printf(struct shell *sh, const char *fmt, ...)
{
	char buf[BUFSIZ];

	va_list args;
	va_start(args, fmt);

	vsnprintf(buf, BUFSIZ, fmt, args);
	shell_puts(sh, buf);

	va_end(args);
	return SHELL_SUCCESS;
}

int shell_gets(struct shell *sh, char *buf, int siz)
{
	int ret, minsiz;

	/* Flush the output first, just like a normal terminal. */
	shell_flush(sh);

	minsiz = MIN(siz, sh->w - PROMPT_LEN);

	term_cursor_move(PROMPT_X(sh), PROMPT_Y(sh));
	term_set_string(PROMPT);

	term_cursor_move(PROMPT_X(sh) + PROMPT_LEN, PROMPT_Y(sh));
	ret = term_get_string(buf, minsiz);

	clear_prompt(sh);
	term_flush();

	buf[minsiz] = '\0';
	return ret;
}

int shell_add_default_cmds(struct shell *sh)
{
	shell_add_cmd(sh, "#", cmd_comment);
	shell_add_cmd(sh, "clear", cmd_clear);
	shell_add_cmd(sh, "exit", cmd_exit);
	shell_add_cmd(sh, "echo", cmd_echo);
	shell_add_cmd(sh, "pause", cmd_pause);
	shell_add_cmd(sh, "runfile", cmd_runfile);
	shell_add_cmd(sh, "runcmds", cmd_runcmds);
	shell_add_cmd(sh, "list", cmd_list);
	shell_add_cmd(sh, "sh", cmd_sh);
	return SHELL_SUCCESS;
}

int shell_exec(struct shell *sh, const char *name, int ac, char *av[])
{
	struct cmd *head = sh->cmds;

	if (!head)
		return SHELL_NOT_FOUND;

	do {
		if (strcmp(name, head->name) == 0) {
			return head->fn(sh, ac, av);
		}

		head = head->next;
	} while (head != NULL);

	return SHELL_NOT_FOUND;
}


int shell_exec_line(struct shell *sh, const char *line)
{
	int rv;
	struct cmdline_args *args;

	args = cmdline_parse(line);
	if (!args)
		return SHELL_FAILURE;

	rv = shell_exec(sh, args->argv[0], args->argc, args->argv);

	if (rv == SHELL_NOT_FOUND)
		shell_printf(sh, "%s: no such command\n", args->argv[0]);

	cmdline_free(args);

	return rv;
}

int shell_exec_linef(struct shell *sh, const char *line, ...)
{
	int rv;
	char buf[BUFSIZ];

	va_list args;
	va_start(args, line);

	vsnprintf(buf, BUFSIZ, line, args);
	rv = shell_exec_line(sh, buf);

	va_end(args);
	return rv;
}

int shell_enter(struct shell *sh)
{
	char buf[80];

	sh->entered = 1;

	do {
		shell_gets(sh, buf, 80);

		if (strlen(buf) == 0)
			break;

		shell_printf(sh, "> %s\n", buf);
		shell_exec_line(sh, buf);

		shell_flush(sh);
	} while (sh->entered);

	return SHELL_SUCCESS;
}
