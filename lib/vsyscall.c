#include <inc/vsyscall.h>
#include <inc/lib.h>

static inline int32_t
vsyscall(int num)
{
	// LAB 12: Your code here.
    cprintf("(VSYSCLALL) mem addr: %p, valL %d\n", (void *) UVSYS + num, *((int32_t *)UVSYS + num));
    cprintf("***** %d \n", vsys[num]);
	return vsys[num];
    //return *((int32_t *)UVSYS + num);
}

int vsys_gettime(void)
{
	return vsyscall(VSYS_gettime);
}