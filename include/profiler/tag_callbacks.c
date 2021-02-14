
#include "lbr-state.h"
#include <linux/sched.h>
#include "tag_callbacks.h"

unsigned int vuprofiler_initialized = 1;
EXPORT_SYMBOL(vuprofiler_initialized);

unsigned long long vuprof_getTag()
{
   unsigned long long tag;

   if (vuprofiler_initialized == 1)
   {
     return 0;
   }
   tag = current->vuprof_tag;
   // Zero out the tag so we can filter out untagged callsites
   current->vuprof_tag = 0;
   return tag;
}

EXPORT_SYMBOL(vuprof_getTag);

void vuprof_setTag(unsigned long long tagId)
{
   if (vuprofiler_initialized == 1)
   {
     return;
   }

   current->vuprof_tag = tagId;
}

EXPORT_SYMBOL(vuprof_setTag);


struct func_entry* lbr_entry_table[NUM_UNITS] = {NULL};
EXPORT_SYMBOL(lbr_entry_table);


/* We use the following variables to check if the sizes chosen for our structure 
   are smaller than required*/
unsigned int callOverflow = 0; 
unsigned int funcOverflow = 0;

EXPORT_SYMBOL(callOverflow);
EXPORT_SYMBOL(funcOverflow);

unsigned int vuprof_maxfuncs = 10000;
unsigned int vuprof_maxcalls = 5000;

EXPORT_SYMBOL(vuprof_maxfuncs);
EXPORT_SYMBOL(vuprof_maxcalls);


void vuprof_epilogue(unsigned long long tag)
{ 

   unsigned long long tos, cs_address, func_address;
   unsigned int i, j, cpu;
   struct func_entry *f_entry;

   if (vuprofiler_initialized == 1)
   {
     return;
   }

   /* Check if we have a nested call from within vuprof_epilogue (happens with preemption enabled)*/

   if (current->vuprof_cnt != 0)
       return;

   /* Increment counter (This should always be one level of indirection)*/
   current->vuprof_cnt++;
   
   preempt_disable(); 
   rdmsrl(MSR_LBR_TOS,  tos);

   // Get one entry above the TOS as the last entry 
   // will be resulted from the call to vuprof_epilogue
   // which we do not inline
   tos = (tos - 1) % LBR_ENTRIES;

   rdmsrl(MSR_LBR_NHM_FROM + tos,  cs_address);
   rdmsrl(MSR_LBR_NHM_TO + tos, func_address);
   cs_address = LBR_FROM(cs_address);
   preempt_enable();



   /* Check for pollution and return in case of pollution */
   if ((!cs_address)||(!func_address)||(!tag)){
       current->vuprof_cnt--;
       return;
   }

   cpu = get_cpu();

   f_entry = lbr_entry_table[cpu];

   // Check if we called module to initialize the profiling data structure
   if (f_entry == NULL){
       put_cpu();
       current->vuprof_cnt--;
       return;
   }

   for (i = 0; i < vuprof_maxfuncs; i++){
      if(f_entry[i].func_address == 0)
      {
           f_entry[i].calls[0].cs_address = cs_address;
           f_entry[i].calls[0].nhits = 1;
           f_entry[i].calls[0].tag = tag;
           f_entry[i].calls_length = 1;
           f_entry[i].func_address = func_address;
           f_entry[i].nhits = 1;
           break;
     }
     if (f_entry[i].func_address == func_address){

          f_entry[i].nhits++;

          for (j = 0; j < f_entry[i].calls_length; j++){
              if (f_entry[i].calls[j].cs_address == cs_address){
                 f_entry[i].calls[j].nhits++;
                 f_entry[i].calls[j].tag = tag;
                 break;
              }
          }

          if ((j ==  f_entry[i].calls_length) && (f_entry[i].calls_length < vuprof_maxcalls)){
             f_entry[i].calls_length++;
             f_entry[i].calls[j].cs_address = cs_address;
             f_entry[i].calls[j].tag = tag;
             f_entry[i].calls[j].nhits = 1;    
          }
          else if (f_entry[i].calls_length == vuprof_maxcalls){
             callOverflow++;
          }
           
          break;
     }
   }
   if (i == vuprof_maxfuncs)
   {
      funcOverflow++;
   }
   put_cpu(); 
   current->vuprof_cnt--;
}

EXPORT_SYMBOL(vuprof_epilogue);

