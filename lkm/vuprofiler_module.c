#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/miscdevice.h>
#include <linux/spinlock.h>
#include <linux/spinlock_types.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/compat.h>
#include <linux/init.h>
#include <linux/kprobes.h>
#include <linux/mman.h>
#include <linux/mm_types.h>
#include <linux/workqueue.h>
#include <linux/sysctl.h>
#include <linux/preempt.h>
#include <linux/delay.h> 
#include <linux/string.h>

#include "vuprofiler_module.h" // includes tag_callbacks.h
#include "lbr-state.h"

#ifdef INCLUDE_LBR_CONTEXT_SWITCHING
#include "lbr_context_switching.h"
#endif

MODULE_AUTHOR("Duta Victor Marin");
MODULE_DESCRIPTION("VUProfiler Module");
MODULE_LICENSE("GPL");


/****************************************************************************
 * EXPORTED FROM TAGLIB                                                  
 */
extern unsigned int vuprofiler_initialized;
extern struct func_entry* lbr_entry_table[NUM_UNITS];
extern unsigned int vuprof_maxfuncs;
extern unsigned int vuprof_maxcalls;
extern unsigned int callOverflow; 
extern unsigned int funcOverflow;

/****************************************************************************
 * MODULE FUNCTIONS AND VARS
 */

static int vuprofiler_users = 0;
static int module_initialized = 0;
static struct profiling_data vuprofiler_data;
static spinlock_t lbr_print_lock;


/******************************************************************************
 * Hooks on context switches.
 */

#ifdef INCLUDE_LBR_CONTEXT_SWITCHING

void init_lbr_context_switching(void *info)
{
     int cpu = get_cpu();
     sched_out_table[cpu] = save_lbr;
     sched_in_table[cpu] = restore_lbr;
     put_cpu();
}

void destroy_lbr_context_switching(void *info)
{
     int cpu = get_cpu();
     sched_out_table[cpu] = NULL;
     sched_in_table[cpu] = NULL;
     put_cpu();
}

#endif

extern spinlock_t lbr_cache_lock;
extern struct lbr_t lbr_cache[LBR_CACHE_SIZE];


static void init_overflow_variables()
{
  callOverflow = 0;
  funcOverflow = 0;
}

static void print_overflow_variables()
{
  printk(KERN_INFO"Func_Entry overflow:%d\n", funcOverflow);
  printk(KERN_INFO"CS_Entry overflow:%d\n", callOverflow);
  printk(KERN_INFO"Number of profiled functions:%ld\n", vuprof_maxfuncs);
  printk(KERN_INFO"Number of profiled callsites:%ld\n", vuprof_maxcalls);
}

static void init_merge_struct()
{
   unsigned int i;
   unsigned int num_func_entries, num_call_entries;
   struct func_entry *entries = vuprofiler_data.entries;


   num_func_entries = (unsigned int)(FUNC_SCALING * vuprof_maxfuncs);
   num_call_entries = (unsigned int)(CALL_SCALING * vuprof_maxcalls);

   if (!entries){
      printk(KERN_INFO"init_merge_struct:nothing to initialize\n");
      return;
   }

   for (i = 0; i < num_func_entries; i++){
     if(entries[i].calls){ 
        memset(entries[i].calls, 0, num_call_entries*sizeof(struct cs_entry));
     }
     entries[i].calls_length = 0;
     entries[i].func_address = 0;
     entries[i].nhits = 0;
   }
}

static void destroy_merge_struct()
{
   unsigned int i;
   unsigned int num_func_entries, num_call_entries;
   struct func_entry *entries = vuprofiler_data.entries;

   num_func_entries = (unsigned int)(FUNC_SCALING * vuprof_maxfuncs);
   num_call_entries = (unsigned int)(CALL_SCALING * vuprof_maxcalls);

   if (!entries)
      return;

   for (i = 0; i < num_func_entries; i++){
       if (entries[i].calls)
           kfree(entries[i].calls);
   }
   
   kfree(entries);
   vuprofiler_data.entries = NULL;
}

