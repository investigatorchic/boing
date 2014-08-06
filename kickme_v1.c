#include <sys/param.h>
#include <sys/systm.h>
#include <sys/sysproto.h>

int
sys_kickme(struct thread *td, struct kickme_args *uap)
{
	panic("kickme");
}
