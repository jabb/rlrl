
#ifndef DUNGEON_H
#define DUNGEON_H

#include "creature.h"
#include "glyph.h"

#define TILE_NONE	0
#define TILE_DFLOOR	1
#define TILE_DWALL	2

struct tile {
	int type;
};

#define TILE_CREATE_NONE tile_create(TILE_NONE)
struct tile tile_create(int type);

struct glyph tile_glyph(struct tile t);
int tile_is_blocked(struct tile t);
int tile_is_opaque(struct tile t);

/* Eight directions. */
#define DIR_NONE	(-1)
#define DIR_UP		0
#define DIR_DOWN	1
#define DIR_LEFT	2
#define DIR_RIGHT	3
#define DIR_UPLEFT	4
#define DIR_UPRIGHT	5
#define DIR_DOWNLEFT	6
#define DIR_DOWNRIGHT	7
#define DIR_COUNT	8

void dir_delta(int dir, int *x, int *y);
const char *dir_to_name(int dir);
int dir_from_name(const char *name);

struct dungeon;

struct dungeon *dungeon_create(int width, int height);
void dungeon_destroy(struct dungeon *dun); /* goofed */

void dungeon_set_level(struct dungeon *dun, int to);
int dungeon_get_level(struct dungeon *dun);
void dungeon_generate(struct dungeon *dun);
void dungeon_populate(struct dungeon *dun);

void dungeon_get_home(struct dungeon *dun, int *x, int *y);
void dungeon_get_stairs(struct dungeon *dun, int *x, int *y);

int dungeon_width(struct dungeon *dun);
int dungeon_height(struct dungeon *dun);

int dungeon_in_bounds(struct dungeon *dun, int x, int y);
int dungeon_walkable(struct dungeon *dun, int x, int y);

struct creature *dungeon_creature_at(struct dungeon *dun, int x, int y);
struct tile *dungeon_tile_at(struct dungeon *dun, int x, int y);
struct creature_list *dungeon_creature_list(struct dungeon *dun);

#endif
