#include <inc/mmu.h>
#include <inc/x86.h>
#include <inc/assert.h>
#include <inc/string.h>
#include <inc/vsyscall.h>

#include <kern/pmap.h>
#include <kern/trap.h>
#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/env.h>
#include <kern/syscall.h>
#include <kern/sched.h>
#include <kern/kclock.h>
#include <kern/picirq.h>
#include <kern/cpu.h>
#include <kern/vsyscall.h>
#include <kern/time.h>
#include <kern/tsc.h>

#ifndef debug
# define debug 0
#endif

long long monotonic_time_start;

static struct Taskstate ts;

/* For debugging, so print_trapframe can distinguish between printing
 * a saved trapframe and printing the current trapframe and print some
 * additional information in the latter case.
 */
static struct Trapframe *last_tf;

/* Interrupt descriptor table.  (Must be built at run time because
 * shifted function addresses can't be represented in relocation records.)
 */
struct Gatedesc idt[256] = { { 0 } };
struct Pseudodesc idt_pd = {
	sizeof(idt) - 1, (uint32_t) idt
};


static const char *trapname(int trapno)
{
	static const char * const excnames[] = {
		"Divide error",
		"Debug",
		"Non-Maskable Interrupt",
		"Breakpoint",
		"Overflow",
		"BOUND Range Exceeded",
		"Invalid Opcode",
		"Device Not Available",
		"Double Fault",
		"Coprocessor Segment Overrun",
		"Invalid TSS",
		"Segment Not Present",
		"Stack Fault",
		"General Protection",
		"Page Fault",
		"(unknown trap)",
		"x87 FPU Floating-Point Error",
		"Alignment Check",
		"Machine-Check",
		"SIMD Floating-Point Exception"
	};

	if (trapno < sizeof(excnames)/sizeof(excnames[0]))
		return excnames[trapno];
	if (trapno == T_SYSCALL)
		return "System call";
	if (trapno >= IRQ_OFFSET && trapno < IRQ_OFFSET + 16)
		return "Hardware Interrupt";
	return "(unknown trap)";
}

void trap_divide();
void trap_debug();
void trap_nmi();
void trap_brkpt();
void trap_oflow();
void trap_bound();
void trap_illop();
void trap_device();
void trap_dblflt();
void trap_tss();
void trap_segnp();
void trap_stack();
void trap_gpflt();
void trap_pgflt();
void trap_fperr();
void trap_align();
void trap_mchk();
void trap_simderr();
void trap_syscall();

void irq_timer();
void irq_kbd();
void irq_serial();
void irq_spurious();
void irq_ide();
void irq_error();

