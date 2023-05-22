#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

struct termios o_termios;

void disableRaw() {
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &o_termios);
}

void enableRaw() {
	tcgetattr(STDIN_FILENO, &o_termios);
	atexit(disableRaw);
	struct termios raw = o_termios;
	raw.c_lflag &= ~(ECHO | ICANON);
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main(int argc, char** argv) {
	enableRaw();
	char c;
	while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q') {
		if (iscntrl(c)) {
			printf("%d\n", c);
		}
		else {
			printf("%d ('%c')\n", c, c);
		}
	}
	return 0;
}
