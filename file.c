/*
 * Copyright © 2014 Ted Unangst <tedu@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <sys/stat.h>

#include <stdint.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <unistd.h>
#include <errno.h>

static void
usage(void)
{
	fprintf(stderr, "usage:\tfile [-hi] ...\n");
	exit(1);
}

static const char *
file(const char *filename, int symlinks, int ignoremagic)
{
	struct stat sb;

	if ((symlinks ? lstat : stat)(filename, &sb) == -1) {
		if (!symlinks)
			return file(filename, 1, ignoremagic);
		return "cannot open";
	} else if (S_ISBLK(sb.st_mode)) {
		return "block special";
	} else if (S_ISCHR(sb.st_mode)) {
		return "character special";
	} else if (S_ISDIR(sb.st_mode)) {
		return "directory";
	} else if (S_ISFIFO(sb.st_mode)) {
		return "FIFO";
	} else if (S_ISLNK(sb.st_mode)) {
		static char buf[1048];
		char linkbuf[1024];
		int len;

		len = readlink(filename, linkbuf, sizeof(linkbuf) - 1);
		if (len == -1)
			return "cannot open";
		linkbuf[len] = 0;
		snprintf(buf, sizeof(buf), "symbolic link to %s", linkbuf);
		return buf;
	} else if (S_ISREG(sb.st_mode)) {
		if (ignoremagic)
			return "regular file";
		else
			return "data file";
	} else if (S_ISSOCK(sb.st_mode)) {
		return "socket";
	}
	return "cannot open";
}

int
main(int argc, char **argv)
{
	int ch, i;
	int symlinks = 0;
	int ignoremagic = 0;

	while ((ch = getopt(argc, argv, "dhim:M:")) != -1) {
		switch(ch) {
		case 'h':
			symlinks = 1;
			break;
		case 'i':
			ignoremagic = 1;
			break;
		case 'd':
		case 'm':
		case 'M':
			/* ignore */
			break;
		default:
			usage();
			break;
		}
	}
	argc -= optind;
	argv += optind;

	if (!argc)
		usage();
	
	for (i = 0; i < argc; i++) {
		const char *type = file(argv[i], symlinks, ignoremagic);
		printf("%s: %s\n", argv[i], type);
	}


	return 0;
}
