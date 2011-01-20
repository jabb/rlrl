
#include "cmdline.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

struct word {
	struct word *next;
	char *str;
};

static const char *parse_brackets(struct word **words, const char *str)
{
	struct word *newword = NULL;
	int brackets = 1;
	int count = 0;

	str++; /* Skip the [ */
	if (!*str)
		return str;

	do {
		if (str[count] == '[')
			brackets++;
		else if (str[count] == ']')
			brackets--;
		count++;
	} while (brackets && str[count]);

	newword = malloc(sizeof(struct word));
	newword->next = NULL;
	newword->str = calloc(count, sizeof(char));

	/* -1 to eat the ] and to leave room for a null char */
	strncpy(newword->str, str, count - 1);
	newword->str[count - 1] = '\0';

	(*words)->next = newword;
	*words = (*words)->next;

	return str + count;
}

static const char *parse_quotes(struct word **words, const char *str)
{
	struct word *newword = NULL;
	int in_quotes = 1;
	int count = 0;

	str++; /* Skip the " */
	if (!*str)
		return str;

	do {
		if (str[count] == '"')
			in_quotes = 0;
		count++;
	} while (in_quotes && str[count]);

	newword = malloc(sizeof(struct word));
	newword->next = NULL;
	newword->str = calloc(count, sizeof(char));

	/* -1 to eat the " and to leave room for a null char */
	strncpy(newword->str, str, count - 1);
	newword->str[count - 1] = '\0';

	(*words)->next = newword;
	*words = (*words)->next;

	return str + count;
}

static const char *parse_word(struct word **words, const char *str)
{
	struct word *newword = NULL;
	int in_word = 1;
	int count = 0;

	do {
		if (isspace(str[count]))
			in_word = 0;
		if (!str[count])
			in_word = 0;
		count++;
	} while (in_word);

	newword = malloc(sizeof(struct word));
	newword->next = NULL;
	newword->str = calloc(count, sizeof(char));

	/* -1 to leave room for a null char */
	strncpy(newword->str, str, count - 1);
	newword->str[count - 1] = '\0';

	(*words)->next = newword;
	*words = (*words)->next;

	return str + count - 1;
}

static void parse_line(struct word *words, const char *str)
{
	struct word *tmp = words;
	while (*str) {
		while (isspace(*str) && *str)
			str++;

		if (*str == '"') {
			str = parse_quotes(&tmp, str);
		}
		else if (*str == '[') {
			str = parse_brackets(&tmp, str);
		}
		else {
			str = parse_word(&tmp, str);
		}
	}

}

struct cmdline_args *cmdline_parse(const char *str)
{
	struct cmdline_args *args = NULL;
	struct word begin = {NULL, ""};
	struct word *prev = NULL, *iter = NULL;
	int argc;

	parse_line(&begin, str);

	iter = begin.next;
	argc = 0;
	while (iter) {
		argc++;
		iter = iter->next;
	}

	args = malloc(sizeof(struct cmdline_args) + (argc * sizeof(char *)));
	args->argc = argc;

	iter = begin.next;
	argc = 0;
	while (iter) {
		args->argv[argc] = iter->str;
		argc++;
		prev = iter;
		iter = iter->next;
		free(prev);
	}

	return args;
}

void cmdline_free(struct cmdline_args *args)
{
	int i;
	for (i = 0; i < args->argc; ++i)
		free(args->argv[i]);
	free(args);
}

#ifdef CMDLINETEST
#include <stdio.h>

int main(void)
{
	int i;
	struct cmdline_args *args;
	args = cmdline_parse("map hi");

	printf("%d\n", args->argc);
	for (i = 0; i < args->argc; ++i)
		printf("'%s'\n", args->argv[i]);
	cmdline_free(args);
	return 0;
}

#endif
