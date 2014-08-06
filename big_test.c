#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char **argv)
{
	char usr_buf[9];
	int rv = -1;

	set_lgname("test123", 0);
	get_lgname(usr_buf);
	printf("usr_buf <%s>\n", usr_buf);
	if (strcmp(usr_buf, "test123")) {
		printf("set_lgname failed to set properly\n");
		exit(-1);
	}
	rv = create_lock("mylock", 0);
	printf("create_lock ret=%d\n", rv);
	rv = check_lock("mylock");
	printf("check_lock ret=%d\n", rv);
	if (rv != 0) {
		printf("check_lock returned something unexpected\n");
		exit(-1);
	}
	rv = set_lock("mylock");
	printf("set_lock returned %d\n", rv);
	if (rv != 0) {
		printf("set_lock returned something unexpected\n");
		exit(-1);
	}
	rv = clear_lock("mylock");
	printf("clear_lock returned %d\n", rv);
	if (rv != 0) {
		printf("clear_lock returned something unexpected\n");
		exit(-1);
	}
	rv = destroy_lock("mylock");
	printf("destroy_lock returned %d\n", rv);
	if (rv != 0) {
		printf("destroy_lock returned something unexpected\n");
		exit(-1);
	}

	exit (0);
}

