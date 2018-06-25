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

	//LAB 3: Your code here.
	// debug_mem();
	// show_env(curenv);
	int sleeping_process_exists = 0;
	long long monotonic_time = nanosec_from_timer();
	curenv->env_time.tv_nsec += monotonic_time - curenv->env_time_start;
	normalize_time(&curenv->env_time);

	// check if we need to wakeup sleeping process
	time_t current_time = gettime();

	// we don't want to halt while have some sleeping processes
	do {
	 	sleeping_process_exists = 0;
		for (int i = 0;  i < NENV; i++) {
			if (envs[i].env_status == ENV_NOT_RUNNABLE && envs[i].env_sleep_clock_type) {
				sleeping_process_exists = 1;
				switch(envs[i].env_sleep_clock_type) {
					case CLOCK_REALTIME:
						if (current_time > envs[i].env_sleep_until) {
							envs[i].env_sleep_clock_type = 0;
							envs[i].env_status = ENV_RUNNABLE;
						}
						break;
					case CLOCK_MONOTONIC:
						if (monotonic_time > envs[i].env_sleep_until) {

							envs[i].env_sleep_clock_type = 0;
							envs[i].env_status = ENV_RUNNABLE;
						}
						break;
				}
			}
		}

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
		if (sleeping_process_exists) {
			monotonic_time = nanosec_from_timer();
			current_time = gettime();
		}
	
	} while (sleeping_process_exists);

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

