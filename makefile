CC = gcc
CFLAGS = -c -Wall -pedantic -std=c99
LDFLAGS = -lm
CFILES = main.c \
	term.c shell.c \
	gameman.c glyph.c dungeon.c creature.c \
	mt19937ar.c rand.c cmdline.c
OBJECTS = $(CFILES:.c=.o)
BINARY = rlrl

all: CFLAGS += -g
all: $(BINARY)

release: CFLAGS += -s -O2
release: $(BINARY)

curses: CFLAGS += -DTERM_CURSES
curses: LDFLAGS += -lcurses
curses: $(BINARY)

curses-release: CFLAGS += -DTERM_CURSES -s -O2
curses-release: LDFLAGS += -lcurses
curses-release: $(BINARY)

$(BINARY): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(OBJECTS) $(BINARY)
