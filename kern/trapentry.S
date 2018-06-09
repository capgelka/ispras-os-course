/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>

#include <kern/picirq.h>

#ifdef CONFIG_KSPACE
.comm intr_ret_eip, 4
.comm intr_ebp_reg, 4
.comm intr_esp_reg, 4
.comm intr_cs, 4
.comm intr_eflags, 4
#endif


###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps


.text

.globl _alltraps
.type _alltraps, @function;
.align 2
_alltraps:
	pushl %ds;
	pushl %es;
	pushal;
	movw $GD_KD, %ax;
	movw %ax, %ds;
	movw %ax, %es;
	pushl %esp;
	call trap;

.globl clock_thdlr
.type clock_thdlr, @function;
.align 2;
#ifdef CONFIG_KSPACE
clock_thdlr:
	popl intr_ret_eip
	popl intr_cs
	popl intr_eflags
	movl %ebp, intr_ebp_reg
	movl %esp, intr_esp_reg
	movl $0x0,%ebp
	movl $(bootstacktop),%esp
	pushl $GD_KD
	pushl intr_esp_reg
	pushl intr_eflags
	pushl intr_cs
	pushl intr_ret_eip
	pushl $0
	pushl $(IRQ_OFFSET + IRQ_CLOCK)
	pushl %ds
	pushl %es

	pushl %eax
	pushl %ecx
	pushl %edx
	pushl %ebx
	pushl intr_esp_reg
	pushl intr_ebp_reg
	pushl %esi
	pushl %edi

	pushl %esp  /* trap(%esp) */
	call trap
	jmp .
#else
TRAPHANDLER_NOEC(clock_thdlr, IRQ_OFFSET + IRQ_CLOCK)
// LAB 8: Your code here.
TRAPHANDLER_NOEC(trap_divide, T_DIVIDE)
TRAPHANDLER_NOEC(trap_debug, T_DEBUG)
TRAPHANDLER_NOEC(trap_nmi, T_NMI)
TRAPHANDLER_NOEC(trap_brkpt, T_BRKPT)
TRAPHANDLER_NOEC(trap_oflow, T_OFLOW)
TRAPHANDLER_NOEC(trap_bound, T_BOUND)
TRAPHANDLER_NOEC(trap_illop, T_ILLOP)
TRAPHANDLER_NOEC(trap_device, T_DEVICE)
TRAPHANDLER(trap_dblflt, T_DBLFLT)
TRAPHANDLER(trap_tss, T_TSS)
TRAPHANDLER(trap_segnp, T_SEGNP)
TRAPHANDLER(trap_stack, T_STACK)
TRAPHANDLER(trap_gpflt, T_GPFLT)
TRAPHANDLER(trap_pgflt, T_PGFLT)
TRAPHANDLER_NOEC(trap_fperr, T_FPERR)
TRAPHANDLER(trap_align, T_ALIGN)
TRAPHANDLER_NOEC(trap_mchk, T_MCHK)
TRAPHANDLER_NOEC(trap_simderr, T_SIMDERR)
TRAPHANDLER_NOEC(trap_syscall, T_SYSCALL)
TRAPHANDLER_NOEC(irq_timer, IRQ_OFFSET + IRQ_TIMER)
TRAPHANDLER_NOEC(irq_kbd, IRQ_OFFSET + IRQ_KBD)
TRAPHANDLER_NOEC(irq_serial, IRQ_OFFSET + IRQ_SERIAL)
TRAPHANDLER_NOEC(irq_spurious, IRQ_OFFSET + IRQ_SPURIOUS)
TRAPHANDLER_NOEC(irq_ide, IRQ_OFFSET + IRQ_IDE)
TRAPHANDLER_NOEC(irq_error, IRQ_OFFSET + IRQ_ERROR)
#endif
