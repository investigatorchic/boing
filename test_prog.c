#include <stdio.h>
#include <stdlib.h>

int
destroy_nonexist_lock(char *lk_name, char *lgname, int pid);

int
destroy_nonexist_lock(char *lk_name, char *lgname, int pid){
printf("destroy_nonexist_lock\n");
set_lgname(lgname, pid);
printf("Done set_lgname\n");
destroy_lock(lk_name);
printf("Done destroy_lock\n");

printf("destroy_nonexist_lock - Done.\n");
return 0;
}


int
main(int argc, char *argv[])
{
	destroy_nonexist_lock(argv[1], argv[2], atoi(argv[3]));
	return (0);
}
