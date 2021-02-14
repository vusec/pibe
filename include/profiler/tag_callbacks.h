#ifndef __TAG_CALLBACKS_H__
#define __TAG_CALLBACKS_H__

#ifndef NUM_UNITS
#define NUM_UNITS 8
#else
#warning Supplied NUM_UNITS from Makefile.inc
#endif

typedef struct cs_entry{
  unsigned long long cs_address;
  unsigned long long nhits;
  unsigned long long tag;
} cs_entry;

typedef struct func_entry {
   struct cs_entry *calls;
   unsigned long long calls_length;
   unsigned long long func_address;
   unsigned long long nhits;
} func_entry;


unsigned long long vuprof_getTag(void);
void vuprof_setTag(unsigned long long);
void vuprof_epilogue(unsigned long long tag);


#endif // __TAG_CALLBACKS_H__
