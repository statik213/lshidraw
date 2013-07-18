
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <linux/hidraw.h>
#include <linux/input.h>
#include <linux/types.h>

extern void hex_dump(void *data, int size, int addr, const char * prefixFirst, const char * prefix);

double getTimestamp() {
	struct timespec tp;
	uint32_t result;

	clock_gettime(CLOCK_MONOTONIC, &tp);

	return tp.tv_sec + (double) tp.tv_nsec * 1e-9;
}

int main(int argc, char* argv[]) {
	int rc, fd;
	char name[128];
	struct hidraw_devinfo info;
	struct hidraw_report_descriptor descriptor;
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

	rc = ioctl(fd, HIDIOCGRAWINFO, &info);
	if (rc < 0) {
		perror(path);
		close(fd);
		return -1;
	}

	rc = ioctl(fd, HIDIOCGRAWNAME(sizeof(name)), name);
	if (rc < 0) {
		perror(path);
		close(fd);
		return -1;
	}

	rc = ioctl(fd, HIDIOCGRDESCSIZE, &descriptor.size);
	if (rc < 0) {
		perror(path);
		close(fd);
		return -1;
	}

	rc = ioctl(fd, HIDIOCGRDESC, &descriptor);
	if (rc < 0) {
		perror(path);
		close(fd);
		return -1;
	}


	printf("%s:", path);
	printf(" - Name: `%s'\n", name);
	printf(" - ID %04x:%04x\n", (int)info.vendor & 0xffff, (int)info.product & 0xffff);
	printf(" - Descriptor Length: %d\n"
		   " - Descriptor:\n",
					(int) descriptor.size);
	hex_dump(&descriptor, descriptor.size, 1, "   >>> ", "   ... ");

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
			if (len <= 16) {
				hex_dump(buffer, len, 0, " | ", "");
			} else {
				hex_dump(buffer, len, 1, "\n    >>> ", "    ... ");
			}

			previous = now;
		}
	}
}