void
trap_init(void)
{
//	extern struct Segdesc gdt[];

	// LAB 8: Your code here.
	// extern void (*ivectors[])();
	// for (int i = 0; i < 20; i++) {
	// 	SETGATE(idt[i], 0, GD_KT, ivectors[i], 0);
	// }


	SETGATE(idt[T_DIVIDE], 0, GD_KT, trap_divide, 0);
	SETGATE(idt[T_DEBUG] , 0, GD_KT, trap_debug , 0);
	SETGATE(idt[T_NMI]   , 0, GD_KT, trap_nmi   , 0);
	SETGATE(idt[T_OFLOW] , 0, GD_KT, trap_oflow , 0);
	SETGATE(idt[T_BOUND] , 0, GD_KT, trap_bound , 0);
	SETGATE(idt[T_ILLOP] , 0, GD_KT, trap_illop , 0);
	SETGATE(idt[T_DEVICE], 0, GD_KT, trap_device, 0);
	SETGATE(idt[T_DBLFLT], 0, GD_KT, trap_dblflt, 0);
	SETGATE(idt[T_TSS]   , 0, GD_KT, trap_tss,    0);
	SETGATE(idt[T_SEGNP] , 0, GD_KT, trap_segnp,  0);
	SETGATE(idt[T_STACK] , 0, GD_KT, trap_stack,  0);
	SETGATE(idt[T_GPFLT] , 0, GD_KT, trap_gpflt,  0);
	SETGATE(idt[T_PGFLT] , 0, GD_KT, trap_pgflt,  0);
	SETGATE(idt[T_FPERR] , 0, GD_KT, trap_fperr,  0);
	SETGATE(idt[T_ALIGN] , 0, GD_KT, trap_align,  0);
	SETGATE(idt[T_MCHK]  , 0, GD_KT, trap_mchk ,  0);
	SETGATE(idt[T_SIMDERR], 0, GD_KT, trap_simderr, 0);

	SETGATE(idt[T_BRKPT], 0, GD_KT, trap_brkpt, 3);
	SETGATE(idt[T_SYSCALL], 0, GD_KT, trap_syscall, 3);

	SETGATE(idt[IRQ_OFFSET + IRQ_TIMER], 0, GD_KT, irq_timer, 0);
	SETGATE(idt[IRQ_OFFSET + IRQ_KBD], 0, GD_KT, irq_kbd, 0);
	SETGATE(idt[IRQ_OFFSET + IRQ_SERIAL], 0, GD_KT, irq_serial, 0);
	SETGATE(idt[IRQ_OFFSET + IRQ_SPURIOUS], 0, GD_KT, irq_spurious, 0);
	SETGATE(idt[IRQ_OFFSET + IRQ_IDE], 0, GD_KT, irq_ide, 0);
	SETGATE(idt[IRQ_OFFSET + IRQ_ERROR], 0, GD_KT, irq_error, 0);

	// Per-CPU setup 
	trap_init_percpu();
}

// Initialize and load the per-CPU TSS and IDT
void
trap_init_percpu(void)
{
	// Setup a TSS so that we get the right stack
	// when we trap to the kernel.
	ts.ts_esp0 = KSTACKTOP;
	ts.ts_ss0 = GD_KD;

	// Initialize the TSS slot of the gdt.
	gdt[GD_TSS0 >> 3] = SEG16(STS_T32A, (uint32_t) (&ts),
					sizeof(struct Taskstate), 0);
	gdt[GD_TSS0 >> 3].sd_s = 0;

	// Load the TSS selector (like other segment selectors, the
	// bottom three bits are special; we leave them 0)
	ltr(GD_TSS0);

	// Load the IDT
	lidt(&idt_pd);
}


void
clock_idt_init(void)
{
	extern void (*clock_thdlr)(void);
	// init idt structure
	SETGATE(idt[IRQ_OFFSET + IRQ_CLOCK], 0, GD_KT, (int)(&clock_thdlr), 0);
	lidt(&idt_pd);

	patch_year(); // for time from 1970
	vsys[VSYS_gettime] = gettime();
	clock_getres(CLOCK_MONOTONIC, (struct timespec*) vsys + VSYS_CLOCK_MONOTONIC);
	clock_getres(CLOCK_REALTIME, (struct timespec*) vsys + VSYS_CLOCK_REALTIME);
	monotonic_time_start = nanosec_from_timer();
}


void
print_trapframe(struct Trapframe *tf)
{
	cprintf("TRAP frame at %p\n", tf);
	print_regs(&tf->tf_regs);
	cprintf("  es   0x----%04x\n", tf->tf_es);
	cprintf("  ds   0x----%04x\n", tf->tf_ds);
	cprintf("  trap 0x%08x %s\n", tf->tf_trapno, trapname(tf->tf_trapno));
	// If this trap was a page fault that just happened
	// (so %cr2 is meaningful), print the faulting linear address.
	if (tf == last_tf && tf->tf_trapno == T_PGFLT)
		cprintf("  cr2  0x%08x\n", rcr2());
	cprintf("  err  0x%08x", tf->tf_err);
	// For page faults, print decoded fault error code:
	// U/K=fault occurred in user/kernel mode
	// W/R=a write/read caused the fault
	// PR=a protection violation caused the fault (NP=page not present).
	if (tf->tf_trapno == T_PGFLT)
		cprintf(" [%s, %s, %s]\n",
			tf->tf_err & 4 ? "user" : "kernel",
			tf->tf_err & 2 ? "write" : "read",
			tf->tf_err & 1 ? "protection" : "not-present");
	else
		cprintf("\n");
	cprintf("  eip  0x%08x\n", tf->tf_eip);
	cprintf("  cs   0x----%04x\n", tf->tf_cs);
	cprintf("  flag 0x%08x\n", tf->tf_eflags);
	cprintf("  esp  0x%08x\n", tf->tf_esp);
	cprintf("  ss   0x----%04x\n", tf->tf_ss);
}

