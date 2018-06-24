#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/env.h>
#include <kern/monitor.h>


#include <kern/kclock.h>
#include <kern/picirq.h>
#include <kern/time.h>
#include <kern/tsc.h>


struct Taskstate cpu_ts;
void sched_halt(void);

// Choose a user environment to run and run it.
void
sched_yield(void)
{
	// Implement simple round-robin scheduling.
	//
	// Search through 'envs' for an ENV_RUNNABLE environment in
	// circular fashion starting just after the env was
	// last running.  Switch to the first such environment found.
	//
	// If no envs are runnable, but the environment previously
	// running is still ENV_RUNNING, it's okay to
	// choose that environment.
	//
	// If there are no runnable environments,
	// simply drop through to the code
	// below to halt the cpu.

// 	int8_t status = rtc_check_status();
// 	cprintf("status: %d\n", status);
// //	panic("NOOO");
// 	pic_send_eoi(8);
	//pic_send_eoi(status);

	//LAB 3: Your code here.
	//sched_halt();
	// debug_mem();
	// show_env(curenv);
	cprintf("SCHED update\n");
	curenv->env_time.tv_nsec += nanosec_from_timer() - curenv->env_time_start;
	normalize_time(&curenv->env_time);
	struct Env* next_env = NULL;
	int curr = (int) (curenv - envs);
	for (int i = curr; i < NENV; i++) {
		if (envs[i].env_status == ENV_RUNNABLE) {
			next_env = &envs[i];
			goto run_env;
		}
	}
	for (int i = 0; i < curr; i++) {
		if (envs[i].env_status == ENV_RUNNABLE) {
			next_env = &envs[i];
			goto run_env;
		}
	}
	if(!next_env && 
		(curenv->env_status == ENV_RUNNING || 
			curenv->env_status == ENV_RUNNABLE)) {
		next_env = curenv;
	}

run_env:
	if (next_env) {
		// show_env(next_env);
		env_run(next_env);
	}
	// sched_halt never returns
	sched_halt();
}

// Halt this CPU when there is nothing to do. Wait until the
// timer interrupt wakes it up. This function never returns.
//
void
sched_halt(void)
{
	int i;

	// For debugging and testing purposes, if there are no runnable
	// environments in the system, then drop into the kernel monitor.
	for (i = 0; i < NENV; i++) {
		if ((envs[i].env_status == ENV_RUNNABLE ||
		     envs[i].env_status == ENV_RUNNING ||
		     envs[i].env_status == ENV_DYING))
			break;
	}
	if (i == NENV) {
		cprintf("No runnable environments in the system!\n");
		while (1)
			monitor(NULL);
	}

	// Mark that no environment is running on CPU
	curenv = NULL;

	// Reset stack pointer, enable interrupts and then halt.
	asm volatile (
		"movl $0, %%ebp\n"
		"movl %0, %%esp\n"
		"pushl $0\n"
		"pushl $0\n"
		"sti\n"
		"hlt\n"
	: : "a" (cpu_ts.ts_esp0));
}

