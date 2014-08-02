#ifndef __DEFINITIONS_BAR__
#define __DEFINITIONS_BAR__

#include <sys/lock.h>
#include <sys/rmlock.h>
#include <sys/mutex.h>

MALLOC_DECLARE(M_LOCKER_FOO);

struct lock_name {
        char *lk_name;
        char  state;
        int   perms;
        uid_t   creator;
        pid_t   heldby;
        struct lock_name *next;
        struct mtx      mutex;
};

struct lock_group_names {
        char lg_name[9];
        int pid;
        uid_t                    owner;
        struct lock_group_names *next;
        struct lock_name *lname_root;

};


int valid(struct set_lgname_args *uap);

struct lock_group_names * find_lgnames(char *lgn);

int is_root(struct thread *td);

int is_owner(struct thread *td, struct lock_group_names *n);

void release_locks(struct lock_group_names *gn);

void init_lock(void *);

char *get_lgn(int pid, char *my_lg);

int count_locks(struct lock_name *lock_root);








#endif /* __DEFINITIONS_BAR__ */
