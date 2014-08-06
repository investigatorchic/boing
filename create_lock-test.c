#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char *argv[])
{
	create_lock(argv[1], atoi(argv[2]));
	return (0);
}
