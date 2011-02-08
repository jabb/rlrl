
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

/******************************************************************************\
 * Prototypes
\******************************************************************************/

static void descend(void);
static void draw(void);
struct creature *act_walk(unsigned long action, int *x, int *y, int dx, int dy);

static int gm_interact(struct shell *sh, int ac, char *av[]);
static int gm_move(struct shell *sh, int ac, char *av[]);
static int gm_act(struct shell *sh, int ac, char *av[]);
static int gm_update(struct shell *sh, int ac, char *av[]);
static int gm_dungeon(struct shell *sh, int ac, char *av[]);

/******************************************************************************\
 * Management
\******************************************************************************/

struct dungeon *curdun = NULL;
int curlvl = -1;

int gm_init(void)
{
	curdun = dungeon_create(GM_PLAY_WIDTH, GM_PLAY_HEIGHT);
	creature_generate(dungeon_get_player(curdun));
	dungeon_set_player_xy(curdun, 0, 0);
	descend();
	return 0;
}

void gm_exit(void)
{
	dungeon_destroy(curdun);
}

void gm_connect_shell(struct shell *sh)
{
	shell_add_cmd(sh, "interact", gm_interact);
	shell_add_cmd(sh, "move", gm_move);
	shell_add_cmd(sh, "act", gm_act);
	shell_add_cmd(sh, "update", gm_update);
	shell_add_cmd(sh, "dungeon", gm_dungeon);
}

/******************************************************************************\
 * Dungeon-ey stuff, like pathfinding and combat.
\******************************************************************************/

static void descend(void)
{
	int tx, ty;

	curlvl++;
	dungeon_set_level(curdun, curlvl);

	dungeon_generate(curdun);
	dungeon_populate(curdun);

	dungeon_get_home(curdun, &tx, &ty);
	dungeon_set_player_xy(curdun, tx, ty);

	draw();
}

static void draw(void)
{
	int x, y;
	struct creature_node *iter;
	struct creature_list *cl;
	struct glyph glyph;
	struct tile tile;

	/* Draw all the tiles. */
	for (x = 0; x < dungeon_width(curdun); ++x) {
		for (y = 0; y < dungeon_height(curdun); ++y) {
			tile = *dungeon_tile_at(curdun, x, y);
			glyph = tile_glyph(tile);
			glyph_draw(x, y, glyph);
		}
	}

	/* Draw all the monsters. */
	cl = dungeon_creature_list(curdun);
	for (cl_begin(cl); !cl_end(cl); cl_next(cl)) {
		iter = cl_iter(cl);
		glyph_draw(iter->x, iter->y, iter->creature.glyph);
	}

	/* Draw the player. */
	dungeon_get_player_xy(curdun, &x, &y);
	glyph_draw(x, y, dungeon_get_player(curdun)->glyph);

	term_flush();
}

struct creature *act_walk(unsigned long action, int *x, int *y, int dx, int dy)
{
	/* The creature that would be run into, if there is one. */
	struct creature *end = NULL;
	int range = act_range(action);
	int capable_range = 1;
	int test_x = dx * capable_range;
	int test_y = dy * capable_range;

	while (1) {
		if (dungeon_creature_at(curdun, *x + test_x, *y + test_y)) {
			end = dungeon_creature_at(curdun,
				*x + test_x, *y + test_y);
			break;
		}

		if (!dungeon_walkable(curdun, *x + test_x, *y + test_y)) {
			break;
		}

		capable_range++;
		test_x = dx * capable_range;
		test_y = dy * capable_range;

		if (capable_range > range)
			break;
	}
	capable_range--;

	*x += dx * capable_range;
	*y += dy * capable_range;

	return end;
}

/******************************************************************************\
 * Shell Functions
\******************************************************************************/

static int gm_interact(struct shell *sh, int ac, char *av[])
{
	int px, py, sx, sy;
	dungeon_get_player_xy(curdun, &px, &py);
	dungeon_get_stairs(curdun, &sx, &sy);
	if (px == sx && py == sy) {
		descend();
	}
	shell_printf(sh, "Interacted! %d, %d -> %d, %d\n", px, py, sx, sy);
	return SHELL_SUCCESS;
}

static int gm_move(struct shell *sh, int ac, char *av[])
{
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

	shell_exec_linef(sh, "act %d %s", 0, av[1]);

	return SHELL_SUCCESS;
}

static int gm_act(struct shell *sh, int ac, char *av[])
{
	int dx;
	int dy;
	int dir;
	int act_no;
	char *tmp;
	struct creature *player = NULL;
	int px;
	int py;

	player = dungeon_get_player(curdun);
	dungeon_get_player_xy(curdun, &px, &py);

	if (ac <= 2) {
		shell_puts(sh, "usage: act <action_no> <direction>\n");
		return SHELL_SUCCESS;
	}

	act_no = strtol(av[1], &tmp, 10);

	if (tmp == av[1]) {
		shell_printf(sh, "error: %s is not an integer\n", av[1]);
		return SHELL_SUCCESS;
	}

	if (player->actions[act_no] == ACT_NONE) {
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

	act_walk(player->actions[act_no], &px, &py, dx, dy);
	dungeon_set_player_xy(curdun, px, py);

	shell_exec_line(sh, "update");

	return SHELL_SUCCESS;
}


static int gm_update(struct shell *sh, int ac, char *av[])
{
	int dx;
	int dy;
	int px;
	int py;
	struct creature_node *iter;
	struct creature_list *cl;

	dungeon_get_player_xy(curdun, &px, &py);

	cl = dungeon_creature_list(curdun);
	for (cl_begin(cl); !cl_end(cl); cl_next(cl)) {
		iter = cl_iter(cl);
		/* Pathfinding goes here. Currently very sucky. */
		dx = px - iter->x;
		dy = py - iter->y;
		CLAMP(dx, -1, 1);
		CLAMP(dy, -1, 1);
		act_walk(iter->creature.actions[0], &iter->x, &iter->y, dx, dy);
	}

	draw();
	return SHELL_SUCCESS;
}

static int gm_dungeon(struct shell *sh, int ac, char *av[])
{
	return SHELL_SUCCESS;
}
