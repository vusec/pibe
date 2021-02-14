
#include "lbr-state.h"


struct lbr_t lbr_cache[LBR_CACHE_SIZE];
spinlock_t lbr_cache_lock;

static unsigned int lru_cache=0;

#ifdef SHADOW_DEBUG
inline void printd(bool print_cpuid, const char *fmt, ...) {
    int core_id, thread_id;
    va_list args;

    //if (print_cpuid) cpuids(&core_id, &thread_id);

    va_start(args, fmt);
    //if (print_cpuid) printk("A:%d:%d: ", core_id, thread_id);
    vprintk(fmt, args);
    va_end(args);
}
#else
inline void printd(bool print_cpuid, const char *fmt, ...) {
}
#endif


/***********************************************************************
 * Helper functions for LBR related functionality.
 */

/* Flush the LBR registers. Caller should do get_cpu() and put_cpu().  */
void flush_lbr(bool enable) {
    int i;

    wrmsrl(MSR_LBR_TOS, 0);
    for (i = 0; i < LBR_ENTRIES; i++) {
        wrmsrl(MSR_LBR_NHM_FROM + i, 0);
        wrmsrl(MSR_LBR_NHM_TO   + i, 0);
    }
    if (enable) wrmsrl(MSR_IA32_DEBUGCTLMSR, IA32_DEBUGCTL);
    else        wrmsrl(MSR_IA32_DEBUGCTLMSR, 0);
}

/* Just clean the LBR in case we have a new task here */
void clear_lbr() {
    int i;
    wrmsrl(MSR_LBR_TOS, 0);
    for (i = 0; i < LBR_ENTRIES; i++) {
        wrmsrl(MSR_LBR_NHM_FROM + i, 0);
        wrmsrl(MSR_LBR_NHM_TO   + i, 0);
    }
}

/* Store the LBR registers for the current CPU into <lbr>. */
void get_lbr(struct lbr_t *lbr) {
    int i;

    rdmsrl(MSR_IA32_DEBUGCTLMSR, lbr->debug);
    rdmsrl(MSR_LBR_SELECT,       lbr->select);
    rdmsrl(MSR_LBR_TOS,          lbr->tos);
    for (i = 0; i < LBR_ENTRIES; i++) {
        rdmsrl(MSR_LBR_NHM_FROM + i, lbr->from[i]);
        rdmsrl(MSR_LBR_NHM_TO   + i, lbr->to  [i]);

        lbr->from[i] = LBR_FROM(lbr->from[i]);
    }
}

/* Write the LBR registers for the current CPU. */
void put_lbr(struct lbr_t *lbr) {
    int i;

    wrmsrl(MSR_IA32_DEBUGCTLMSR, lbr->debug);
    wrmsrl(MSR_LBR_SELECT,       lbr->select);
    wrmsrl(MSR_LBR_TOS,          lbr->tos);
    for (i = 0; i < LBR_ENTRIES; i++) {
        wrmsrl(MSR_LBR_NHM_FROM + i, lbr->from[i]);
        wrmsrl(MSR_LBR_NHM_TO   + i, lbr->to  [i]);
    }
}

/* Dump the LBR registers as stored in <lbr>. */
void dump_lbr(struct lbr_t *lbr) {
    int i;
    printd(true, "MSR_IA32_DEBUGCTLMSR: 0x%llx\n", lbr->debug);
    printd(true, "MSR_LBR_SELECT:       0x%llx\n", lbr->select);
    printd(true, "MSR_LBR_TOS:          %lld\n", lbr->tos);
    for (i = 0; i < LBR_ENTRIES; i++) {
      printd(true, "MSR_LBR_NHM_FROM[%2d]: 0x%llx\n", i, lbr->from[i]);
      printd(true, "MSR_LBR_NHM_TO  [%2d]: 0x%llx\n", i, lbr->to[i]);
    }
}

/* Enable the LBR feature for the current CPU. *info may be NULL (it is required
 * by on_each_cpu()).
 */
