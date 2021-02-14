#ifndef __VUPROFILER_MODULE_H__
#define __VUPROFILER_MODULE_H__

#include "tag_callbacks.h"

#ifdef __KERNEL__

#include <linux/ioctl.h>
#include <linux/spinlock.h>
#include <linux/spinlock_types.h>


static void init_overflow_variables(void);
static void print_overflow_variables(void);
static void init_merge_struct(void);
static void destroy_merge_struct(void);
static int create_merge_struct(void);
static void init_function_entries(void *info);
static void destroy_function_entries(void *info);
static void create_function_entries(void *info);
static int create_profiling_struct(unsigned int max_funcs, unsigned int max_calls);
static void destroy_profiling_struct(void);
static void init_profiling_struct(void);
static void print_lbr_registers(void *info);
static void get_profiling_struct(void *info);
 

struct profiling_data {
   struct func_entry *entries;
   spinlock_t lock;
};


#else

#include <sys/ioctl.h>

#endif

#define CHUNK_SIZE 500
#define MAX_FUNCS 10000
#define MAX_CALLS 1000
#define FUNC_SCALING 1
#define CALL_SCALING 1

#define PROFILER_IOC_MAGIC 'l'
#define PROFILER_IOC_INIT_STRUCT          _IO(PROFILER_IOC_MAGIC,  1)
#define PROFILER_IOC_GET_OVERFLOW         _IO(PROFILER_IOC_MAGIC,  2)
#define PROFILER_IOC_GET_DATA		  _IO(PROFILER_IOC_MAGIC,  3)
#define PROFILER_IOC_PRINT_LBR            _IO(PROFILER_IOC_MAGIC,  4)
#define PROFILER_IOC_PRINT_DATA           _IO(PROFILER_IOC_MAGIC,  5)

#define SHADOW_MINOR 133

#endif /* __VUPROFILER_MODULE_H__ */
