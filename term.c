
#include "term.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

#define MAXATTRS 48

static int fg = TERM_WHITE;
static int bg = TERM_BLACK;

static int get_char(void)
{
	char ch = 0;
	struct termios vt;

	term_flush();

	tcgetattr(0, &vt);
	/* Set local modes. */
	vt.c_lflag &= ~ICANON;	/* Special characters. */
	vt.c_lflag &= ~ECHO;	/* Don't echo. */
	/* Set control characters. */
	vt.c_cc[VMIN] = 1;	/* Minimum number of characters for a read. */
	vt.c_cc[VTIME] = 0;	/* Timeout in deciseconds 0 is infinite. */
	tcsetattr(0, TCSANOW, &vt);

	read(0, &ch, 1); /* 0 is stdin */

	/* Reset local modes. */
	vt.c_lflag |= ICANON;
	vt.c_lflag |= ECHO;
	tcsetattr(0, TCSADRAIN, &vt);

	return ch;
}

static int char_avail(void)
{
	int rd = 0;
	char ch = 0;
	struct termios vt;

	term_flush();

	tcgetattr(0, &vt);
	/* Set local modes. */
	vt.c_lflag &= ~ICANON;	/* Special characters. */
	vt.c_lflag &= ~ECHO;	/* Don't echo. */
	/* Set control characters. */
	vt.c_cc[VMIN] = 0;	/* Minimum number of characters for a read. */
	vt.c_cc[VTIME] = 1;	/* Timeout in deciseconds 0 is infinite. */
	tcsetattr(0, TCSANOW, &vt);

	rd = read(0, &ch, 1); /* 0 is stdin */

	if (rd == 1)
		ungetc(ch, stdin);

	/* Reset local modes. */
	vt.c_lflag |= ICANON;
	vt.c_lflag |= ECHO;
	tcsetattr(0, TCSADRAIN, &vt);

	return rd;
}

int term_open(void)
{
	printf("\x1b=");
	term_clear();
	return 0;
}

int term_close(void)
{
	term_set_foreground(TERM_WHITE);
	term_set_background(TERM_BLACK);
	printf("%cc", TERM_ESCAPE);
	return 0;
}

int term_width(void)
{
	struct {
		unsigned short ts_lines;
		unsigned short ts_cols;
		unsigned short ts_xxx;
		unsigned short ts_yyy;
	} ts;
	ioctl(0, TIOCGWINSZ, &ts);
	return ts.ts_cols;
}

int term_height(void)
{
	struct {
		unsigned short ts_lines;
		unsigned short ts_cols;
		unsigned short ts_xxx;
		unsigned short ts_yyy;
	} ts;
	ioctl(0, TIOCGWINSZ, &ts);
	return ts.ts_lines;
}

int term_clear(void)
{
	printf("\x1b[2J");
	return 0;
}

int term_clear_line(void)
{
	printf("\x1b[2K");
	return 0;
}

int term_cursor_move(int x, int y)
{
	printf("\x1b[%d;%dH", y + 1, x + 1);
	return 0;
}

int term_cursor_show(int yes)
{
	if (yes)
		printf("\x1b[?25h");
	else
		printf("\x1b[?25l");
	return 0;
}

int term_cursor_up(int by)
{
	printf("\x1b[%dA", by);
	return 0;
}

int term_cursor_down(int by)
{
	printf("\x1b[%dB", by);
	return 0;
}

int term_cursor_right(int by)
{
	printf("\x1b[%dC", by);
	return 0;
}

int term_cursor_left(int by)
{
	printf("\x1b[%dD", by);
	return 0;
}

int term_get_foreground(void)
{
	return fg;
}

int term_get_background(void)
{
	return bg;
}

int term_set_attribute(int attr, int yes)
{
	static int attrs[MAXATTRS] = {0};
	int i;

	attrs[attr] = yes;

	printf("\x1b[");
        for (i = 0; i < MAXATTRS; ++i) {
		if (attrs[i] && i == 0)
			printf("%d", i);
		else if (attrs[i])
			printf(";%d", i);
	}
	printf("m");
	return 0;
}

int term_set_foreground(int c)
{
	term_set_attribute(fg + 30, 0);
	fg = c;
	term_set_attribute(fg + 30, 1);
	return 0;
}

int term_set_background(int c)
{
	term_set_attribute(bg + 40, 0);
	/* This is to make it so black is the default background for a
	 * terminal. */
	if (c != TERM_BLACK) {
		bg = c;
		term_set_attribute(bg + 40, 1);
	}
	return 0;
}

int term_set_char(int c)
{
	printf("%c", c);
	return 0;
}

int term_set_string(const char *str)
{
	ssize_t len = strlen(str);
	int i;

	for (i = 0; i < len; ++i)
		term_set_char(str[i]);
	return 0;
}

int term_flush(void)
{
	fflush(stdout);
	return 0;
}

int term_get_char(void)
{
	int ch;

	while (1) {
		ch = get_char();

		if (ch == TERM_ESCAPE) {
			if (!char_avail())
				break;

			do {
				ch = get_char();
			} while (!isalpha(ch));
		}
		else {
			break;
		}
	}

	return ch;
}


int term_get_string(char *buf, int len)
{
	char *fill = buf;

	char ch;

	while ((ch = term_get_char()) != '\n') {

		if ((ch == '\b' || ch == 127)) {
			if (fill > buf) {
				term_cursor_left(1);
				term_set_char(' ');
				term_cursor_left(1);
				fill--;
			}
		}
		else if (ch == 27) {
			fill = buf;
			break;
		}
		else if (ch == '\t') {
			if (fill - buf < len -1) {
				term_set_char(' ');
				*fill = ch;
				fill++;
			}
		}
		else {
			if (fill - buf < len -1) {
				term_set_char(ch);
				*fill = ch;
				fill++;
			}
		}
	}

	*fill = '\0';

	return 0;
}
