#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

extern double getTimestamp() {
	struct timespec tp;
	uint32_t result;

	clock_gettime(CLOCK_MONOTONIC, &tp);

	return tp.tv_sec + (double) tp.tv_nsec * 1e-9;
}

/**
 * dumps size bytes of *data to stdout.
 * Looks like:
 * [prefix][0000] 75 6E 6B 6E 6F 77 6E 20
 *                  30 FF 00 00 00 00 39 00 unknown 0.....9.
 * (in a single line of course)
 * @param data data to dumps
 * @param size size of data
 * @param printAddr print address
 * @param prefixFirst string to prefix first line with
 * @param prefix string to prefix each additional line
 */
void hex_dump(void *data, int size, int printAddr, const char * prefixFirst, const char * prefix) {

	unsigned char *p = (unsigned char *)data;
	unsigned char c;
	int n;
	char bytestr[4] = {0};
	char addrstr[10] = {0};
	char hexstr[ 16*3 + 5] = {0};
	char charstr[16*1 + 5] = {0};

	if (!prefixFirst) {
		prefixFirst = NULL;
	}

	if (!prefix) {
		prefix = NULL;
	}

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
			const char * p = (n <= 16) ? prefixFirst : prefix;
			if (printAddr) {
				printf("%s[%4.4s]   %-50.50s  %s\n", p, addrstr, hexstr, charstr);
			} else {
				printf("%s%-50.50s  %s\n", p, hexstr, charstr);
			}
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
		const char * p = (n <= 16) ? prefixFirst : prefix;
		if (printAddr) {
			printf("%s[%4.4s]   %-50.50s  %s\n", p, addrstr, hexstr, charstr);
		} else {
			printf("%s%-50.50s  %s\n", p, hexstr, charstr);
		}
	}
}
