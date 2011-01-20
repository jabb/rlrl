
#include "gameman.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dungeon.h"
#include "glyph.h"
#include "shell.h"
#include "term.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) > (b) ? (b) : (a))
#define ABS(a) ((a) < 0 ? (-(a)) : (a))

#define CLAMP(a, min, max)	\
do {				\
	if (a < min)		\
		a = min;	\
	else if (a > max)	\
		a = max;	\
} while (0)

struct dungeon *curdun = NULL;
int curlvl = -1;

int px = 0;
int py = 0;
struct creature player;

static void descend(void);
static void draw(void);
static void act(struct creature *c, int a, int *x, int *y, int dx, int dy);

static int gm_move(struct shell *sh, int ac, char *av[]);
static int gm_act(struct shell *sh, int ac, char *av[]);
static int gm_update(struct shell *sh, int ac, char *av[]);
static int gm_dungeon(struct shell *sh, int ac, char *av[]);

int gm_init(void)
{
	creature_generate(&player);
	curdun = dungeon_create(GM_PLAY_WIDTH, GM_PLAY_HEIGHT);
	descend();
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

static void descend(void)
{
	curlvl++;
	dungeon_set_level(curdun, curlvl);
	dungeon_generate(curdun);
	dungeon_populate(curdun);
	dungeon_get_home(curdun, &px, &py);
	draw();
}

static void draw(void)
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

static void act(struct creature *c, int a, int *x, int *y, int dx, int dy)
{
	int range = 1;
	int capable_range = 1;
	int test_x = dx * capable_range;
	int test_y = dy * capable_range;
	while (dungeon_walkable(curdun, *x + test_x, *y + test_y) &&
		(*x + test_x != px || *y + test_y != py)) {

		capable_range++;
		test_x = dx * capable_range;
		test_y = dy * capable_range;

		if (capable_range > range)
			break;
	}
	capable_range--;

	*x += dx * capable_range;
	*y += dy * capable_range;
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
	int dx;
	int dy;
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

	if (player.actions[act_no] == ACT_NONE) {
		shell_printf(sh, "You don't have action %d!\n", act_no);
		return SHELL_SUCCESS;
	}

	dir = dir_from_name(av[2]);

	if (dir == DIR_NONE) {
		shell_printf(sh, "error: %s is not a direction\n", av[2]);
		return SHELL_SUCCESS;
	}

	dir_delta(dir, &dx, &dy);

	/* Now that we have all the information like action number and
	 * direction...
	 */

	act(&player, act_no, &px, &py, dx, dy);

	return SHELL_SUCCESS;
}


static int gm_update(struct shell *sh, int ac, char *av[])
{
	int dx;
	int dy;
	struct creature_node *iter;
	struct creature_list *cl;

	cl = dungeon_creature_list(curdun);

	for (cl_begin(cl); !cl_end(cl); cl_next(cl)) {
		iter = cl_iter(cl);
		dx = px - iter->x;
		dy = py - iter->y;
		CLAMP(dx, -1, 1);
		CLAMP(dy, -1, 1);
		act(&iter->creature, 0, &iter->x, &iter->y, dx, dy);
	}

	draw();
	return SHELL_SUCCESS;
}

static int gm_dungeon(struct shell *sh, int ac, char *av[])
{
	return SHELL_SUCCESS;
}

