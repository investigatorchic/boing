#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/sysproto.h>
#include <sys/priv.h>
#include <sys/malloc.h>
#include <sys/proc.h>
#include <sys/lock.h>
#include <sys/rmlock.h>
#include <sys/mutex.h>
#include "definitions-bar.h"

MALLOC_DEFINE(M_LOCKER_FOO, "locker", "a kernel level locking mechanism");

static struct lock_group_names *lgnames_root = NULL;
static struct mtx lgname_lock;

int 
valid(struct set_lgname_args *uap)
{
	return uap->lgn!=NULL && uap->pid > 0 && uap->pid < PID_MAX; 
}

struct lock_group_names *
find_lgnames(char *lgn)
{
	struct lock_group_names *n;
	if (!lgn || !*lgn) return NULL;
	for(n = lgnames_root; n && strncmp(n->lg_name, lgn, 8) ; n = n->next);
	return n;
}

int
is_root(struct thread *td)
{
	return (td->td_ucred->cr_uid == 0); 
}

int
is_owner(struct thread *td, struct lock_group_names *n)
{
	return (td->td_ucred->cr_uid == n->owner);
}

char *
get_lgn(int pid, char *my_lg)
{
	struct proc *p;
	if (pid == 0) return my_lg;
	FOREACH_PROC_IN_SYSTEM(p)
		if(p->p_pid == pid) return p->lockgroupname;
	return NULL;
}

void 
release_locks(struct lock_group_names *gn)
{
	if (gn) {
			struct lock_name *l = gn->lname_root;
			for ( ; l ; l = l->next ){  
				if (mtx_owned(&(l->mutex))){
					mtx_unlock(&(l->mutex));
				}	
			}
		}
}

SYSINIT(locker, SI_SUB_SMP, SI_ORDER_ANY, init_lock, NULL);

void 
init_lock(void *unused)
{
	mtx_init(&lgname_lock, "init_lock", NULL, MTX_DEF);
}

int 
count_locks(struct lock_name *lock_root) 
{
        int count = 0;
        if (lock_root) {
        	struct lock_name *node = NULL;
        	for (node = lock_root->next; node; node = node->next)
               	{
              		++count;
                }
        }
        else
        {
                return EPERM;
        }
        return count;
}

struct lock_name 
*find_lock(struct lock_name *lname_root, char *lk_name)
{
	if (!lk_name || !*lk_name) return NULL;
	for( ; lname_root && lname_root->lk_name && strcmp(lname_root->lk_name, lk_name) ; lname_root = lname_root->next) ;
	return lname_root;
}



/*

 SYSTEM CALL NAME
   sys_set_lgname

 DESC
   set the lockgroup name for the given process.
   lockgroup is stored as part of the thread struct in sys/proc.h

 CONDITIONS
   1. lgn is at most 8 characters
   2. inherited by children (by virtue of being stored in the thread struct)
   3. If not root:
        a. You may set your lgn to an existing lgn if you created that lgn
        b. If an lgn doesnt alread exist, you can create it
        c. You may set the lgn of any process that you own
   4. If you are root:
        a. You may set your lgn to anything you want even an existing one not owned by you
        b. You may set the lgn of any process, even if it already has one set
   5. if pid is zero, it means 'set lgn for the current process'
*/

int
sys_set_lgname(struct thread *td, struct set_lgname_args *uap)
{
	if (!valid(uap)) {
		return EINVAL;
	}

	int error;
	char usr_buf[9];
	struct lock_group_names *found = lgnames_root, *current = NULL;
	size_t copied = 0; 
	char *a_lg = NULL;
	memset(usr_buf, 0, 9);

	if ((error = copyinstr(uap->lgn, &usr_buf, 8, &copied))) {
		return (error);
	}

	mtx_lock(&lgname_lock);
	found = find_lgnames(usr_buf);
	
	if (found == NULL) {
 	
                if (lgnames_root == NULL) {
                        lgnames_root = malloc(sizeof(struct lock_group_names), M_LOCKER_FOO, M_NOWAIT);
                        if (lgnames_root == NULL) {
                                return ENOMEM;
                        }
                        found = lgnames_root;
                }
                else
                {
                        for(found = lgnames_root ; found->next ; found = found->next );
                        found->next = malloc(sizeof(struct lock_group_names), M_LOCKER_FOO, M_NOWAIT);
                        if (found->next) {
                                found = found->next;
                        }
                        else {
                                return ENOMEM;
                        }
                }

                memset(found, 0, sizeof(struct lock_group_names));
                found->owner = td->td_ucred->cr_uid;
                memcpy(found->lg_name, usr_buf, 8);
                a_lg = get_lgn(uap->pid, td->td_proc->lockgroupname);
                mtx_unlock(&lgname_lock);
                printf("lgname_unlocked");
		if (a_lg == NULL){
			printf("a_lag == NULL");
                	mtx_unlock(&lgname_lock);
			return ESRCH;
		}

	memcpy(a_lg, usr_buf, 8);
	return 0;

	}  
	else
	{	printf("else2");
		if (is_root(td) || is_owner(td,found)){
			printf("is_root or is_owner");
                        a_lg = get_lgn(uap->pid, td->td_proc->lockgroupname);
                        if (a_lg == NULL) return ESRCH;
                        current = find_lgnames(a_lg);
                        if (current){
				printf("if current");
                                release_locks(current);
                        }
                        memcpy(a_lg, usr_buf, 8);
                        mtx_unlock(&lgname_lock);
                        return(0);
		}
        }
 	mtx_unlock(&lgname_lock);

        return(0);
}


