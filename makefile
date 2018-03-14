
CC = gcc
CFLAGS = -Wextra -Wall -std=c99 -Iinclude
VPATH = include:src

OBJDIR = obj
SRCDIR = src
SOURCES := $(wildcard $(SRCDIR)/*.c)
OBJECTS := $(addprefix $(OBJDIR)/,$(SOURCES:%.c=%.o))

all: dirs tests

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -c $(CFLAGS) $< -o $@

dirs:
	mkdir -p obj
	mkdir -p bin

tests: tests.c
	$(CC) $(CFLAGS) $^ -lcmocka -o ./bin/$@

clean:
	rm -rf obj bin