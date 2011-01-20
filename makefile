CC = gcc
CFLAGS = -c -Wall -pedantic -std=c99 -g
LDFLAGS = -lm
CFILES = main.c \
	term.c shell.c gameman.c glyph.c \
	dungeon.c creature.c \
	mt19937ar.c rand.c cmdline.c
OBJECTS = $(CFILES:.c=.o)
BINARY = rlrl

all: $(BINARY)

$(BINARY): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(OBJECTS) $(BINARY)
