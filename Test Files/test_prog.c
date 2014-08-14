#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int
destroy_nonexist_lock(char *lk_name, char *lgname, int pid);

int
unlock_twice(char *lk_name, char *lgname, int pid, int perms, int slpflag);

int
perm_test(char *lk_name1, char *lk_name2, char *lk_name3);

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
unlock_twice(char *lk_name, char *lgname, int pid, int perms, int slpflag){
	set_lgname(lgname, pid);
	printf("Done set_lgname\n");
	create_lock(lk_name, perms);
	printf("Done create_lock\n");
	set_lock(lk_name, slpflag);
	printf("Done set_lock\n");
	clear_lock(lk_name);
	printf("Done clear_lock #1\n");
	clear_lock(lk_name);
	printf("Done clear_lock #2\n");
	return 0;
}

int
perm_test(char *lk_name1, char *lk_name2, char *lk_name3){
	assert(create_lock(lk_name1, 1)==0);
	assert(create_lock(lk_name2, 0)==0);
	assert(create_lock(lk_name3, 1)==0);
	setuid(1);
	printf("switched to uid %d\n", getuid());
	assert(set_lock(lk_name2)==0);
	assert(set_lock(lk_name3)!=0);
	assert(clear_lock(lk_name2)==0);    /* attempt to clear an already locked lock */
	assert(clear_lock(lk_name3)!=0);  /* attempt to clear an unlocked lock - should fail */
	assert(clear_lock(lk_name1)!=0); /* attempt to clear a locked but unowned lock - should fail */
	assert(destroy_lock(lk_name2)==0); 
	assert(destroy_lock(lk_name3)!=0);
	printf("Done. Works as expected.\n");

	exit (0);
}

int
main(int argc, char *argv[])
{
	char* test_name = argv[1];
	printf("test_name=%s\n", test_name);
	if (strcmp("destroy_nonexist_lock", test_name)==0) {
	destroy_nonexist_lock(argv[2], argv[3], atoi(argv[4]));
	return (0);
	}
	else if (strcmp("unlock_twice", test_name)==0){
	printf("unlock_twice\n");
 	unlock_twice(argv[2], argv[3], atoi(argv[4]), atoi(argv[5]), atoi(argv[6]));
	return (0);	
	}
	else if (strcmp("perm_test", test_name)==0){
	printf("perm_test\n");
	perm_test(argv[2], argv[3], argv[4]);
	return (0);
	}
	
}