void enable_lbr(void *info) {

    get_cpu();

    printd(true, "Enabling LBR\n");

    /* Apply the filter (what kind of branches do we want to track?) */
    wrmsrl(MSR_LBR_SELECT, LBR_SELECT);
    
    /* Flush the LBR and enable it */
    flush_lbr(true);

    put_cpu();
}

/* Disable the LBR feature for the current CPU
 */

void disable_lbr(void *info) {

   get_cpu();

   printd(true, "Disabling LBR\n");

   flush_lbr(false);

   put_cpu();
   
}


/********* SAVE LBR */
void save_lbr(void) {
    int i;
    bool saved = false;
    unsigned long lbr_cache_flags;

    /* Multiple processes may be calling this function simultanously. We need to
     * lock a mutex so that no LBR stores are lost due to race conditions.
     */
    spin_lock_irqsave(&lbr_cache_lock, lbr_cache_flags);

    /* Loop over the LBR cache to find the first empty entry. */
    for (i = 0; i < LBR_CACHE_SIZE; i++) {
        if (lbr_cache[i].task == NULL) {
            
//          printd(true, "SAVE LBR - cache[%d] = %p (%s)\n", i, current, current->comm);

            /* Read the LBR registers into the LBR cache entry. */
            get_lbr(&lbr_cache[i]);

            /* Store our task_struct pointer in the LBR cache entry. */
            lbr_cache[i].task = current;

            saved = true;
            break;
        }
    }

    /* Crash if we were unable to save the LBR state. This could happen if too
     * many tasks (processes or threads) are running in parellel.
     */
    if (!saved) {
        printk("SAVE LBR - Purpose failed. LBR cache full?\n");
//      kill_pid(task_pid(current), SIGKILL, 1);
        get_lbr(&lbr_cache[lru_cache]);
        lbr_cache[lru_cache].task = current;
        lru_cache = (lru_cache+1) % LBR_CACHE_SIZE;  
        
    }

    /* Disable the LBR for other processes. We don't care about the original LBR
     * state for now. Other modules also using the LBR should do their own
     * bookkeeping.
     *
     * One may argue that the LBR should be flushed here in order to avoid
     * information leakage about the target's ASLR state. The LBR registers can
     * only be accessed from within RING 0 though, so this is not really an
     * issue. 
     *
     * As an optimization step, we could decide to leave LBR debugging on. This
     * would save us a couple of instructions, but will slow down the entire
     * system.
     */
    //wrmsrl(MSR_IA32_DEBUGCTLMSR, 0);

    /* Unlock the mutex. */
    spin_unlock_irqrestore(&lbr_cache_lock, lbr_cache_flags);
}

/********* RESTORE LBR */
void restore_lbr(void) {
    int i;
    bool found = false;
    unsigned long lbr_cache_flags;

    /* This mutex is here for safety only. I think it could be removed. */
    spin_lock_irqsave(&lbr_cache_lock, lbr_cache_flags);

    /* Loop over the LBR cache to find ourself. */
    for (i = 0; i < LBR_CACHE_SIZE; i++) {
        if (lbr_cache[i].task == current) {
//          printd(true, "LOAD LBR - cache[%d] = %p (%s)\n", i, current, current->comm);

            /* Write the LBR registers from the LBR cache entry. */
            put_lbr(&lbr_cache[i]);

            /* Clear the task_struct pointer so this entry can be reused again */
            lbr_cache[i].task = NULL;
            
            found = true;
            break;
        }
    }
    
    if (!found) {
        //printk("LOAD LBR - Purpose failed for task %p (%s)\n", current, current->comm);

        clear_lbr();
        /* This should never happen. All new tasks pass wake_up_new_task(), which
         * installs a LBR state into the cache.
         */
//      kill_pid(task_pid(current), SIGKILL, 1);
    }
    
    /* Unlock the mutex. */
    spin_unlock_irqrestore(&lbr_cache_lock, lbr_cache_flags);

}

/* Initialize the LBR cache. */
void init_lbr_cache(void) {
    memset(&lbr_cache, 0, sizeof(lbr_cache));
}