static int create_merge_struct()
{
  unsigned int num_func_entries, num_call_entries;
  unsigned int i;
  struct func_entry *entries;
  num_func_entries = (unsigned int)(FUNC_SCALING * vuprof_maxfuncs);
  num_call_entries = (unsigned int)(CALL_SCALING * vuprof_maxcalls);
  
  entries = kmalloc(num_func_entries*sizeof(struct func_entry), GFP_KERNEL);

  if (!entries){
     printk(KERN_ERR "create_merge_struct:Could not allocate storage for %ld functions\n", num_func_entries);
     vuprofiler_data.entries = NULL;
     return -ENOMEM;
  }
  vuprofiler_data.entries = entries;
  memset(entries, 0, num_func_entries*sizeof(struct func_entry));

  for (i = 0; i < num_func_entries; i++){
     entries[i].calls = kmalloc(num_call_entries*sizeof(struct cs_entry), GFP_KERNEL);
     if(!entries[i].calls){
        printk(KERN_ERR "create_merge_struct:Could not allocate storage for %ld calls for entry %ld\n", num_call_entries, i);
        return -ENOMEM;
     }
     entries[i].calls_length = 0;
     entries[i].func_address = 0;
     entries[i].nhits = 0;

     memset(entries[i].calls, 0, num_call_entries*sizeof(struct cs_entry));
  }

  spin_lock_init(&(vuprofiler_data.lock));

  return 0;
  
}

static void init_function_entries(void *info)
{
  int cpu;
  unsigned int i;
  struct func_entry *func_entries;
  cpu = get_cpu();
  func_entries = lbr_entry_table[cpu];

  if (!func_entries){
     printk(KERN_INFO"init_function_entries:nothing to initialize\n");
     return;
  }

  for (i = 0; i < vuprof_maxfuncs; i++){
     if(func_entries[i].calls){ 
        memset(func_entries[i].calls, 0, vuprof_maxcalls*sizeof(struct cs_entry));
     }
     func_entries[i].calls_length = 0;
     func_entries[i].func_address = 0;
     func_entries[i].nhits = 0;
  }
  
  put_cpu();
}

static void destroy_function_entries(void *info)
{
  int cpu;
  unsigned int i;

  struct func_entry *func_entries;
  cpu = get_cpu();
  func_entries = lbr_entry_table[cpu];
  if (!func_entries)
     return;
  for (i = 0; i < vuprof_maxfuncs; i++){
     if(func_entries[i].calls){ 
         kfree(func_entries[i].calls);
     }
  }
  kfree(func_entries);
  // Mark the structure as destroyed for all running threads
  // that were past the vuprofiler_initialized check prior
  // to the structure being destroyed
  lbr_entry_table[cpu] = NULL; 
  put_cpu();
}


static void create_function_entries(void *info)
{
  int cpu;
  unsigned int i;
  int *err = (int *)(info);
  struct func_entry *func_entries;
  cpu = get_cpu();
  lbr_entry_table[cpu] = kmalloc(vuprof_maxfuncs*sizeof(struct func_entry), GFP_KERNEL);

  if (!lbr_entry_table[cpu]){
     printk(KERN_ERR "Could not allocate storage for %d functions on CPU %d\n", vuprof_maxfuncs, cpu);
     *err = -ENOMEM;
     return;
  }

  memset(lbr_entry_table[cpu], 0, vuprof_maxfuncs*sizeof(struct func_entry));
  func_entries = lbr_entry_table[cpu];

  for (i = 0; i < vuprof_maxfuncs; i++){
     func_entries[i].calls = kmalloc(vuprof_maxcalls*sizeof(struct cs_entry), GFP_KERNEL);
     if(!func_entries[i].calls){
        printk(KERN_ERR "Could not allocate storage for %d calls on CPU %d entry %d\n", vuprof_maxcalls, cpu, i);
        *err = -ENOMEM;
        return;
     }
     func_entries[i].calls_length = 0;
     func_entries[i].func_address = 0;
     func_entries[i].nhits = 0;

     memset(func_entries[i].calls, 0, vuprof_maxcalls*sizeof(struct cs_entry));
  }
       
  put_cpu();
}

