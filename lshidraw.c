#define DEV_DIR "/dev/"
#define HIDRAW_PREFIX  "hidraw"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
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

/**
 * dumps size bytes of *data to stdout.
 * Looks like:
 * [prefix][0000] 75 6E 6B 6E 6F 77 6E 20
 *                  30 FF 00 00 00 00 39 00 unknown 0.....9.
 * (in a single line of course)
 * @param data data to dumps
 * @param prefix string to prefix each line
 * @param size size of data
 */
static void hex_dump(void *data, const char * prefix, int size)
{

    unsigned char *p = (unsigned char *)data;
    unsigned char c;
    int n;
    char bytestr[4] = {0};
    char addrstr[10] = {0};
    char hexstr[ 16*3 + 5] = {0};
    char charstr[16*1 + 5] = {0};
    for(n=1;n<=size;n++) {
        if (n%16 == 1) {
            /* store address for this line */
            unsigned int offset = ((uintptr_t)p-(uintptr_t)data);
            snprintf(addrstr, sizeof(addrstr), "%.4x", offset);
        }

        c = *p;
        if (!isprint(c)) {
            c = '.';
        }

        /* store hex str (for left side) */
        snprintf(bytestr, sizeof(bytestr), "%02X ", *p);
        strncat(hexstr, bytestr, sizeof(hexstr)-strlen(hexstr)-1);

        /* store char str (for right side) */
        snprintf(bytestr, sizeof(bytestr), "%c", c);
        strncat(charstr, bytestr, sizeof(charstr)-strlen(charstr)-1);

        if(n%16 == 0) {
            /* line completed */
            printf("%s[%4.4s]   %-50.50s  %s\n", prefix, addrstr, hexstr, charstr);
            hexstr[0] = 0;
            charstr[0] = 0;
        } else if(n%8 == 0) {
            /* half line: add whitespaces */
            strncat(hexstr, "  ", sizeof(hexstr)-strlen(hexstr)-1);
            strncat(charstr, " ", sizeof(charstr)-strlen(charstr)-1);
        }
        p++; /* next byte */
    }

    if (strlen(hexstr) > 0) {
        /* print rest of buffer if not empty */
        printf("%s[%4.4s]   %-50.50s  %s\n", prefix, addrstr, hexstr, charstr);
    }
}

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

	printf("%s - ID %04x:%04x %s\n", path, (int)info.vendor, (int)info.product, name);

	if (args->dumpDescriptors) {
		printf("    - Descriptor Length: %d\n"
			   "    - Descriptor:\n",
						(int) descriptor.size);
		hex_dump(&descriptor, "      ", descriptor.size);
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
