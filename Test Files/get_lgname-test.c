#include <stdio.h>

int
main(int argc, char **argv)
{
	char usr_buf[10];
	usr_buf[0] = 0;

	if (argc == 1) {
		int r = get_lgname(usr_buf);
		printf("get_lgname returned %d and usr_buf is <%s>\n", r, usr_buf);
	}
	else {
		printf("Usage: %s\n", argv[0]);
	}
	return (0);
}

