#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char *argv[])
{
	check_lock(argv[1], atoi(argv[2]));
	return (0);
}
