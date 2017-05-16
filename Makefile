CC := clang
CFLAGS := -g -O0 -Wall -std=c99 -Isrc
LDFLAGS := -lz
EXECUTABLE := dmfdecode

SOURCES := $(wildcard src/*.c)
OBJECTS := $(SOURCES:.c=.o)

.PHONY: all clean

all: $(EXECUTABLE)

clean:
	$(RM) $(OBJECTS) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LDFLAGS) -o $@

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@
