
#ifndef TERM_H
#define TERM_H

#define TERM_BACKSPACE	263
#define TERM_ESCAPE	27
#define TERM_UP		259
#define TERM_DOWN	258
#define TERM_LEFT	260
#define TERM_RIGHT	261

enum {
	TERM_NONE = 0,
	TERM_BOLD = 1,
	TERM_DIM = 2,
	TERM_UNDERSCORE = 4,
	TERM_BLINK = 5,
	TERM_REVERSE = 7,
	TERM_HIDDEN = 8
};

enum {
	TERM_BLACK,
	TERM_RED,
	TERM_GREEN,
	TERM_YELLOW,
	TERM_BLUE,
	TERM_MAGENTA,
	TERM_CYAN,
	TERM_WHITE
};

#ifdef __cplusplus
extern "C" {
#endif

int term_open(void);
int term_close(void);

int term_width(void);
int term_height(void);

int term_clear(void);
int term_clear_line(void);

int term_cursor_show(int yes);
int term_cursor_move(int x, int y);
int term_cursor_up(int by);
int term_cursor_down(int by);
int term_cursor_right(int by);
int term_cursor_left(int by);


int term_get_foreground(void);
int term_get_background(void);

int term_set_attribute(int attr, int yes);
int term_set_foreground(int c);
int term_set_background(int c);
int term_set_char(int c);
int term_set_string(const char *str);

int term_flush(void);

int term_get_char(void);
int term_get_string(char *buf, int len);

#ifdef __cplusplus
}
#endif

#endif
