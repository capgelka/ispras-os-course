// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;addr=addr;
	uint32_t err = utf->utf_err;err=err;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 9: Your code here.
	int rc;

	if (~err & FEC_WR) {
		panic("pgfault: fault in va %p caused by write\n", addr);
	}

	if (~uvpt[PGNUM(addr)] & PTE_COW) {
		panic("pgfault: fault in va %p is not a fault caused by copy-on-write\n", addr);
	}

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.
	//   No need to explicitly delete the old page's mapping.

	// LAB 9: Your code here.
	
	addr = ROUNDDOWN(addr, PGSIZE);

	rc = sys_page_alloc(0, PFTEMP, PTE_P | PTE_U | PTE_W);
	if (rc) {
		panic("sys_page_alloc fails: %i\n", rc);
	}

	memmove(PFTEMP, addr, PGSIZE);

	rc = sys_page_map(0, PFTEMP, 0, addr, PTE_P|PTE_U|PTE_W);
	if (rc) {
		panic("sys_page_map fails: %i\n", rc);
	}

	rc = sys_page_unmap(0, PFTEMP);
	if (rc) {
		panic("sys_page_unmap fails: %i\n", rc);
	}
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	// LAB 9: Your code here.
	void* va = (void *) (pn * PGSIZE);
	int perm = uvpt[pn] & PTE_SYSCALL;
	if (!(perm & PTE_SHARE) && ((perm & PTE_W) || (perm & PTE_COW))) {
		perm &= ~PTE_W;
		perm |= PTE_COW;
	}

	int rc = sys_page_map(0, va, envid, va, perm);
	if (rc) {
		panic("sys_page_map: %i", rc);
	}
	if (perm & PTE_SHARE) {
		return 0;
	}

	rc = sys_page_map(0, va, 0, va, perm);
	if (rc) {
		panic("sys_page_map: %i", rc);
	}

	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 9: Your code here.
	extern void _pgfault_upcall(void);

	envid_t envid;
	int rc;
	uintptr_t page_va;

	set_pgfault_handler(pgfault);

	envid = sys_exofork();
	if (envid < 0) {
		return envid;
	}

	if (envid > 0) { 
		for (page_va = 0; page_va < UTOP; page_va += PGSIZE) {
			if (page_va == UXSTACKTOP - PGSIZE) { // user exception stack
				continue;
			}
			if ((uvpd[PDX(page_va)] & PTE_P) && (uvpt[PGNUM(page_va)] & PTE_P)) {
				rc = duppage(envid, PGNUM(page_va));
				if (rc) {
					panic("duppage fails: %i\n", rc);
				}
			}
		}

		rc = sys_page_alloc(envid, (void *) (UXSTACKTOP - PGSIZE), PTE_P | PTE_U | PTE_W);
		if (rc) {
			panic("sys_page_alloc fails: %i\n", rc);
		}

		rc = sys_env_set_pgfault_upcall(envid, _pgfault_upcall);
		if (rc) {
			panic("sys_env_set_pgfault_upcall fails: %i\n", rc);
		}

		rc = sys_env_set_status(envid, ENV_RUNNABLE);
		if (rc) {
			panic("sys_env_set_status fails: %i\n", rc);
		}
	} else { 
		thisenv = envs + ENVX(sys_getenvid());
	}

    return envid;
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