int
sys_get_lgname(struct thread *td, struct get_lgname_args *uap)
{
	printf("sys_get_lgname");
	return (copyout(td->td_proc->lockgroupname, uap->lgn, 9));
}

int
sys_create_lock(struct thread *td, struct create_lock_args *uap)
{
	int error = EINVAL;
        char usr_buf[256];
	size_t copied = 0;
	int lock_count = 0;
	struct lock_group_names *found = NULL;
	struct lock_name *new_lock = NULL, *present_lock = NULL;

	memset(usr_buf, 0, 256);
	printf ("error = copyintsr1");
	if ((error = copyinstr(uap->name, &usr_buf, 255, &copied)))
                return (error);

	mtx_lock(&lgname_lock);

	found = find_lgnames(td->td_proc->lockgroupname);
	printf("!found return EPERM;");
	if (!found) return EPERM;
	
	lock_count = count_locks(found->lname_root);
	printf("lock_count==EPERM");
	if (lock_count==EPERM || lock_count >= 32) return EPERM;
	printf("if (found->lname_root");
	if (found->lname_root == NULL) {
		found->lname_root = malloc(sizeof(struct lock_name), M_LOCKER_FOO, M_NOWAIT);
		printf("2nd found->lname_root == NULL");
		if (found->lname_root == NULL) { 
			mtx_unlock(&lgname_lock); 
			return ENOMEM; 
		}
		new_lock = found->lname_root;
		memset(found->lname_root, 0, sizeof(struct lock_name));
		found->lname_root->perms = uap->perms;
		found->lname_root->creator = td->td_ucred->cr_uid;
		found->lname_root->lk_name = strdup(usr_buf, M_LOCKER_FOO);
		mtx_init(&(found->lname_root->mutex), "int_locking", NULL, MTX_DEF);
		mtx_unlock(&lgname_lock);
		return 0;
	}
	else
	{
		printf("else");
		present_lock->next = malloc(sizeof(struct lock_name), M_LOCKER_FOO, M_NOWAIT);
		printf("if present_lock->next == NULL");
		if (present_lock->next == NULL) { 
			mtx_unlock(&lgname_lock); 
			return ENOMEM; 
		}
		memset(present_lock->next, 0, sizeof(struct lock_name));
		mtx_init(&(present_lock->next->mutex), "int_locking", NULL, MTX_DEF);
		present_lock->next->perms = uap->perms;
		present_lock->next->creator = td->td_ucred->cr_uid;
		present_lock->next->lk_name = strdup(usr_buf, M_LOCKER_FOO);
		mtx_unlock(&lgname_lock);
		return 0;
	}

	mtx_unlock(&lgname_lock); 

	printf("sys_create_lock");
        return(EPERM);
}

int
sys_set_lock(struct thread *td, struct set_lock_args *uap)
{
	int error = EINVAL;
        char usr_buf[256];
	size_t copied = 0;
        struct lock_group_names *found = NULL;
        struct lock_name *lock = NULL;

	memset(usr_buf, 0, 256);
	if ((error = copyinstr(uap->name, &usr_buf, 255, &copied)))
                return (error);
	printf("error = copyinstr");
	usr_buf[255] = 0;

	mtx_lock(&lgname_lock);

        found = find_lgnames(td->td_proc->lockgroupname);
        if (found) {
		printf("found=find_lgnames");
		lock = find_lock(found->lname_root, usr_buf);
                if (lock) {
			printf("if lock");
			if (mtx_owned(&(lock->mutex))) {
				return 0;
			}
			if (lock->perms != 0 && lock->creator != td->td_ucred->cr_uid && !(td->td_ucred->cr_uid == 0)) {
				printf("lock->perms");
				mtx_unlock(&lgname_lock);
				return EPERM;
			}

			if (mtx_trylock(&(lock->mutex)) == 0) { 
				printf("mtx_trylock");
				if (uap->slpflg) { 
				printf("uap->slpflg");
					mtx_unlock(&lgname_lock); 
					mtx_lock(&(lock->mutex));
					return 0;
				}
			}
			else {
				printf("else"); 
				mtx_unlock(&lgname_lock);
				return 0;
			}
		}
	}

	mtx_unlock(&lgname_lock);
	return(EPERM);
}

int
sys_clear_lock(struct thread *td, struct clear_lock_args *uap)
{	
	printf("sys_clear_lock");
        return(EPERM);
}

int
sys_check_lock(struct thread *td, struct check_lock_args *uap)
{
	printf("sys_check_lock");
        return(EPERM);
}

int
sys_destroy_lock(struct thread *td, struct destroy_lock_args *uap)
{
	printf("sys_destroy_lock");
        return(EPERM);
}


