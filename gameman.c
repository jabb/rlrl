
#include "gameman.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dungeon.h"
#include "glyph.h"
#include "shell.h"
#include "term.h"

struct dungeon *curdun = NULL;
int curlvl = -1;

int px = 0;
int py = 0;

static void gm_descend(void);
static void gm_draw(void);

static int gm_move(struct shell *sh, int ac, char *av[]);
static int gm_act(struct shell *sh, int ac, char *av[]);
static int gm_update(struct shell *sh, int ac, char *av[]);
static int gm_dungeon(struct shell *sh, int ac, char *av[]);

int gm_init(void)
{
	curdun = dungeon_create(GM_PLAY_WIDTH, GM_PLAY_HEIGHT);
	gm_descend();
	return 0;
}

void gm_exit(void)
{
	dungeon_destroy(curdun);
}

void gm_connect_shell(struct shell *sh)
{
	shell_add_cmd(sh, "move", gm_move);
	shell_add_cmd(sh, "act", gm_act);
	shell_add_cmd(sh, "update", gm_update);
	shell_add_cmd(sh, "dungeon", gm_dungeon);
}

static void gm_descend(void)
{
	curlvl++;
	dungeon_set_level(curdun, curlvl);
	dungeon_generate(curdun);
	dungeon_populate(curdun);
	dungeon_get_home(curdun, &px, &py);
	gm_draw();
}

static void gm_draw(void)
{
	int x, y;
	struct creature_node *iter;
	struct creature_list *cl;

	for (x = 0; x < dungeon_width(curdun); ++x) {
		for (y = 0; y < dungeon_height(curdun); ++y) {
			glyph_draw(x, y,
				tile_glyph(*dungeon_tile_at(curdun, x, y)));
		}
	}

	cl = dungeon_creature_list(curdun);

	for (cl_begin(cl); !cl_end(cl); cl_next(cl)) {
		iter = cl_iter(cl);
		glyph_draw(iter->x, iter->y, iter->creature.glyph);
	}


	glyph_draw(px, py, glyph_create('@'));

	term_flush();
}

static int gm_move(struct shell *sh, int ac, char *av[])
{
	int dx = 0;
	int dy = 0;
	int dir;

	if (ac != 2) {
		shell_puts(sh, "usage: move <direction>\n");
		shell_flush(sh);
		return SHELL_SUCCESS;
	}

	dir = dir_from_name(av[1]);

	if (dir == DIR_NONE) {
		shell_printf(sh, "unrecognized direction: %s\n", av[1]);
		shell_flush(sh);
		return SHELL_SUCCESS;
	}

	dir_delta(dir, &dx, &dy);

	if (dungeon_walkable(curdun, px + dx, py + dy)) {
		term_cursor_move(px, py);
		term_set_char(' ');

		px += dx;
		py += dy;

		shell_exec_line(sh, "update");
	}

	return SHELL_SUCCESS;
}

static int gm_act(struct shell *sh, int ac, char *av[])
{
	int dx = 0;
	int dy = 0;
	int dir;
	int act_no;
	char *tmp;

	if (ac <= 2) {
		shell_puts(sh, "usage: act <action_no> <direction>\n");
		return SHELL_SUCCESS;
	}

	act_no = strtol(av[1], &tmp, 10);

	if (tmp == av[1]) {
		shell_printf(sh, "error: %s is not an integer\n", av[1]);
		return SHELL_SUCCESS;
	}

	/* Special case, action 0 is always move. */
	if (act_no == 0) {
		shell_exec_linef(sh, "move %s", av[2]);
		return SHELL_SUCCESS;
	}

	dir = dir_from_name(av[2]);

	if (dir == DIR_NONE) {
		shell_printf(sh, "error: %s is not a direction\n", av[2]);
		return SHELL_SUCCESS;
	}

	dir_delta(dir, &dx, &dy);

	/* TODO: Deal with act_no */

	return SHELL_SUCCESS;
}


static int gm_update(struct shell *sh, int ac, char *av[])
{
	struct creature_node *iter;
	struct creature_list *cl;

	cl = dungeon_creature_list(curdun);

	for (cl_begin(cl); !cl_end(cl); cl_next(cl)) {
		iter = cl_iter(cl);
	}

	gm_draw();
	return SHELL_SUCCESS;
}

static int gm_dungeon(struct shell *sh, int ac, char *av[])
{
	return SHELL_SUCCESS;
}

