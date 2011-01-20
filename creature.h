
#ifndef CREATURE_H
#define CREATURE_H

#define BODY_SIZE_MINUTE	0
#define BODY_SIZE_SMALL		1
#define BODY_SIZE_TINY		2
#define BODY_SIZE_MEDIUM	3
#define BODY_SIZE_LARGE		4
#define BODY_SIZE_HUGE		5
#define BODY_SIZE_IMMENSE	6
#define BODY_SIZES		7

#define BODY_TYPE_AVERAGE	0
#define BODY_TYPE_STRONG	1
#define BODY_TYPE_BULKY		2
#define BODY_TYPE_BRUTE		3
#define BODY_TYPE_AGILE		4
#define BODY_TYPE_WITHY		5
#define BODY_TYPE_STEADFAST	6
#define BODY_TYPE_TOUGH		7
#define BODY_TYPE_JUGGERNAUT	8
#define BODY_TYPE_BRUISER	9
#define BODY_TYPE_SCRAWY	10
#define BODY_TYPE_WEAK		11
#define BODY_TYPE_CRIPPLED	12
#define BODY_TYPES		13

struct body {
	int score;
	int type;
	int size;
};

struct body body_create(int score, int type, int size);

int body_score_bonus(struct body b);
int body_true_score(struct body b);
int body_brawn(struct body b);
int body_deft(struct body b);
int body_grit(struct body b);

const char *body_type_name(int type);
const char *body_size_name(int size);



#define STATE_ALL		0xFFFFFFFF
#define STATE_NONE		0x00000000

#define STATE_ACTIVE		0x000000FF
#define STATE_NORMAL		0x00000001

#define STATE_INACTIVE		0x0000FF00
#define STATE_GRAPPLED		0x00000100

#define STATE_HYPERACTIVE	0x00FF0000
#define STATE_ENRAGED		0x00010000



#define ACT_NONE		0
#define ACT_MOVE		1

#define ACT_IS_NONE		0
#define ACT_IS_MOVE		1
#define ACT_IS_CHARGE		2
#define ACT_IS_RANGED		3

int act_does_damage(unsigned long act);
int act_applies_state(unsigned long act);
unsigned int act_is(unsigned long act);
unsigned int act_range(unsigned long act);

unsigned long act_applies_to(unsigned long act);
unsigned long act_can_from(unsigned long act);

unsigned long act_bonus(unsigned long act);
unsigned long act_effect(unsigned long act);



struct feature {
	int type;
};



#include "glyph.h"

#define CREATURE_ACTIONS	10
#define CREATURE_FEATURES	10

struct creature {
	unsigned int fatigue;
	struct glyph glyph;
	struct body body;
	unsigned long state;
	unsigned long actions[CREATURE_ACTIONS];
	struct feature features[CREATURE_FEATURES];
};

void creature_generate(struct creature *creature);



struct combat_report {
	int damage;
};

struct combat_report
combat_perform(struct creature *att, unsigned long act, struct creature *def);



struct creature_node {
	int x, y;
	struct creature creature;
	struct creature_node *next;
};

struct creature_list {
	struct creature_node *head;
	struct creature_node *iter;
	struct creature_node *iter_next;
};

struct creature_node *cl_create_node(void);
void cl_destroy_node(struct creature_node *node);

struct creature_list *cl_create(void);
void cl_destroy(struct creature_list *cl);

void cl_begin(struct creature_list *cl);
int cl_end(struct creature_list *cl);
void cl_next(struct creature_list *cl);
struct creature_node *cl_iter(struct creature_list *cl);

void cl_del(struct creature_list *cl, struct creature_node *node);
void cl_add(struct creature_list *cl, struct creature_node *node);

#endif
