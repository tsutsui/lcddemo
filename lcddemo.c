/*
 * Copyright (c) 2014 Kenji Aoyama.
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

/*
 * lcddemo - display some messages on LUNA68K and LUNA-88K's front panel LCD
 */

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <machine/lcd.h>

#define LCD_DEVICE "/dev/lcd"

/*
 * prototypes
 */
void sigcatch(int);

/*
 * global variables
 */
int lcd_fd;
int wait = 5;	/* sec. */
int ncpu;
int physmem;

int
main(int argc, char **argv)
{
	char buf[80 + 1];
	double la[3];
	int mib[2];
	size_t len;

	/* get number of cpu(s) */
	mib[0] = CTL_HW;
	mib[1] = HW_NCPU;
	len = sizeof(ncpu);
	if (sysctl(mib, 2, &ncpu, &len, NULL, 0) == -1) {
		perror("sysctl hw.ncpu failed");
		exit(1);
	}
	mib[1] = HW_PHYSMEM;
	len = sizeof(physmem);
	if (sysctl(mib, 2, &physmem, &len, NULL, 0) == -1) {
		perror("sysctl hw.physmem failed");
		exit(1);
	}

	/* open LCD device */
	lcd_fd = open(LCD_DEVICE, O_RDWR);
	if (lcd_fd == -1) {
		perror("Can not open " LCD_DEVICE);
		exit(1);
	}

	/* signal handler */
	if (signal(SIGINT, sigcatch) == SIG_ERR) {
		perror("Can not set SIGINT handler");
		exit(1);
	}

	if (signal(SIGTERM, sigcatch) == SIG_ERR) {
		perror("Can not set SIGTERM handler");
		exit(1);
	}

	/* clear LCD */
	ioctl(lcd_fd, LCDCLS);

	while (1) {
		/*	      0         1    *    2         3          */
		/*	      0123456789012345678901234567890123456789 */
		strlcpy(buf, "Welcome to NBUG                         ", 41);
		strlcat(buf, "   & OSC Nagoya!                        ", 81);
		write(lcd_fd, buf, 80);

		sleep(wait);

		/*	      0123456789012345678901234567890123456789 */
#ifdef __NetBSD__
		strlcpy(buf, "NetBSD/luna68k                          ", 41);
		strlcat(buf, "    on LUNA                             ", 81);
#else
		strlcpy(buf, "OpenBSD/luna88k                         ", 41);
		strlcat(buf, "    on LUNA-88K2                        ", 81);
#endif
		write(lcd_fd, buf, 80);

		sleep(wait);

		/*	      0123456789012345678901234567890123456789 */
		snprintf(buf, 41,
			     "With %d CPU%c                             ",
			     ncpu, ncpu == 1 ? ' ' : 's');
		snprintf(&buf[40], 41,
			     "  & %dMB memory                        ",
			     physmem / 1024 / 1024);
		write(lcd_fd, buf, 80);

		sleep(wait);

		getloadavg(la, 3);
		/*	      0123456789012345678901234567890123456789 */
		strlcpy(buf, "Load Average                            ", 41);
		snprintf(&buf[40], 41,
			     "  %4.2f %4.2f %4.2f                        ",
			     la[0], la[1], la[2]);
		write(lcd_fd, buf, 80);

		sleep(wait);

		/* loop until SIGINT or SIGTERM is received */
	}
}

void
sigcatch(int sig)
{
	/* reset signal handler to default setting */
	signal(SIGINT, SIG_DFL);
	signal(SIGTERM, SIG_DFL);

	/* reset LCD */
	ioctl(lcd_fd, LCDRESTORE);
	close(lcd_fd);

	exit(0);
}