/* Create a profiling structure and the merge structure able to store max_funcs on each core 
   each able to store max_calls callsites for each function */

static int create_profiling_struct(unsigned int max_funcs, unsigned int max_calls)
{
  int wait = 1;
  int err = 0;

  /* Initialize overflow variables used to verify if we allocated 
     an sufficiently large structure to store our results 
   */
  init_overflow_variables();

  vuprof_maxfuncs = max_funcs;
  vuprof_maxcalls = max_calls;

  on_each_cpu(create_function_entries, &err, wait);

  if (err){
     destroy_profiling_struct();
     return err;
  }

  if ((err = create_merge_struct()) != 0){
    destroy_profiling_struct();
    destroy_merge_struct();
    return err;
  }
 
  vuprofiler_initialized = 0;
  
  return 0;
}

static void destroy_profiling_struct()
{
   int wait = 1;

   vuprofiler_initialized = 1;

   /* Sleep for a small ammount such that all threads finish
      their access to the profiling
      structure */    
   //usleep_range(100000, 100001);


   on_each_cpu(destroy_function_entries, NULL, wait);
}

void init_profiling_struct()
{
   int wait = 1; 
   on_each_cpu(init_function_entries, NULL, wait);
}


void print_lbr_registers(void *info)
{
    struct lbr_t lbr;
    unsigned long flags;
    spinlock_t *lock=(spinlock_t *)(info);

    spin_lock_irqsave(lock, flags);

    get_cpu();
    get_lbr(&lbr);
    dump_lbr(&lbr);
    put_cpu();

    spin_unlock_irqrestore(lock, flags);   
}

void get_profiling_struct(void *info)
{
   unsigned long flags;
   unsigned int i, j, k, t;
   unsigned int num_func_entries, num_call_entries;
   int cpu;
   struct func_entry *entries;
   struct profiling_data *vuprof_data=(struct profiling_data *)(info); 
  
   num_func_entries = (unsigned int)(FUNC_SCALING * vuprof_maxfuncs);
   num_call_entries = (unsigned int)(CALL_SCALING * vuprof_maxcalls);  
 
   cpu = get_cpu();
   entries = lbr_entry_table[cpu];
   spin_lock_irqsave(&(vuprof_data->lock), flags);

   for(i = 0; i < vuprof_maxfuncs; i++)
   {
      if (entries[i].func_address == 0)
      {
         break;
      }
      for(j = 0; j < num_func_entries; j++)
      {
           if(vuprof_data->entries[j].func_address == entries[i].func_address)
           {
                for( k = 0; k < entries[i].calls_length; k++)
                {
                   for(t = 0; t < vuprof_data->entries[j].calls_length; t++)
                   {
                       if (entries[i].calls[k].cs_address == vuprof_data->entries[j].calls[t].cs_address)
                       {
                           vuprof_data->entries[j].calls[t].nhits += entries[i].calls[k].nhits;
                           break;
                       }
                   } 
 
                   if ( (t ==  vuprof_data->entries[j].calls_length) && (vuprof_data->entries[j].calls_length < num_call_entries))
                   {
                        vuprof_data->entries[j].calls_length++;
                        vuprof_data->entries[j].calls[t].nhits = entries[i].calls[k].nhits;
                        vuprof_data->entries[j].calls[t].cs_address =  entries[i].calls[k].cs_address;
                        vuprof_data->entries[j].calls[t].tag =  entries[i].calls[k].tag;
                   }
                }
                vuprof_data->entries[j].nhits += entries[i].nhits;
                break;
           }
           if(vuprof_data->entries[j].func_address == 0)
           {
              //vuprof_data->entries[j].tos =  entries[i].tos;
              //vuprof_data->entries[j].address = entries[i].address;
              vuprof_data->entries[j].func_address =   entries[i].func_address;
              vuprof_data->entries[j].nhits = entries[i].nhits;
              vuprof_data->entries[j].calls_length = entries[i].calls_length;
              memcpy(vuprof_data->entries[j].calls, entries[i].calls,  (entries[i].calls_length)*sizeof(struct cs_entry));
              break;
           }
      }
   }
   spin_unlock_irqrestore(&(vuprof_data->lock), flags);
   put_cpu();
}

