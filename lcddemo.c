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
 * lcddemo - display some messages on LUNA-88K's front panel LCD
 */

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/ioctl.h>
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

int
main(int argc, char **argv)
{
	char buf[80 + 1];
	double la[3];

	lcd_fd = open(LCD_DEVICE, O_RDWR);

	if (lcd_fd == -1) {
		perror("Can not open /dev/lcd");
		exit(1);
	}

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
		strlcpy(buf, "OpenBSD/luna88k                         ", 41);
		strlcat(buf, "    on LUNA-88K2                        ", 81);
		write(lcd_fd, buf, 80);

		sleep(wait);

		getloadavg(la, 3);
		/*	      0123456789012345678901234567890123456789 */
		strlcpy(buf, "Load Average:                           ", 41);
		snprintf(&buf[40], 41,
			     "  %4.2f %4.2f %4.2f                        ",
			     la[0], la[1], la[2]);
		write(lcd_fd, buf, 80);

		sleep(wait);

		/* loop until IGINT or SIGTERM is received */
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
