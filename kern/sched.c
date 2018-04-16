#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/env.h>
#include <kern/monitor.h>


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

	//LAB 3: Your code here.
	//sched_halt();
	// debug_mem();
	//show_env(curenv);
	struct Env* next_env = NULL;
	int curr = (find_env_num(curenv) + 1) % NENV;
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
	// while (next_env->env_link) {
	// 	next_env = next_env->env_link;
	// 	cprintf("!!!!!possible env enty point: 0x%x\n",
	// 		next_env->env_tf.tf_eip);

	// 	if (next_env->env_status == ENV_RUNNABLE ||
	// 		next_env == curenv) {
	// 				cprintf("111111break! %d %d\n",next_env->env_status == ENV_RUNNABLE, next_env == curenv);
	// 		break;
	// 	}
	// 	cprintf("111111not break! %d %d\n",next_env->env_status == ENV_RUNNABLE, next_env == curenv);
	// 	/* code */
	// }

	// if ((!next_env || next_env->env_status != ENV_RUNNABLE) &&
	// 	curenv->env_status == ENV_RUNNING) {
	// 	cprintf("env not changed!\n");
	// 	next_env = curenv;
	// } else {
	// 	cprintf("else in scheduler\n");
	// 	//next_env = envs;
	// }
	// //env_run(envs);
	// cprintf("ENV GOIG TO RUN : %p, curenv: %p\n\n\n", next_env, curenv);
	// // debug_mem();
run_env:
	if (next_env) {
		// cprintf("WTF!&&&\n\nj]n");
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
	// cprintf("HALTING\n");
	// For debugging and testing purposes, if there are no runnable
	// environments in the system, then drop into the kernel monitor.
	for (i = 0; i < NENV; i++) {
		if ((envs[i].env_status == ENV_RUNNABLE ||
		     envs[i].env_status == ENV_RUNNING)) {

			// show_env(&envs[i]);
			// debug_mem();
			break;
		}

	}
	// cprintf("%d %d", i, NENV);
	if (i == NENV) {
		cprintf("No runnable environments in the system!\n");
		while (1)
			monitor(NULL);
	}
		// cprintf("HALTING\n");
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
	// cprintf("HALTED\n");
}

