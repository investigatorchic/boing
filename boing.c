/*
 * boing.c - program to test the kickme(2) system call with.
 */

#include <stdio.h>

int
main(int argc, char *argv[])
{
	if (kickme())
		perror("kickme");
}
