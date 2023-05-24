/*
 * pi: a text-editor
 * Written by Edward P. 
 *
 * Based on the Kilo text-editor
 * 
 */

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>


struct termios o_termios;

void error(const char *s) {
	perror(s);
	exit(1);
}

void disableRaw() {
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &o_termios) == -1)
		error("tcsetattr error");
}

void enableRaw() {
	if (tcgetattr(STDIN_FILENO, &o_termios) == -1)
		error("tcgetattr error");
	atexit(disableRaw);
	struct termios raw = o_termios;
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
		error("tcsetattr error");
}

int main(int argc, char** argv) {
	enableRaw();
	while (1) {
		char c = 0;
		if (read(STDIN_FILENO, &c, 1) == -1)
			error("read error");
		if (iscntrl(c)) {
			printf("%d\r\n", c);
		}
		else {
			printf("%d ('%c')\r\n", c, c);
		}
		if (c == 'q')
			break;
	}
	return 0;
}
