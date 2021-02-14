#ifndef LBR_STATE_H
#define LBR_STATE_H

#ifndef __KERNEL__

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif 

#ifndef __USE_GNU
#define __USE_GNU
#endif

#include <stdint.h>
#include <stdio.h>

#else /* __KERNEL__ */
#include <linux/kernel.h>
#include <linux/sched.h>

#endif

/* This one is also defined in lbr.h, but lbr.h is not included (as it is only
 * kernel stuff there). 
 */
#ifndef LBR_ENTRIES
#define LBR_ENTRIES 16
#else
#warning Supplied LBR_ENTRIES from Makefile.inc
#endif


#include "lbr.h" //contains all necesarry defines for lbr register manipulation

struct lbr_t {
    uint64_t debug;   // contents of IA32_DEBUGCTL MSR
    uint64_t select;  // contents of LBR_SELECT
    uint64_t tos;     // index to most recent branch entry
    uint64_t from[LBR_ENTRIES];
    uint64_t   to[LBR_ENTRIES];
    struct task_struct *task; // pointer to the task_struct this state belongs to
};

/***** Function prototipes *****/

/* Enable the LBR feature for the current CPU */
void enable_lbr(void* info);

void disable_lbr(void* info);

void flush_lbr(bool enable);

/* Read the contents of the LBR registers into a lbr_t structure */
void get_lbr(struct lbr_t *lbr);
/* Write the contents of a lbr_t structure into the LBR registers */
void put_lbr(struct lbr_t *lbr);

/* Dump the contents of a lbr_t structure to syslog */
void dump_lbr(struct lbr_t *lbr);

void clear_lbr(void);


inline void printd(bool print_cpuid, const char *fmt, ...);

/* Context Switching Hooks */
void save_lbr(void);
void restore_lbr(void);
void init_lbr_cache(void);



#endif /* __LBR_STATE__ */


