ROOT=../..

include $(ROOT)/Makefile.inc

CFLAGS  += $(CXXFLAGS) -Wno-long-long -Wall -W -Wno-unused-parameter -Wwrite-strings -DHAVE_EXCEPTIONS=0
CFLAGS  += $(shell $(LLVMPREFIX)/bin/llvm-config --cxxflags) -DLLVM_VERSION=$(LLVM_VERSION) 


llvm_utils.o: llvm_utils.cpp llvm_utils.h
	$(CXX) $(CFLAGS) $(IFLAGS) $(DFLAGS) -c -o $@ $< 

libLLVMUtils.a: llvm_utils.o
	ar crf $@ $< 

install: $(INSTALL_TOP_DIR)/libLLVMUtils.a

$(INSTALL_TOP_DIR)/libLLVMUtils.a: libLLVMUtils.a
	 install -c -D -m 744 $? $@

clean:
	rm -f libLLVMUtils.a llvm_utils.o $(INSTALL_TOP_DIR)/libLLVMUtils.a
	


