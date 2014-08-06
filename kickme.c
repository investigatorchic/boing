#include <sys/param.h>
#include <sys/systm.h>
#include <sys/sysproto.h>
#include <sys/priv.h>

int
sys_kickme(struct thread *td, struct kickme_args *uap)
{
	if (!priv_check(td, PRIV_REBOOT))
		panic("kickme");
	return(EPERM);
}