/*****************************************************************************
 * MODULE INTERFACE
 */

int vuprofiler_open(struct inode *inode, struct file *filp) {
    vuprofiler_users++;

    if(module_initialized) 
                 return 0;
    
    module_initialized = 1;
    return 0;
}

int vuprofiler_close(struct inode *inode, struct file *filp) {
    vuprofiler_users--;

    if(vuprofiler_users) 
           return 0;

    module_initialized = 0;
    return 0;
}


static void handle_ioc_print_data()
{
   unsigned int num_func_entries, num_call_entries;
   unsigned int i,j;
   int wait = 1;
   num_func_entries = (unsigned int)(FUNC_SCALING * vuprof_maxfuncs);
   num_call_entries = (unsigned int)(CALL_SCALING * vuprof_maxcalls);
   init_merge_struct();

   on_each_cpu(get_profiling_struct, &vuprofiler_data, wait);

   for (i = 0; i < num_func_entries; i++){
       if (vuprofiler_data.entries[i].func_address == 0)
          break;
       for(j = 0; j < vuprofiler_data.entries[i].calls_length; j++){
          printk("%llx %lld %llx %lld %lld\n", vuprofiler_data.entries[i].func_address,
                                               vuprofiler_data.entries[i].nhits,
                                               vuprofiler_data.entries[i].calls[j].cs_address,
                                               vuprofiler_data.entries[i].calls[j].nhits,
                                               vuprofiler_data.entries[i].calls[j].tag );
       }
   } 
}

static unsigned long long handle_ioc_get_data(void __user *user_pointer)
{
   static int initialized = 0;
   static unsigned int n_sent;
   static int send_funcs = 0, send_calls = 0;
   static int entry_index;
   unsigned int to_send = 0;
   int wait = 1;
   unsigned int num_func_entries, num_call_entries;
   struct cs_entry *cs_entries;
   struct func_entry *entries = vuprofiler_data.entries;


   num_func_entries = (unsigned int)(FUNC_SCALING * vuprof_maxfuncs);
   num_call_entries = (unsigned int)(CALL_SCALING * vuprof_maxcalls);

   if (!initialized){
      printk(KERN_INFO"handle_ioc_get_data:zeroing merge function\n");
      init_merge_struct();
      on_each_cpu(get_profiling_struct, &vuprofiler_data, wait);
      initialized = 1;
      send_funcs = 1;
      n_sent = 0;
   } 

   if (send_funcs){
       if (n_sent + CHUNK_SIZE > num_func_entries){
           to_send = num_func_entries - n_sent;
       } 
       else {
           to_send = CHUNK_SIZE;
       }
       
       if (copy_to_user(user_pointer, &entries[n_sent], to_send*sizeof(struct func_entry)) != 0) {
            printk(KERN_ERR "handle_ioc_get_data: Error while copying function entries to userland\n");
            initialized = 0;
            send_funcs = 0;
            return -EAGAIN;
       }

       n_sent += to_send;

       if (n_sent >= num_func_entries){
          send_funcs = 0;
          n_sent = 0;
          send_calls = 1;
          entry_index = 0;
       }

       return to_send;
   }

   if (send_calls){
      if (entry_index == num_func_entries){
         send_calls = 0;
         initialized = 0;
         n_sent = 0;
         return 0;
      }

      cs_entries = entries[entry_index].calls;

      if (n_sent + CHUNK_SIZE > num_call_entries){
         to_send = num_call_entries - n_sent;
      } 
      else {
         to_send = CHUNK_SIZE;
      }

      if (copy_to_user(user_pointer, &cs_entries[n_sent], to_send*sizeof(struct cs_entry)) != 0) {
            printk(KERN_ERR "handle_ioc_get_data: Error while copying function entries to userland\n");
            initialized = 0;
            send_calls = 0;
            return -EAGAIN;
      }

      n_sent += to_send;

      if (n_sent >= num_call_entries){
          n_sent = 0;
          entry_index++;
      }
   }

   return to_send;
}