void
print_regs(struct PushRegs *regs)
{
	cprintf("  edi  0x%08x\n", regs->reg_edi);
	cprintf("  esi  0x%08x\n", regs->reg_esi);
	cprintf("  ebp  0x%08x\n", regs->reg_ebp);
	cprintf("  oesp 0x%08x\n", regs->reg_oesp);
	cprintf("  ebx  0x%08x\n", regs->reg_ebx);
	cprintf("  edx  0x%08x\n", regs->reg_edx);
	cprintf("  ecx  0x%08x\n", regs->reg_ecx);
	cprintf("  eax  0x%08x\n", regs->reg_eax);
}


static void
trap_dispatch(struct Trapframe *tf)
{
	// Handle processor exceptions.

	// Handle spurious interrupts
	// The hardware sometimes raises these because of noise on the
	// IRQ line or other reasons. We don't care.
	//
	if (tf->tf_trapno == T_PGFLT) {
		//cprintf("Page fault interrupt\n");
		page_fault_handler(tf);
		return;
	}

	if (tf->tf_trapno == IRQ_OFFSET + IRQ_SPURIOUS) {
		cprintf("Spurious interrupt on irq 7\n");
		print_trapframe(tf);
		return;
	}

	if (tf->tf_trapno == T_BRKPT) {
		monitor(tf);
		return;
	}

	if (tf->tf_trapno == IRQ_OFFSET + IRQ_KBD) {
		kbd_intr();
		return;
	}

	if (tf->tf_trapno == IRQ_OFFSET + IRQ_SERIAL) {
		serial_intr();
		return;
	}

	if (tf->tf_trapno == IRQ_OFFSET + IRQ_CLOCK) {
		rtc_check_status();
		pic_send_eoi(IRQ_CLOCK);

		vsys[VSYS_gettime] = gettime();
		sched_yield();
		return;
	}

	if (tf->tf_trapno == T_SYSCALL) {
		tf->tf_regs.reg_eax = syscall(
			tf->tf_regs.reg_eax,
			tf->tf_regs.reg_edx,
			tf->tf_regs.reg_ecx,
			tf->tf_regs.reg_ebx,
			tf->tf_regs.reg_edi,
			tf->tf_regs.reg_esi
		);
		return;
	}

	// Handle keyboard and serial interrupts.
	// LAB 11: Your code here.
	print_trapframe(tf);
	if (tf->tf_cs == GD_KT) {
		panic("unhandled trap in kernel");
	} else {
		env_destroy(curenv);
	}
}

void
trap(struct Trapframe *tf)
{
	// The environment may have set DF and some versions
	// of GCC rely on DF being clear
	asm volatile("cld" ::: "cc");

	// Halt the CPU if some other CPU has called panic()
	extern char *panicstr;
	if (panicstr)
		asm volatile("hlt");

	// Check that interrupts are disabled.  If this assertion
	// fails, DO NOT be tempted to fix it by inserting a "cli" in
	// the interrupt path.
	assert(!(read_eflags() & FL_IF));

	if (debug) {
		cprintf("Incoming TRAP frame at %p\n", tf);
	}

	assert(curenv);

	// Garbage collect if current enviroment is a zombie
	if (curenv->env_status == ENV_DYING) {
		env_free(curenv);
		curenv = NULL;
		sched_yield();
	}

	// Copy trap frame (which is currently on the stack)
	// into 'curenv->env_tf', so that running the environment
	// will restart at the trap point.
	curenv->env_tf = *tf;
	// The trapframe on the stack should be ignored from here on.
	tf = &curenv->env_tf;

	// Record that tf is the last real trapframe so
	// print_trapframe can print some additional information.
	last_tf = tf;

	// Dispatch based on what type of trap occurred
	trap_dispatch(tf);

	// If we made it to this point, then no other environment was
	// scheduled, so we should return to the current environment
	// if doing so makes sense.
	if (curenv && curenv->env_status == ENV_RUNNING) {
		curenv->env_time.tv_nsec += nanosec_from_timer() - curenv->env_time_start;
		normalize_time(&curenv->env_time);
		env_run(curenv);
	}
	else
		sched_yield();
}


