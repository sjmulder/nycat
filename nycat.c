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

static void
nywrite(const char *p, size_t n, FILE *f)
{
	static int color = 0;

	const char *cur = p;
	const char *lf;

	fputs(colors[color], stdout);

	while ((lf = memchr(cur, '\n', p+n-cur)) != NULL) {
		fwrite(cur, 1, lf-cur+1, f);
		color = (color+1) % (int)LEN(colors);
		fputs(colors[color], f);
		cur = lf+1;
	}

	fwrite(cur, 1, p+n-cur, f);
	fputs("\x1B[0m", f);
}

int
main(int argc, char **argv)
{
	char **path;
	FILE *f;
	char buf[4096];
	size_t n;

	if (argc <= 1) {
		fputs("usage: nycat file file ...\n", stderr);
		return 1;
	}
	
	for (path = &argv[1]; *path; path++) {
		if (!(f = fopen(*path, "r")))
			goto error;
		while ((n = fread(buf, 1, sizeof(buf), f)) > 0)
			nywrite(buf, n, stdout);
		if (ferror(f))
			goto error;
		fclose(f);
	}

	return 0;

error:
	fflush(stdout);
	fprintf(stderr, "%s: %s\n", strerror(errno), *path);
	return 1;
}