static long vuprofiler_ioctl(struct file *file, unsigned int cmd, unsigned long arg1) {
    int wait = 1;
    switch(cmd)
    {
         case PROFILER_IOC_INIT_STRUCT:
			   init_profiling_struct();
                           break;
         case PROFILER_IOC_GET_OVERFLOW:
			   print_overflow_variables();
                           break;
         case PROFILER_IOC_PRINT_LBR:
                           on_each_cpu(print_lbr_registers, &lbr_print_lock, wait);
			   printk(KERN_INFO "Printed LBR registers on all cpus\n");
                           break;
         case PROFILER_IOC_GET_DATA:
                           return handle_ioc_get_data((void __user *) arg1);
         case PROFILER_IOC_PRINT_DATA:
                           handle_ioc_print_data();
                           break;
         
         
    }
    return 0;

}


/******************************************************************************
 * MODULE SETUP                                                               
 */

static struct file_operations vuprofiler_fops = {
    .owner          = THIS_MODULE,
    .unlocked_ioctl = vuprofiler_ioctl,
    .open           = vuprofiler_open,
    .release        = vuprofiler_close,
};

static struct miscdevice vuprofiler_miscdev = {
    .minor          = MISC_DYNAMIC_MINOR,
    .name           = "vuprofiler_dev",
    .fops           = &vuprofiler_fops,
}; 



static int __init vuprofiler_module_init(void) {
    int wait = 1;

    spin_lock_init(&lbr_print_lock);

#ifdef INCLUDE_LBR_CONTEXT_SWITCHING
#warning Context Switching is enabled
    init_lbr_cache();
    on_each_cpu(init_lbr_context_switching, NULL, wait);
#endif
    printk(KERN_INFO"Vuprofiler vuprofiler_initialized is %ld\n",vuprofiler_initialized);
    
    if (misc_register(&vuprofiler_miscdev))
    {
       printk(KERN_ERR "cannot register miscdev on minor=%d\n", SHADOW_MINOR);
       return -1;
    }
   
    printk(KERN_INFO"registered miscdev on minor=%d\n", vuprofiler_miscdev.minor);

    /* Enable lbr on each cpu */
    on_each_cpu(enable_lbr, NULL, wait);

    printk(KERN_INFO"initialized lbr on each cpu\n");


    if (create_profiling_struct(MAX_FUNCS, MAX_CALLS)){
       return -1;
    }

    printk(KERN_INFO"Vuprofiler module initialized\n");

    return 0;
}
static void __exit vuprofiler_module_exit(void) {
    int wait = 1;

    destroy_profiling_struct();

    destroy_merge_struct();

    /* Disable lbr on each cpu */
    on_each_cpu(disable_lbr, NULL, wait);

#ifdef INCLUDE_LBR_CONTEXT_SWITCHING
    on_each_cpu(destroy_lbr_context_switching, NULL, wait);
#endif

    misc_deregister(&vuprofiler_miscdev);

   
    printk(KERN_INFO"Vuprofiler module disabled\n");
}

module_init(vuprofiler_module_init);
module_exit(vuprofiler_module_exit);

