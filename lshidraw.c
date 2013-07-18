#define DEV_DIR "/dev/"
#define HIDRAW_PREFIX  "hidraw"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <linux/hidraw.h>
#include <linux/input.h>
#include <linux/types.h>

struct Args {
	int verbose;
	int dumpDescriptors;
};

extern void hex_dump(void *data, int size, int printAddr, const char * prefixFirst, const char * prefix);

int ls(const char * path, struct Args * args) {
	int rc, fd;
	char name[128];
	struct hidraw_devinfo info;
	struct hidraw_report_descriptor descriptor;

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

	printf("%s - ID %04x:%04x %s\n", path, (int)info.vendor & 0xffff, (int)info.product & 0xffff, name);

	if (args->dumpDescriptors) {
		printf("    - Descriptor Length: %d\n"
			   "    - Descriptor:\n",
						(int) descriptor.size);
		hex_dump(&descriptor, descriptor.size, 1, "      ", "      ");
	}

	close(fd);
	return 0;
}

void printUsage(const char * name, int rc) {
	printf("Usage: %s -dvh file1 file ... filen\n", name);
	exit(rc);
}

int main(int argc, char* argv[]) {
	DIR*    dev_dir;
	struct Args args;
	int i, c;

	opterr = 0;
	args.verbose = 1;
	args.dumpDescriptors = 0;

	while ((c = getopt(argc, argv, "dvh")) != -1) {
		switch (c) {
			case 'h':
				printUsage(argv[1], 0);
				break;

			case 'd':
				args.dumpDescriptors = 1;
				break;

			case 'v':
				args.verbose++;
				break;

			case '?':
			default:
				printUsage(argv[1], 0);
				break;
		}
	}

	if (optind < argc) {
		// list only specified files
		for (i = optind; i < argc; i++) {
			ls(argv[i], &args);
		}
		return 0;
	}


	dev_dir = opendir(DEV_DIR);
	if (!dev_dir) {
		perror(DEV_DIR);
	}

	struct dirent*  ent;
	while ( (ent = readdir(dev_dir)) != NULL ) {
		char abspath[PATH_MAX] = "";
		if (strncmp(ent->d_name, HIDRAW_PREFIX, strlen(HIDRAW_PREFIX)) != 0) {
			continue;
		}

		strcat(abspath, DEV_DIR);
		strcat(abspath, ent->d_name);
		ls(abspath, &args);
	}
	return 0;
}
