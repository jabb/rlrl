
#include <stdlib.h>
#include <string.h>

#include "gameman.h"
#include "rand.h"
#include "shell.h"
#include "term.h"

#define WIDTH	80
#define HEIGHT	24

#define NUM_KEYBINDS	256

struct shell *sh = NULL;

char *mystrdup(const char *s)
{
	char *tmp = malloc(strlen(s) + 1);

	if (!tmp)
		return NULL;

	strcpy(tmp, s);

	return tmp;
}

char *keybinds[NUM_KEYBINDS] = {0};

int main_init(void);
void main_exit(int code);

int sh_map(struct shell *sh, int ac, char *av[]);
int sh_unmap(struct shell *sh, int ac, char *av[]);
int sh_commands(struct shell *sh, int ac, char *av[]);
int sh_quit(struct shell *sh, int ac, char *av[]);
int sh_rand(struct shell *sh, int ac, char *av[]);

int main(void)
{
	int ch = -1;

	main_init();

	shell_add_cmd(sh, "map", sh_map);
	shell_add_cmd(sh, "unmap", sh_unmap);
	shell_add_cmd(sh, "commands", sh_commands);
	shell_add_cmd(sh, "quit", sh_quit);
	shell_add_cmd(sh, "rand", sh_rand);

	shell_exec_line(sh, "map ` sh");
	shell_exec_line(sh, "map ~ sh");
	shell_exec_line(sh, "runfile init.rlc");

	gm_connect_shell(sh);

	shell_printf(sh, "Welcome to RLRL!\n"
			"Press ` or ~ to enter the shell.\n");

	shell_flush(sh);
	do {
		if (ch >= 0 && ch < NUM_KEYBINDS && keybinds[ch]) {
			shell_exec_line(sh, keybinds[ch]);
			shell_flush(sh);
		}
		ch = term_get_char();
	} while (1);

	main_exit(0);
}

int main_init(void)
{
	term_open();
	term_cursor_show(0);
	gm_init();

	sh = shell_create(GM_PLAY_WIDTH, 0, WIDTH - GM_PLAY_WIDTH, HEIGHT);

	shell_add_default_cmds(sh);

	rand_init_time();

	return 0;
}

void main_exit(int code)
{
	int i;
	shell_destroy(sh);
	term_close();
	gm_exit();

	for (i = 0; i < NUM_KEYBINDS; ++i)
		free(keybinds[i]);

	exit(code);
}

int sh_map(struct shell *sh, int ac, char *av[])
{
	int force = 0;
	int silent = 0;
	int arg;

	for (arg = 1; arg < ac; ++arg) {
		if (strcmp(av[arg], "-f") == 0)
			force = 1;
		else if (strcmp(av[arg], "-s") == 0)
			silent = 1;
		else
			break;
	}

	if (ac < arg + 2) {
		shell_puts(sh, "usage: map <options?> <char> \"<command>\"\n");
		shell_flush(sh);
		return SHELL_SUCCESS;
	}

	if (av[arg][0] >= 0 && av[arg][0] < NUM_KEYBINDS) {
		if (keybinds[(int)av[arg][0]] && !force) {
			if (!silent)
				shell_printf(sh, "'%c' is already mapped!\n",
					av[arg][0]);
			shell_flush(sh);
			return SHELL_SUCCESS;
		}
		else {
			keybinds[(int)av[arg][0]] = mystrdup(av[arg + 1]);
			if (!silent)
				shell_printf(sh, "'%c' successfully bound!\n",
					av[arg][0]);
		}
	}

	return SHELL_SUCCESS;
}

int sh_unmap(struct shell *sh, int ac, char *av[])
{
	if (ac != 2) {
		shell_puts(sh, "usage: unmap <char>\n");
		shell_flush(sh);
		return SHELL_SUCCESS;
	}

	if (av[1][0] >= 0 && av[1][0] < NUM_KEYBINDS) {
		free(keybinds[(int)av[1][0]]);
		keybinds[(int)av[1][0]] = NULL;
		shell_printf(sh, "'%c' successfully unbound!\n", av[1][0]);
		shell_flush(sh);
	}

	return SHELL_SUCCESS;
}

int sh_commands(struct shell *sh, int ac, char *av[])
{
	int i;
	int lines = shell_lines_written(sh);

	for (i = 0; i < NUM_KEYBINDS; ++i) {
		if (keybinds[i]) {
			shell_printf(sh, "'%c': \"%s\"\n", i, keybinds[i]);
			if (shell_lines_written(sh) - lines >=
				shell_get_height(sh) - 1) {
				shell_exec_line(sh, "pause");
				lines = shell_lines_written(sh);
			}
		}
	}
	shell_flush(sh);
	return SHELL_SUCCESS;
}

int sh_quit(struct shell *sh, int ac, char *av[])
{
	int i;
	int ch;
	int prompt = 0;

	for (i = 1; i < ac; ++i) {
		if (strcmp(av[i], "-p") == 0)
			prompt = 1;
	}

	if (prompt) {
		shell_puts(sh, "Are you sure? (y/n)\n");
		shell_flush(sh);
		ch = term_get_char();
		if (ch != 'y' && ch != 'Y') {
			shell_puts(sh, "Okay!\n");
			shell_flush(sh);
			return SHELL_SUCCESS;
		}
	}

	main_exit(0);

	return SHELL_SUCCESS;
}

int sh_rand(struct shell *sh, int ac, char *av[])
{
	int lo, hi;
	char *tmp;

	if (ac < 2) {
		shell_puts(sh, "usage: rand <seed|int|double|range>\n");
		return SHELL_SUCCESS;
	}

	if (strcmp("seed", av[1]) == 0) {
		shell_printf(sh, "%lu\n", rand_seed());
	}
	else if (strcmp("int", av[1]) == 0) {
		shell_printf(sh, "%lu\n", rand_int());
	}
	else if (strcmp("double", av[1]) == 0) {
		shell_printf(sh, "%lf\n", rand_double());
	}
	else if (strcmp("range", av[1]) == 0) {
		if (ac != 4) {
			shell_puts(sh, "usage: rand range <lo> <hi>\n");
			return SHELL_SUCCESS;
		}

		lo = strtol(av[2], &tmp, 10);
		if (av[2] == tmp) {
			shell_puts(sh, "usage: rand range <lo> <hi>\n");
			return SHELL_SUCCESS;
		}

		hi = strtol(av[3], &tmp, 10);
		if (av[3] == tmp) {
			shell_puts(sh, "usage: rand range <lo> <hi>\n");
			return SHELL_SUCCESS;
		}

		shell_printf(sh, "%lu\n", rand_range(lo, hi));
	}
	else {
		shell_puts(sh, "usage: rand <seed|int|double|range>\n");
	}
	return SHELL_SUCCESS;
}

