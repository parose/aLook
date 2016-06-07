all: aLook

aLook : aLook.c aLook.h
	gcc -Wall -Werror -O3 -o aLook aLook.c

clean:
	rm aLook
