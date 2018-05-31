/* nycat.c

   Like cat(1), but with rainbow colours. Uses ANSI terminal escapes, which
   should work on most Unixy terminals and on Windows 10 (Anniversary Update)
   or newer.

   This code follows the JPL coding standards because rainbows are obviously
   safety critical.

   To build and use:

       make
       ./nycat file file ...

   Run static analysis (may have to tweak the Makefile):

       make lint

   By Sijmen J. Mulder <ik@sjmulder.nl>
   ___
   Copyright (c) 2018, Sijmen J. Mulder
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE. */

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#define LEN(a) (sizeof(a)/sizeof(*(a)))

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

static const char colors[][6] = {
	"\x1B[31m",
	"\x1B[33m",
	"\x1B[93m",
	"\x1B[92m",
	"\x1B[96m",
	"\x1B[94m",
	"\x1B[95m"
};

static int
setup(void)
{
#ifdef _WIN32
	HANDLE handle;
	DWORD mode;
	BOOL bret;

	handle = GetStdHandle(STD_OUTPUT_HANDLE);
	if (handle == NULL) {
		(void)fputs("No attached console\n", stdout);
		return -1;
	} else if (handle == INVALID_HANDLE_VALUE) {
		(void)fprintf(stderr, "GetStdHandle(): error %d\n",
		    GetLastError());
		return -1;
	}

	bret = GetConsoleMode(handle, &mode);
	if (!bret) {
		(void)fprintf(stderr, "GetConsoleMode(): error %d\n",
		    GetLastError());
		return -1;
	}

	mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	bret = SetConsoleMode(handle, mode);
	if (!bret) {
		(void)fprintf(stderr, "SetConsoleMode(): error %d\n",
		    GetLastError());
		return -1;
	}
#endif

	return 0;
}

static void
nywrite(const char *p, size_t n, FILE *f)
{
	static int color = 0;

	size_t i;
	const char *cur = p;
	const char *lf;

	assert(p != NULL);
	assert(f != NULL);

	(void)fputs(colors[color], stdout);

	lf = memchr(cur, '\n', (size_t)(p+n-cur));
	for (i = 0; lf && i < n; i++) {
		(void)fwrite(cur, 1, (size_t)(lf-cur+1), f);
		color = (color+1) % (int)LEN(colors);
		(void)fputs(colors[color], f);
		cur = lf+1;
		lf = memchr(cur, '\n', (size_t)(p+n-cur));
	}

	(void)fwrite(cur, 1, (size_t)(p+n-cur), f);
	(void)fputs("\x1B[0m", f);

	assert(lf == NULL);
}

int
main(int argc, char **argv)
{
	int iret;
	int i;
	int j;
	int err;
	FILE *f;
	char buf[4096];
	size_t nread;

	iret = setup();
	if (iret == -1) {
		return 1;
	}

	if (argc <= 1) {
		(void)fputs("usage: nycat file file ...\n", stderr);
		return 1;
	}

	assert(argv);

	for (i = 1; i < argc; i++) {
		f = fopen(argv[i], "r");
		if (!f) {
			err = errno;
			(void)fflush(stdout);
			(void)fprintf(stderr, "%s: %s\n", strerror(err),
			    argv[i]);
			return 1;
		}

		nread = fread(buf, 1, sizeof(buf), f);
		for (j = 0; nread && j < 1024 * 1024 * 1024; j++) {
			nywrite(buf, nread, stdout);
			nread = fread(buf, 1, sizeof(buf), f);
		}

		assert(nread == 0);

		if (ferror(f)) {
			err = errno;
			(void)fclose(f);
			(void)fflush(stdout);
			(void)fprintf(stderr, "%s: %s\n", strerror(err),
			    argv[i]);
			return 1;
		}

		(void)fclose(f);
	}

	return 0;
}
