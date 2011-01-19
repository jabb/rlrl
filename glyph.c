
#include "glyph.h"

#include "term.h"


struct glyph glyph_create(int sym)
{
	return glyph_create_fgbg(sym, TERM_WHITE, TERM_BLACK);
}

struct glyph glyph_create_fgbg(int sym, int fg, int bg)
{
	struct glyph g;
	g.sym = sym;
	g.fg = fg;
	g.bg = bg;
	return g;
}

void glyph_draw(int x, int y, struct glyph g)
{
	term_cursor_move(x, y);
	term_set_foreground(g.fg);
	term_set_background(g.bg);
	term_set_char(g.sym);
}
