#ifndef __LLVM_MACROS_H__
#define __LLVM_MACROS_H__

// warning_print, debug_print, error_print should receive either strings or elements that can be outputed as strings 

#if PASS_DEBUG_LEVEL == 3

#define warning_print(elem) outs() << "Warning: " << elem << "\n"
#define error_print(elem) outs() << "Error: " << elem << "\n"
#define debug_print(elem) outs() << "Debug: " << elem << "\n"
#define debug_print_element(elem)  outs() << "======Begin Element=====\n"; \
                                   elem->print(outs()); \
                                   outs() << "\n"; \
                                   outs() << "======End- -Element=====\n" 

#elif PASS_DEBUG_LEVEL == 2

#define warning_print(elem) outs() << "Warning: " << elem << "\n"
#define error_print(elem)  outs() << "Error: " << elem << "\n"
#define debug_print(elem)
#define debug_print_element(elem) 

#elif PASS_DEBUG_LEVEL == 1

#define warning_print(elem)
#define error_print(elem) outs() << "Error: " << elem << "\n"
#define debug_print(elem)
#define debug_print_element(elem) 

#else

#define warning_print(elem)
#define error_print(elem)
#define debug_print(elem)
#define debug_print_element(elem) 

#endif

#define progress_print(elem) outs() << elem << "\n"

#endif // __LLVM_MACROS_H__
