#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

struct Args {
	int verbose;
	const char * path;
	int rewind;
};

static int runPoll(struct Args const * args) {
	unsigned int num = 0;
	double start = getTimestamp();
	double previous = start;
	struct pollfd p;

	int fd = open(args->path, O_RDONLY | O_NONBLOCK);
	if (fd < 0) {
		perror(args->path);
		return -1;
	}

	p.fd = fd;
	p.events = POLLPRI | POLLIN | POLLERR | POLLHUP;

	while (1) {
		int rc;
		char buffer[256];

		p.revents = 0;
		rc = poll(&p, 1, 1000);
		if (rc < 0) {
			perror("error in poll");
			close(fd);
			return  -1;
		}

		if (rc == 0) {
			printf(".");
			fflush(stdout);
			continue;
		}
		printf("\r");

		double now = getTimestamp();
		int i;

		int len = read(fd, buffer, sizeof(buffer));
		if (len < 0) {
			perror(args->path);
			close(fd);
			return -1;
		}

		int quit = 0;
		if (p.revents & POLLERR) {
			quit = 0;
			printf(" - POLLERR\n");
		}

		if (p.revents & POLLHUP) {
			quit = 1;
			printf(" - HUP\n");
		}

		if (quit) {
			close(fd);
			return 0;
		}

		const char p = (p & POLLPRI) ? '*' : ' ';

		num++;

		printf("#%-5d t: %5.3f dt: %5.4f len: %2d %c", num, now - start, now - previous, len, p);

		if (len == 0) {
			if (args->rewind) {
				rc = lseek(fd, 0, SEEK_SET);
				if (rc < 0) {
					perror("cannot seek back");
					return -1;
				}
			}
			printf(" - EOF\n");
		} else if (len <= 16) {
			hex_dump(buffer, len, 0, " | ", "");
		} else {
			hex_dump(buffer, len, 1, "\n    >>> ", "    ... ");
		}

		previous = now;
	}
}

int main(int argc, char* argv[]) {
	int rc, fd;
	char c;

	int showUsage = 0;
	struct Args args = {
		.verbose = 0,
		.path = NULL,
		.rewind = 0
	};
	while ((c = getopt(argc, argv, "dvh")) != -1) {
		switch (c) {
			case 'h':
				showUsage = 1;
				break;

			case 'r':
				args.rewind = 1;
				break;

			case 'v':
				args.verbose++;
				break;

			case '?':
			default:
				showUsage = -1;
				break;
		}
	}

	if (optind < argc) {
		int i;
		// list only specified files
		for (i = optind; i < argc; i++) {
			if (args.path) {
				showUsage = 1;
				break;
			}
			args.path = argv[i];
		}
	}

	if (showUsage || !args.path) {
		printf("Usage: %s -r file\n", argv[0]);
		return -1;
	}

	printf("path: %s", args.path);
	runPoll(&args);
}
