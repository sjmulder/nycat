CFLAGS += -Wall -Wextra -pedantic -std=iso9899:1999

all: nycat

clean:
	rm -f nycat

lint:
	lint -usX 265,233 nycat.c
	cppcheck nycat.c
	clang-check nycat.c --
	clang-check -analyze nycat.c --

.PHONY: all clean scan
