#define _WITH_GETLINE
#define LEN(a) (sizeof(a)/sizeof(*(a)))

#include <stdio.h>
#include <string.h>
#include <errno.h>

static const char colors[][6] = {
	"\x1B[31m",
	"\x1B[33m",
	"\x1B[93m",
	"\x1B[92m",
	"\x1B[96m",
	"\x1B[94m",
	"\x1B[95m"
};

int
main(int argc, char **argv)
{
	char **path;
	int colori = 0;
	FILE *f;
	char *line = NULL;
	size_t cap = 0;

	if (argc <= 1) {
		fputs("usage: nycat file file ...\n", stderr);
		return 1;
	}
	
	for (path = &argv[1]; *path; path++) {
		if (!(f = fopen(*path, "r")))
			goto error;

		while (getline(&line, &cap, f) != -1) {
			fputs(colors[colori], stdout);
			colori = (colori + 1) % LEN(colors);
			fputs(line, stdout);
		}

		if (ferror(f))
			goto error;

		fclose(f);
	}

	fputs("\x1B[0m", stdout);
	return 0;

error:
	fputs("\x1B[0m", stdout);
	fflush(stdout);
	fprintf(stderr, "%s: %s\n", strerror(errno), *path);
	return 1;
}
