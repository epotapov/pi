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
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#define CTRL_K(k) ((k) & 0x1f)

enum specialKeys {
	LEFT = 1000,
  RIGHT,
  UP,
  DOWN,
  PAGE_UP,
  PAGE_DOWN
};

struct config {
	int x;
	int y;
	int rows;
	int cols;
	struct termios o_termios;
};

struct config conf;

struct buffer {
	char *buff;
	int len;
};

void append(struct buffer *b, const char *s, int len) {
	char *new_string = realloc(b->buff, b->len + len);
	if (!new_string)
		return;
	memcpy(&new_string[b->len], s, len);
	b->buff = new_string;
	b->len += len;
}

void b_free(struct buffer *b) {
	free(b->buff);
}


// function to print out errors and close program
void error(const char *s) {
	write(STDOUT_FILENO, "\x1b[2J", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);
	perror(s);
	exit(1);
}

// bring terminal back into canonical mode
void disableRaw() {
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &conf.o_termios) == -1)
		error("tcsetattr error");
}

// all the things to put a terminal into "raw" mode
void enableRaw() {
	if (tcgetattr(STDIN_FILENO, &conf.o_termios) == -1)
		error("tcgetattr error");
	atexit(disableRaw);
	struct termios raw = conf.o_termios;
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
		error("tcsetattr error");
}

//get the keys pressed from read
int readKey() {
	int num;
	char c;
	while ((num = read(STDIN_FILENO, &c, 1)) != 1) {
		if (num == -1) {
			error("read error");
		}
	}
	if (c == '\x1b') {
		char seq[3];
		if (read(STDIN_FILENO, &seq[0], 1) != 1)
			return '\x1b';
		if (read(STDIN_FILENO, &seq[1], 1) != 1)
			return '\x1b';
		if (seq[0] == '[') {
			if (seq[1] >= '0' && seq[1] <= '9') {
        if (read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';
        if (seq[2] == '~') {
          switch (seq[1]) {
            case '5': return PAGE_UP;
            case '6': return PAGE_DOWN;
          }
        }
      } else {
				switch (seq[1]) {
					case 'A':
						return UP; //up arrow
					case 'B':
						return DOWN; //down arrow
					case 'C':
						return RIGHT; //right arrow
					case 'D':
						return LEFT; //left arrow
				}
			}
		}
		return '\x1b';
	}
	return c;
}

// parse the key and check for the key pressed
void processKeypresses() {
	int c = readKey();
	switch (c) {
		case CTRL_K('q'):
			write(STDOUT_FILENO, "\x1b[2J", 4);
			write(STDOUT_FILENO, "\x1b[H", 3);
			exit(0);
			break;
		case UP:
			if (conf.y != 0)
				conf.y--;
			break;
		case  LEFT:
			if (conf.x != 0)
				conf.x--;
			break;
		case DOWN:
			if (conf.y != conf.rows - 1)
				conf.y++;
			break;
		case RIGHT:
			if (conf.x != conf.cols - 1)
				conf.x++;
			break;
		case PAGE_UP:
			if (conf.y != 0)
				conf.y--;
			break;
		case PAGE_DOWN:
			if (conf.y != conf.rows - 1)
				conf.y++;
	}
}

// function that obtains the size of the terminal (easy way)
int getWinSize(int *rows, int *cols) {
	struct winsize ws;
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
		return -1;
	}
	*cols = ws.ws_col;
	*rows = ws.ws_row;
	return 0;
}

// draws tildes like vim to show parts of the file that isn't present
void drawLines(struct buffer *b) {
	for (int i = 0; i < conf.rows; i++) {
		if (i == conf.rows / 2) {
			char *message = "π a text-editor";
			int message_size = strlen(message);
			if (message_size > conf.cols)
				message_size = conf.cols;
			int padding = (conf.cols - message_size) / 2;
      if (padding) {
        append(b, "~", 1);
        padding--;
      }
      while (padding--)
				append(b, " ", 1);
			append(b, message, message_size);
		}
		else {
			append(b, "~", 1);
		}
		append(b, "\x1b[K", 3);
		if (i != conf.rows - 1)
			append(b, "\r\n", 2);
	}
}

void refreshScreen() {
	struct buffer b = {0};
	append(&b, "\x1b[?25l", 6);
	append(&b, "\x1b[H", 3);
	drawLines(&b);
	char buf[32];
	snprintf(buf, sizeof(buf), "\x1b[%d;%dH", conf.y + 1, conf.x + 1);
  append(&b, buf, strlen(buf));
	append(&b, "\x1b[?25h", 6);
	write(STDOUT_FILENO, b.buff, b.len);
	b_free(&b);
}

void initPi() {
	conf.x = 10;
	conf.y = 10;
	if (getWinSize(&conf.rows, &conf.cols) == -1)
		error("getWinSize error");
}

int main(int argc, char** argv) {
	enableRaw();
	initPi();
	while (1) {
		refreshScreen();
		processKeypresses();
	}
	return 0;
}
