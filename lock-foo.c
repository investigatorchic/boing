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
sys_set_lgname(struct thread *td, struct set_lgname_args *uap)
{

/*	 
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


	if (!valid(uap)) {
		return EINVAL;
	}

	int error;
	char usr_buf[9];
	struct lock_group_names *found = lgnames_root, *current = NULL;
	size_t copied = 0; 
/*	char *lgname = NULL; */

	if ((error = copyinstr(uap->lgn, &usr_buf, 8, &copied))) {
		return (error);
	}

	usr_buf[8]=0;
	
	mtx_lock(&lgname_lock);
	printf("found=");
	found = find_lgnames(usr_buf);
	printf("if (found != NULL)");
	if (found != NULL) {
		if (td->td_ucred->cr_uid == 0 || (td->td_ucred->cr_uid == found->owner)){ 
			current = find_lgnames(td->td_proc->lockgroupname);
		if (current){ 
			release_locks(current);
		}
		memcpy(td->td_proc->lockgroupname, usr_buf, 8);
		mtx_unlock(&lgname_lock);
		return(0);
	    }
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

		memset(found, 0, sizeof(struct lock_group_names));
		found->owner = td->td_ucred->cr_uid;
		memcpy(found->lg_name, usr_buf, 8);
		memcpy(td->td_proc->lockgroupname, usr_buf, 8);
		mtx_unlock(&lgname_lock);
		return 0;
	}
	 mtx_unlock(&lgname_lock);
	
	printf("%s\n", usr_buf); 
	printf("testing");
	return(0);
}

int
sys_get_lgname(struct thread *td, struct get_lgname_args *uap)
{
	printf("sys_get_lgname");
        return(EPERM);
}

int
sys_create_lock(struct thread *td, struct create_lock_args *uap)
{
	printf("sys_create_lock");
        return(EPERM);
}

int
sys_set_lock(struct thread *td, struct set_lock_args *uap)
{
	printf("sys_set_lock");
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


