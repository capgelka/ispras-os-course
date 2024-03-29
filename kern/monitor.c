// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/x86.h>

#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/kdebug.h>
#include <kern/tsc.h>
#include <kern/pmap.h>
#include <kern/trap.h>

#define CMDBUF_SIZE	80	// enough for one VGA text line


struct Command {
	const char *name;
	const char *desc;
	// return -1 to force monitor to exit
	int (*func)(int argc, char** argv, struct Trapframe* tf);
};

static struct Command commands[] = {
	{ "help", "Display this list of commands", mon_help },
	{ "kerninfo", "Display information about the kernel", mon_kerninfo },
	{ "debug",  "Display debug string", mon_debug },
	{ "backtrace",  "Display stack backtrace", mon_backtrace },
	{ "timer_start",  "Start tcs timer", start_timer },
	{ "timer_stop",  "Stop tcs timer", stop_timer },
	{ "mv", "View physical memory layout", memory_view },
	{ "pc", "Print constants", print_constants }
};
#define NCOMMANDS (sizeof(commands)/sizeof(commands[0]))

/***** Implementations of basic kernel monitor commands *****/

int
print_constants(int argc, char **argv, struct Trapframe *tf)
{
	cprintf("\n"); 
	return 0;
}

void
print_interval(int start, int current, bool allocated)
{
	if (start == (current -1)) {
		cprintf("%d %s\n", current, allocated ? "ALLOCATED": "FREE");
	} else {
		cprintf("%d..%d %s\n", start, current, allocated ? "ALLOCATED": "FREE");
	}
}

int 
memory_view(int argc, char **argv, struct Trapframe *tf)
{
	size_t i;
	uint32_t interval_start = 0;
	bool allocated = 1; // first is always allocated

	for (i = 0; i < npages; ++i) {
		if ((pages[i].pp_ref > 0) ^ allocated) {
			print_interval(interval_start, i, allocated);
			interval_start = i + 1;
			allocated = !allocated;
		}
	}
	print_interval(interval_start, i, allocated);
	return 0;
}

int
start_timer(int argc, char **argv, struct Trapframe *tf)
{
	timer_start();
	return 0;
}


int
stop_timer(int argc, char **argv, struct Trapframe *tf)
{
	timer_stop();
	return 0;
}

int
mon_debug(int argc, char **argv, struct Trapframe  *tf)
{
	cprintf("My funct works! %x %o\n", 10, 10);
	return  0;
}

int
mon_help(int argc, char **argv, struct Trapframe *tf)
{
	int i;

	for (i = 0; i < NCOMMANDS; i++)
		cprintf("%s - %s\n", commands[i].name, commands[i].desc);
	return 0;
}

int
mon_kerninfo(int argc, char **argv, struct Trapframe *tf)
{
	extern char _start[], entry[], etext[], edata[], end[];

	cprintf("Special kernel symbols:\n");
	cprintf("  _start                  %08x (phys)\n", (uint32_t)_start);
	cprintf("  entry  %08x (virt)  %08x (phys)\n",
            (uint32_t)entry, (uint32_t)entry - KERNBASE);
	cprintf("  etext  %08x (virt)  %08x (phys)\n",
            (uint32_t)etext, (uint32_t)etext - KERNBASE);
	cprintf("  edata  %08x (virt)  %08x (phys)\n",
            (uint32_t)edata, (uint32_t)edata - KERNBASE);
	cprintf("  end    %08x (virt)  %08x (phys)\n",
            (uint32_t)end, (uint32_t)end - KERNBASE);
	cprintf("Kernel executable memory footprint: %dKB\n",
		ROUNDUP(end - entry, 1024) / 1024);
	return 0;
}

int
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
	typedef union {
		uintptr_t* addr;
		uint32_t value;
	} register_data;

	struct Eipdebuginfo eip_info;

	register_data current_frame = (register_data) read_ebp();
	cprintf("%s\n", "Stack backtrace:");
	while(current_frame.value) {
		cprintf(
			"  ebp %08x eip %08x args ",
			current_frame.value,
			current_frame.addr[1]
		);
		for (uint8_t i = 0; i < 5; i++) {
			cprintf("%08x ", current_frame.addr[i + 2]);
		}
		cprintf("%s", "\n");
		debuginfo_eip(current_frame.addr[1], &eip_info);
		cprintf(
			"\t%s:%d: %.*s+%d\n",
			eip_info.eip_file,
			eip_info.eip_line,
			eip_info.eip_fn_namelen,
			eip_info.eip_fn_name,
			current_frame.addr[1] - eip_info.eip_fn_addr
		);
		current_frame = (register_data) *current_frame.addr;
	}
	return 0;
}



/***** Kernel monitor command interpreter *****/

#define WHITESPACE "\t\r\n "
#define MAXARGS 16

static int
runcmd(char *buf, struct Trapframe *tf)
{
	int argc;
	char *argv[MAXARGS];
	int i;

	// Parse the command buffer into whitespace-separated arguments
	argc = 0;
	argv[argc] = 0;
	while (1) {
		// gobble whitespace
		while (*buf && strchr(WHITESPACE, *buf))
			*buf++ = 0;
		if (*buf == 0)
			break;

		// save and scan past next arg
		if (argc == MAXARGS-1) {
			cprintf("Too many arguments (max %d)\n", MAXARGS);
			return 0;
		}
		argv[argc++] = buf;
		while (*buf && !strchr(WHITESPACE, *buf))
			buf++;
	}
	argv[argc] = 0;

	// Lookup and invoke the command
	if (argc == 0)
		return 0;
	for (i = 0; i < NCOMMANDS; i++) {
		if (strcmp(argv[0], commands[i].name) == 0)
			return commands[i].func(argc, argv, tf);
	}
	cprintf("Unknown command '%s'\n", argv[0]);
	return 0;
}

void
monitor(struct Trapframe *tf)
{
	char *buf;

	cprintf("Welcome to the JOS kernel monitor!\n");
	cprintf("Type 'help' for a list of commands.\n");

	if (tf != NULL)
		print_trapframe(tf);

	while (1) {
		buf = readline("K> ");
		if (buf != NULL)
			if (runcmd(buf, tf) < 0)
				break;
	}
}
