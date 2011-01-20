
#include "creature.h"

/******************************************************************************\
 * Body
\******************************************************************************/

#include <math.h>

struct body_types {
	double brawn_mult;
	double deft_mult;
	double grit_mult;
	const char *name;
};

struct body_types body_types_table[BODY_TYPES] = {
/*	Brawn Deft Grit */
	{1.0, 1.0, 1.0, "average"},
	{1.1, 1.0, 1.0, "strong"},
	{1.1, 0.8, 1.1, "bulky"},
	{1.4, 0.5, 1.1, "brute"},
	{1.0, 1.1, 1.0, "agile"},
	{0.5, 1.3, 1.2, "withy"},
	{1.2, 1.2, 0.6, "steadfast"},
	{1.0, 1.0, 1.1, "tough"},
	{1.0, 0.8, 1.3, "juggernaut"},
	{1.1, 0.9, 1.1, "bruiser"},
	{0.9, 1.0, 0.9, "scrawny"},
	{0.8, 1.0, 1.0, "weak"},
	{0.8, 0.8, 0.8, "cripple"}
};

int body_size_modifier[BODY_SIZES] = {
	-3, -2, -1, 0, 1, 2, 3
};

const char *body_size_names[] = {
	"minute",
	"small",
	"tiny",
	"medium",
	"large",
	"huge",
	"immense"
};

struct body body_create(int score, int type, int size)
{
	struct body b;
	b.score = score;
	b.type = type;
	b.size = size;
	return b;
}

int body_score_bonus(struct body b)
{
	return body_size_modifier[b.size];
}

int body_true_score(struct body b)
{
	return b.score + body_score_bonus(b);
}

int body_brawn(struct body b)
{
	return ceil(body_true_score(b) * body_types_table[b.type].brawn_mult);
}

int body_deft(struct body b)
{
	return ceil(body_true_score(b) * body_types_table[b.type].deft_mult);
}

int body_grit(struct body b)
{
	return ceil(body_true_score(b) * body_types_table[b.type].grit_mult);
}

const char *body_type_name(int type)
{

	return body_types_table[type].name;
}

const char *body_size_name(int size)
{

	return body_size_names[size];
}

/******************************************************************************\
 * States
\******************************************************************************/

/******************************************************************************\
 * Actions
\******************************************************************************/

struct action_defn {
	/* What it does to the target. */
	unsigned does_damage : 1;
	unsigned applies_state : 1;
	/* What the action is, movement, charge, ranged. */
	unsigned is : 2;
	/* This is 0 for IS_NONE and IS_MOVE */
	unsigned range : 4;

	unsigned long applies_to;
	unsigned long can_from;

	/* Bonus damage. */
	unsigned long bonus;
	/* Which state it applies. */
	unsigned long effect;
};

struct action_defn action_tab[] = {
	{0, 0, ACT_IS_NONE, 0, STATE_NONE, STATE_ALL, 0, STATE_NONE},
	{0, 0, ACT_IS_MOVE, 1, STATE_NONE, STATE_ACTIVE, 0, STATE_NONE}
};

int act_does_damage(unsigned long act)
{
	return action_tab[act].does_damage;
}

int act_applies_state(unsigned long act)
{
	return action_tab[act].applies_state;
}

unsigned int act_is(unsigned long act)
{
	return action_tab[act].is;
}

unsigned int act_range(unsigned long act)
{
	return action_tab[act].range;
}

unsigned long act_applies_to(unsigned long act)
{
	return action_tab[act].applies_to;
}

unsigned long act_can_from(unsigned long act)
{
	return action_tab[act].can_from;
}

unsigned long act_bonus(unsigned long act)
{
	return action_tab[act].bonus;
}

unsigned long act_effect(unsigned long act)
{
	return action_tab[act].effect;
}

/******************************************************************************\
 * Features
\******************************************************************************/

/******************************************************************************\
 * Creature
\******************************************************************************/

void creature_generate(struct creature *creature)
{
	/* Action 0 is ALWAYS move. */
	creature->actions[0] = ACT_MOVE;
}

/******************************************************************************\
 * Combat
\******************************************************************************/

struct combat_report
combat_perform(struct creature *att, unsigned long act, struct creature *def)
{
	struct combat_report cr;

	return cr;
}

/******************************************************************************\
 * Creature List
\******************************************************************************/

#include <stdlib.h>

struct creature_node *cl_create_node(void)
{
	struct creature_node *no = NULL;

	no = malloc(sizeof(struct creature_node));

	if (!no)
		return NULL;

	no->next = NULL;

	return no;
}

void cl_destroy_node(struct creature_node *node)
{
	free(node);
}

struct creature_list *cl_create(void)
{
	struct creature_list *cl = NULL;

	cl = malloc(sizeof(struct creature_list));

	if (!cl)
		return NULL;

	cl->head = NULL;
	cl->iter = NULL;
	cl->iter_next = NULL;

	return cl;
}

void cl_destroy(struct creature_list *cl)
{
	free(cl);
}

void cl_begin(struct creature_list *cl)
{
	cl->iter = cl->head;
	if (cl->head)
		cl->iter_next = cl->head->next;
}

int cl_end(struct creature_list *cl)
{
	return cl->iter == NULL;
}

void cl_next(struct creature_list *cl)
{
	cl->iter = cl->iter_next;
	if (cl->iter_next)
		cl->iter_next = cl->iter_next->next;
}

struct creature_node *cl_iter(struct creature_list *cl)
{
	return cl->iter;
}

void cl_del(struct creature_list *cl, struct creature_node *node)
{
	struct creature_node *prev = NULL, *iter = NULL;

	if (node == cl->head) {
		cl->head = NULL;
		return;
	}

	for (iter = cl->head; iter; prev = iter, iter = iter->next) {
		if (node == iter) {
			if (cl->iter_next == node)
				cl->iter_next = cl->iter_next->next;

			prev->next = node->next;
			node->next = NULL;
		}
	}
}

void cl_add(struct creature_list *cl, struct creature_node *node)
{
	node->next = cl->head;
	cl->head = node;
}
