
#ifndef GLYPH_H
#define GLYPH_H

struct glyph {
	char sym;
	int fg;
	int bg;
};

struct glyph glyph_create(int sym);
struct glyph glyph_create_fgbg(int sym, int fg, int bg);
void glyph_draw(int x, int y, struct glyph g);

#endif
