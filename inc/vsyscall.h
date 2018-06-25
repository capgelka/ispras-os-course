#ifndef JOS_INC_VSYSCALL_H
#define JOS_INC_VSYSCALL_H

#include <inc/time.h>

/* system call numbers */
#define VSYS_gettime 0
#define VSYS_CLOCK_REALTIME (sizeof(struct timespec)/sizeof(int) + sizeof(struct timespec) % sizeof(int))
#define VSYS_CLOCK_MONOTONIC (VSYS_CLOCK_REALTIME + sizeof(struct timespec)/sizeof(int) + sizeof(struct timespec) % sizeof(int))
#define NVSYSCALLS (VSYS_CLOCK_MONOTONIC + 1)

#endif /* !JOS_INC_VSYSCALL_H */
