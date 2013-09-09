#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

int main(int argc, char* argv[]) {
	int rc, fd;
	const char * path;

	if (argc < 2) {
		return -1;
	}

	path = argv[1];

	fd = open(path, O_RDONLY);
	if (fd < 0) {
		perror(path);
		close(fd);
		return -1;
	}

	{
		unsigned int num = 0;
		double start = getTimestamp();
		double previous = start;

		while (1) {
			char buffer[256];
			int len = read(fd, buffer, sizeof(buffer));
			double now = getTimestamp();
			int i;

			if (len < 0) {
				perror(path);
				close(fd);
				return -1;
			}

			num++;

			printf("#%-5d t: %5.3f dt: %5.4f len: %2d", num, now - start, now - previous, len);

			if (len == 0) {
				printf(" EOF\n");
				return 0;
			}

			if (len <= 16) {
				hex_dump(buffer, len, 0, " | ", "");
			} else {
				hex_dump(buffer, len, 1, "\n    >>> ", "    ... ");
			}

			previous = now;
		}
	}
}
