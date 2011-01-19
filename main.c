
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "gameman.h"
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

unsigned long seed;

int main_init(void);
void main_exit(int code);

int sh_map(struct shell *sh, int ac, char *av[]);
int sh_unmap(struct shell *sh, int ac, char *av[]);
int sh_commands(struct shell *sh, int ac, char *av[]);
int sh_quit(struct shell *sh, int ac, char *av[]);
int sh_seed(struct shell *sh, int ac, char *av[]);

int main(void)
{
	int ch = -1;

	main_init();

	shell_add_cmd(sh, "map", sh_map);
	shell_add_cmd(sh, "unmap", sh_unmap);
	shell_add_cmd(sh, "commands", sh_commands);
	shell_add_cmd(sh, "quit", sh_quit);
	shell_add_cmd(sh, "seed", sh_seed);

	shell_exec_line(sh, "map ` sh");
	/*shell_exec_line(sh, "map ? commands");
	shell_exec_line(sh, "map Q \"quit -p\"");*/

	shell_exec_line(sh, "run init.rlc");

	gm_connect_shell(sh);

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

	seed = (unsigned)time(NULL);
	srand(seed);

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

	exit(0);
}

int sh_map(struct shell *sh, int ac, char *av[])
{
	if (ac != 3) {
		shell_puts(sh, "usage: map <char> \"<command>\"\n");
		shell_flush(sh);
		return SHELL_SUCCESS;
	}

	if (av[1][0] >= 0 && av[1][0] < NUM_KEYBINDS) {
		if (keybinds[(int)av[1][0]]) {
			shell_printf(sh, "'%c' is already mapped!\n", av[1][0]);
			shell_flush(sh);
			return SHELL_SUCCESS;
		}
		else {
			keybinds[(int)av[1][0]] = mystrdup(av[2]);
			shell_printf(sh, "'%c' successfully bound!\n", av[1][0]);
		}
	}

	return SHELL_SUCCESS;
}

int sh_unmap(struct shell *sh, int ac, char *av[])
{
	if (ac != 2) {
		shell_puts(sh, "usage: unmap <char>");
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
	int lines = 0;

	for (i = 0; i < NUM_KEYBINDS; ++i) {
		if (keybinds[i]) {
			shell_printf(sh, "'%c': \"%s\"\n", i, keybinds[i]);
			if (lines >= shell_get_height(sh) - 1) {
				shell_exec_line(sh, "pause");
				lines = 0;
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

int sh_seed(struct shell *sh, int ac, char *av[])
{
	shell_printf(sh, "random seed: %lu\n", seed);
	return SHELL_SUCCESS;
}

