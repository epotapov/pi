pi: pi.c
	gcc -o pi -Wall -Werror -std=c17 pi.c

.PHONY: clean
clean:
	rm pi
