CC = gcc
CFLAGS = -c -Wall -pedantic -std=c99 -g
LDFLAGS = -lm
CFILES = term.c shell.c gameman.c glyph.c dungeon.c creature.c main.c
OBJECTS = $(CFILES:.c=.o)
BINARY = rlrl

all: $(CFILES) $(BINARY)

$(BINARY): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(OBJECTS) $(BINARY)
