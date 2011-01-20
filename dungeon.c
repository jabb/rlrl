
#include "dungeon.h"

#include "rand.h"
#include "term.h"

/******************************************************************************\
 * Tile
\******************************************************************************/

struct tile_defn {
	struct glyph glyph;
	unsigned blocks : 1;
	unsigned opaque : 1;
	unsigned reserved : 6;
};

struct tile_defn tile_tab[] = {
	{{'/', TERM_BLUE, TERM_BLACK}, 1, 1, 0},
	{{'.', TERM_WHITE, TERM_BLACK}, 0, 0, 0},
	{{'#', TERM_WHITE, TERM_BLACK}, 1, 1, 0}
};

struct tile tile_create(int type)
{
	struct tile t;
	t.type = type;
	return t;
}

struct glyph tile_glyph(struct tile t)
{
	return tile_tab[t.type].glyph;
}

int tile_is_blocked(struct tile t)
{
	return tile_tab[t.type].blocks;
}

int tile_is_opaque(struct tile t)
{
	return tile_tab[t.type].opaque;
}

/******************************************************************************\
 * Dir
\******************************************************************************/

#include <string.h>

void dir_delta(int dir, int *x, int *y)
{
	if (x) *x = 0;
	if (y) *y = 0;

	switch (dir) {
	case DIR_UP:
		if (y)
			*y = -1;
		break;

	case DIR_DOWN:
		if (y)
			*y = 1;
		break;

	case DIR_LEFT:
		if (x)
			*x = -1;
		break;

	case DIR_RIGHT:
		if (x)
			*x = 1;
		break;

	case DIR_UPLEFT:
		if (y)
			*y = -1;
		if (x)
			*x = -1;
		break;

	case DIR_UPRIGHT:
		if (y)
			*y = -1;
		if (x)
			*x = 1;
		break;

	case DIR_DOWNLEFT:
		if (y)
			*y = 1;
		if (x)
			*x = -1;
		break;

	case DIR_DOWNRIGHT:
		if (y)
			*y = 1;
		if (x)
			*x = 1;
		break;

	default:
		break;
	}
}

const char *dir_to_name(int dir)
{
	const char *names[DIR_COUNT] = {
		"up", "down", "left", "right",
		"up-left", "up-right", "down-left", "down-right"
	};
	return names[dir];
}

int dir_from_name(const char *name)
{
	if (strcmp(name, "up") == 0)
		return DIR_UP;
	else if (strcmp(name, "down") == 0)
		return DIR_DOWN;
	else if (strcmp(name, "left") == 0)
		return DIR_LEFT;
	else if (strcmp(name, "right") == 0)
		return DIR_RIGHT;
	else if (strcmp(name, "up-left") == 0)
		return DIR_UPLEFT;
	else if (strcmp(name, "up-right") == 0)
		return DIR_UPRIGHT;
	else if (strcmp(name, "down-left") == 0)
		return DIR_DOWNLEFT;
	else if (strcmp(name, "down-right") == 0)
		return DIR_DOWNRIGHT;
	else
		return DIR_NONE;
}

/******************************************************************************\
 * Dungeon
\******************************************************************************/

#include <stdlib.h>

struct dungeon {
	int width;
	int height;
	int level;
	int home_x;
	int home_y;
	int stair_x;
	int stair_y;
	struct creature_list *list;
	struct tile tiles[];
};

#define DUNTILEAT(dun, x, y) ((dun)->tiles[(y) * (dun)->width + (x)])

struct dungeon *dungeon_create(int width, int height)
{
	int x;
	int y;
	struct dungeon *dun = NULL;

	dun = malloc(sizeof(struct dungeon) +
		(width * height * sizeof(struct tile)));

	if (!dun)
		return NULL;

	dun->list = cl_create();
	dun->width = width;
	dun->height = height;
	dun->level = -1;
	dun->home_x = -1;
	dun->home_y = -1;
	dun->stair_x = -1;
	dun->stair_y = -1;

	for (x = 0; x < width; ++x)
		for (y = 0; y < height; ++y)
			DUNTILEAT(dun, x, y) = TILE_CREATE_NONE;

	return dun;
}

void dungeon_destroy(struct dungeon *dun)
{
	struct creature_node *tmp = NULL;

	for (cl_begin(dun->list); !cl_end(dun->list); cl_next(dun->list)) {
		tmp = cl_iter(dun->list);
		cl_del(dun->list, cl_iter(dun->list));
		cl_destroy_node(tmp);
	}
	cl_destroy(dun->list);
	free(dun);
}

void dungeon_set_level(struct dungeon *dun, int to)
{
	dun->level = to;
}

int dungeon_get_level(struct dungeon *dun)
{
	return dun->level;
}

void dungeon_generate(struct dungeon *dun)
{
	int x, y;

	/* Fill with walls. */
	for (x = 0; x < dun->width; ++x)
		for (y = 0; y < dun->height; ++y)
			DUNTILEAT(dun, x, y) = tile_create(TILE_DFLOOR);

	dun->home_x = 0;
	dun->home_y = 0;
	dun->stair_x = 1;
	dun->stair_y = 1;
}

void dungeon_populate(struct dungeon *dun)
{
	int i;
	int sparsity = 40;
	int cnum = dun->width * dun->height / sparsity;
	struct creature_node *tmp = NULL;

	for (cl_begin(dun->list); !cl_end(dun->list); cl_next(dun->list)) {
		tmp = cl_iter(dun->list);
		cl_del(dun->list, cl_iter(dun->list));
		cl_destroy_node(tmp);
	}

	for (i = 0; i < cnum; ++i) {
		tmp = cl_create_node();
		tmp->x = rand_range(1, dun->width - 1);
		tmp->y = rand_range(1, dun->height - 1);
		tmp->creature.glyph.sym = 'M';
		tmp->creature.glyph.fg = TERM_RED;
		tmp->creature.glyph.bg = TERM_BLACK;
		cl_add(dun->list, tmp);
	}
}

void dungeon_get_home(struct dungeon *dun, int *x, int *y)
{
	*x = dun->home_x;
	*y = dun->home_y;
}

void dungeon_get_stairs(struct dungeon *dun, int *x, int *y)
{
	*x = dun->stair_x;
	*y = dun->stair_y;
}

int dungeon_width(struct dungeon *dun)
{
	return dun->width;
}

int dungeon_height(struct dungeon *dun)
{
	return dun->height;
}

int dungeon_in_bounds(struct dungeon *dun, int x, int y)
{
	return x >= 0 && x < dun->width && y >= 0 && y < dun->height;
}

int dungeon_walkable(struct dungeon *dun, int x, int y)
{
	if (!dungeon_in_bounds(dun, x, y))
		return 0;

	if (dungeon_creature_at(dun, x, y))
		return 0;

	if (tile_is_blocked(DUNTILEAT(dun, x, y)))
		return 0;

	return 1;
}

struct creature *dungeon_creature_at(struct dungeon *dun, int x, int y)
{
	for (cl_begin(dun->list); !cl_end(dun->list); cl_next(dun->list)) {
		if (cl_iter(dun->list)->x == x && cl_iter(dun->list)->y == y)
			return &cl_iter(dun->list)->creature;
	}
	return NULL;
}

struct tile *dungeon_tile_at(struct dungeon *dun, int x, int y)
{
	return &DUNTILEAT(dun, x, y);
}

struct creature_list *dungeon_creature_list(struct dungeon *dun)
{
	return dun->list;
}
