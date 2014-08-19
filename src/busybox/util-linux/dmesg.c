/* vi: set sw=4 ts=4: */
/*
 *
 * dmesg - display/control kernel ring buffer.
 *
 * Copyright 2006 Rob Landley <rob@landley.net>
 * Copyright 2006 Bernhard Reutner-Fischer <rep.nop@aon.at>
 *
 * Licensed under GPLv2, see file LICENSE in this source tree.
 */

//usage:#define dmesg_trivial_usage
//usage:       "[-c] [-n LEVEL] [-r] [-s SIZE] [-C]"
//usage:#define dmesg_full_usage "\n\n"
//usage:       "Print or control the kernel ring buffer\n"
//usage:     "\n	-c		Clear ring buffer after printing"
//usage:     "\n	-n LEVEL	Set console logging level"
//usage:     "\n	-r		Show level prefix"
//usage:     "\n	-s SIZE		Buffer size"
//usage:     "\n	-C		Colored output"

#include <sys/klog.h>
#include "libbb.h"

#define COLOR_DEFAULT 0
#define COLOR_WHITE   231
#define COLOR_YELLOW  226
#define COLOR_ORANGE  166
#define COLOR_RED     196

int dmesg_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int dmesg_main(int argc UNUSED_PARAM, char **argv)
{
	int len, level;
	char *buf;
	unsigned opts;
	enum {
		OPT_c = 1 << 0,
		OPT_s = 1 << 1,
		OPT_n = 1 << 2,
		OPT_r = 1 << 3,
		OPT_C = 1 << 4
	};

	opt_complementary = "s+:n+"; /* numeric */
	opts = getopt32(argv, "cs:n:rC", &len, &level);
	if (opts & OPT_n) {
		if (klogctl(8, NULL, (long) level))
			bb_perror_msg_and_die("klogctl");
		return EXIT_SUCCESS;
	}

	if (!(opts & OPT_s))
		len = klogctl(10, NULL, 0); /* read ring buffer size */
	if (len < 16*1024)
		len = 16*1024;
	if (len > 16*1024*1024)
		len = 16*1024*1024;

	buf = xmalloc(len);
	len = klogctl(3 + (opts & OPT_c), buf, len); /* read ring buffer */
	if (len < 0)
		bb_perror_msg_and_die("klogctl");
	if (len == 0)
		return EXIT_SUCCESS;


	if ((ENABLE_FEATURE_DMESG_PRETTY || (opts & OPT_C)) && !(opts & OPT_r)) {
		int last = '\n';
		int in = 0, l, color;
		char pfx[16], *lvl;

		/* Skip <[0-9]+> at the start of lines */
		while (1) {
			if (last == '\n' && buf[in] == '<') {
				if (opts & OPT_C) {
					lvl = buf + in + 1;
					sscanf(lvl, "%d", &level);

					switch (level) {
					case 1:
					case 2:
					case 3: color = COLOR_RED;    break;
					case 4: color = COLOR_ORANGE; break;
					case 5: color = COLOR_YELLOW; break;
					case 7: color = COLOR_WHITE;  break;
					case 6: // common dmesg info
					default: color = COLOR_DEFAULT;
					}

					if (color != COLOR_DEFAULT)
						l = sprintf(pfx, "%c[%d;%d;%dm",
							0x1B, 38, 5, color);
					else
						l = sprintf(pfx, "%c[%dm", 0x1B, 0);

					full_write(STDOUT_FILENO, pfx, l);
				}
				while (buf[in++] != '>' && in < len)
					;
			} else {
				last = buf[in++];
				putchar(last);
			}
			if (in >= len)
				break;
		}

		if (opts & OPT_C) {
			/* Reset default terminal color */
			l = sprintf(pfx, "%c[%dm", 0x1B, 0);
			full_write(STDOUT_FILENO, pfx, l);
		}

		/* Make sure we end with a newline */
		if (last != '\n')
			bb_putchar('\n');

	} else {
		full_write(STDOUT_FILENO, buf, len);
		if (buf[len-1] != '\n')
			bb_putchar('\n');
	}

	if (ENABLE_FEATURE_CLEAN_UP) free(buf);

	return EXIT_SUCCESS;
}