void
page_fault_handler(struct Trapframe *tf)
{
	uint32_t fault_va;

	// Read processor's CR2 register to find the faulting address
	fault_va = rcr2();

	// Handle kernel-mode page faults.

	// LAB 8: Your code here.
	if (tf->tf_cs == GD_KT) {
		panic(
			"[0x%08x] page fault in kernel, fault va: 0x%08x",
			tf->tf_eip,
			fault_va
		);
	}

	// We've already handled kernel-mode exceptions, so if we get here,
	// the page fault happened in user mode.

	// Call the environment's page fault upcall, if one exists.  Set up a
	// page fault stack frame on the user exception stack (below
	// UXSTACKTOP), then branch to curenv->env_pgfault_upcall.
	//
	// The page fault upcall might cause another page fault, in which case
	// we branch to the page fault upcall recursively, pushing another
	// page fault stack frame on top of the user exception stack.
	//
	// The trap handler needs one word of scratch space at the top of the
	// trap-time stack in order to return.  In the non-recursive case, we
	// don't have to worry about this because the top of the regular user
	// stack is free.  In the recursive case, this means we have to leave
	// an extra word between the current top of the exception stack and
	// the new stack frame because the exception stack _is_ the trap-time
	// stack.
	//
	// If there's no page fault upcall, the environment didn't allocate a
	// page for its exception stack or can't write to it, or the exception
	// stack overflows, then destroy the environment that caused the fault.
	// Note that the grade script assumes you will first check for the page
	// fault upcall and print the "user fault va" message below if there is
	// none.  The remaining three checks can be combined into a single test.
	//
	// Hints:
	//   user_mem_assert() and env_run() are useful here.
	//   To change what the user environment runs, modify 'curenv->env_tf'
	//   (the 'tf' variable points at 'curenv->env_tf').

	// LAB 9: Your code here.

	if (curenv->env_pgfault_upcall) {
		uintptr_t uxstacktop = UXSTACKTOP;
		// check if we allready in the exception stack
		if (tf->tf_esp >= UXSTACKTOP - PGSIZE && tf->tf_esp <= UXSTACKTOP -1) {
			uxstacktop = tf->tf_esp - 4;
		}
		uint32_t offset = sizeof(struct UTrapframe) + sizeof(uint32_t);
		user_mem_assert(curenv, (void *) uxstacktop - offset, offset, PTE_U | PTE_W);
		struct UTrapframe *utr = (struct UTrapframe *)(uxstacktop - offset);
		utr->utf_fault_va = fault_va;
		utr->utf_err = tf->tf_err;
		utr->utf_regs = tf->tf_regs;
		utr->utf_eflags = tf->tf_eflags;
		utr->utf_esp = tf->tf_esp;
		utr->utf_eip = tf->tf_eip;

		tf->tf_eip = (uintptr_t) curenv->env_pgfault_upcall;
		tf->tf_esp = (uintptr_t) utr;

		env_run(curenv);
	}

	// Destroy the environment that caused the fault.
	cprintf("[%08x] user fault va %08x ip %08x\n",
		curenv->env_id, fault_va, tf->tf_eip);
	print_trapframe(tf);
	env_destroy(curenv);
}

