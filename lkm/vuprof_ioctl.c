#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>


#include "vuprofiler_module.h" // includes tag_callbacks.h

#define INIT_DATA "init_data"
#define CHECK_OVERFLOW "check_overflow"
#define PRINT_LBR  "print_lbr"
#define GET_DATA   "get_data"
#define PRINT_DATA "print_data"
struct func_entry *data = NULL;

/* Allocate and zero-initialize the data structure in which we copy the 
   profiling structure from kernel space. 
   Do this in two stages as the ioctl will overwrite the cs_entry* array
   references.
   Phase 1: create_func_entries creates and initializes the function entries
   Phase 2: create_cs_entries creates and initializes the call-site entries
 */
static void create_func_entries(){
  unsigned int num_func_entries;

  num_func_entries = (unsigned int)(FUNC_SCALING * MAX_FUNCS);

  data = (func_entry *)malloc(num_func_entries*sizeof(struct func_entry));
  
  if(!data){
     printf("init_profiling_data:func_entry struct allocation failed\n");
     exit(1);
  }
  
  memset(data, 0, num_func_entries*sizeof(struct func_entry));
}

static void create_cs_entries(){
  unsigned int num_func_entries, num_call_entries;
  unsigned int i;

  num_func_entries = (unsigned int)(FUNC_SCALING * MAX_FUNCS);
  num_call_entries = (unsigned int)(CALL_SCALING * MAX_CALLS);

  for (i = 0; i < num_func_entries; i++){
     data[i].calls = (cs_entry *) malloc(num_call_entries*sizeof(struct cs_entry));

     if (!data[i].calls){
        printf("init_profiling_data:cs_entry struct allocation failed\n");
        exit(1);
     }

     memset(data[i].calls, 0, num_call_entries*sizeof(struct cs_entry));
  }
}

static void handle_get_data(int fd){
  unsigned int num_func_entries, num_call_entries;
  unsigned int chunk, i;
  unsigned int offset;
  struct cs_entry *cs_data;

  num_func_entries = (unsigned int)(FUNC_SCALING * MAX_FUNCS);
  num_call_entries = (unsigned int)(CALL_SCALING * MAX_CALLS);
  
  // Initialize the func_entry array
  create_func_entries();

  offset = 0;
  /* Copy the function entries from kernel space.
     This will overwrite all data[i].calls pointers which
     we will reinitialize via a call to create_cs_entries.
  */
  while(offset < num_func_entries){
     chunk = ioctl(fd, PROFILER_IOC_GET_DATA, &data[offset]);
     offset += chunk;
  }

  // We can now initialize the call arrays
  create_cs_entries();
  
  // Copy call arrays from kernel space
  for (i = 0; i < num_func_entries; i++){
     offset = 0;
     cs_data = data[i].calls;
     while(offset < num_call_entries){
        chunk = ioctl(fd, PROFILER_IOC_GET_DATA, &cs_data[offset]);
        offset += chunk;
     }
  } 

  // Revert the kernel data structure to initialized == 0 
  chunk = ioctl(fd, PROFILER_IOC_GET_DATA, NULL);

  /* This is an extra check to verify that our ioctl interface
     and userspace client are synchronized 
   */
  if (chunk != 0){
     printf("handle_get_data: Issue with ioctl interface\n");
  }
  else{
     //printf("handle_get_data: Finished OK!\n");
  }

  
}
static void print_entries(){
  unsigned int num_func_entries, num_call_entries;
  unsigned int i, j;

  num_func_entries = (unsigned int)(FUNC_SCALING * MAX_FUNCS);
  num_call_entries = (unsigned int)(CALL_SCALING * MAX_CALLS);

  for(i = 0; i < num_func_entries; i++){
    if(data[i].func_address == 0){
       break;
    }

    for(j = 0; j < data[i].calls_length; j++){
        printf("%llx %lld %llx %lld %lld\n", data[i].func_address, data[i].nhits, 
                                             data[i].calls[j].cs_address, 
                                             data[i].calls[j].nhits, 
                                             data[i].calls[j].tag);
    }
  }
}

static void interpret_cmd(int fd, char *cmd){
  
  if(!strcmp(cmd, INIT_DATA)){
     printf("Initializing profiling database...\n");
     ioctl(fd, PROFILER_IOC_INIT_STRUCT);
     return;
  }

  if(!strcmp(cmd, CHECK_OVERFLOW)){
     printf("Check overflow (dmesg to see results)...\n");
     ioctl(fd, PROFILER_IOC_GET_OVERFLOW);
     return;
  }

  if(!strcmp(cmd, PRINT_LBR)){
     printf("Print LBR registers (dmesg to see results)...\n");
     ioctl(fd, PROFILER_IOC_PRINT_LBR);
     return;
  }

  if(!strcmp(cmd, PRINT_DATA)){
     printf("Print profiling structure (dmesg to see results)...\n");
     ioctl(fd, PROFILER_IOC_PRINT_DATA);
     return;
  }
   
  if(!strcmp(cmd, GET_DATA)){
      // Get profiling data from kernel space
      handle_get_data(fd);
      // Print profiling data
      print_entries();
      return;
  }


}

int main(int argc,  char *argv[]){
   int profiler_fd;
   int i;
   
   profiler_fd = open("/dev/vuprofiler_dev", O_RDWR);
 
   if(profiler_fd < 0)
   {
       printf("Error opening profiler interface\n");
       return 1;
   }
   for(i = 1; i < argc; i++)
   {
      interpret_cmd(profiler_fd, argv[i]);
   } 

   close(profiler_fd);

   return 0;
}
