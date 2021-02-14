DIRS := binutils llvm_10

ROOT=..

include $(ROOT)/Makefile.inc

CONFIGFLAGS_llvm := -g ¨Unix Makefiles¨
CONFIGFLAGS_llvm += -DLLVM_ENABLE_EH=ON -DLLVM_ENABLE_RTTI=ON 
CONFIGFLAGS_llvm += -DCMAKE_INSTALL_PREFIX=${LLVM_SOURCE}/bin
CONFIGFLAGS_llvm += -DLLVM_TARGETS_TO_BUILD="X86"

CONFIGFLAGS_llvm_10 := -g ¨Unix Makefiles¨
CONFIGFLAGS_llvm_10 += -DLLVM_ENABLE_EH=ON -DLLVM_ENABLE_RTTI=ON 
CONFIGFLAGS_llvm_10 += -DCMAKE_INSTALL_PREFIX=${LLVM_SOURCE}/bin
CONFIGFLAGS_llvm_10 += -DLLVM_TARGETS_TO_BUILD="X86"
CONFIGFLAGS_llvm_10 += -DLLVM_ENABLE_PROJECTS='clang;clang-tools-extra;libcxx;libcxxabi;compiler-rt;lld'

ifeq ($(HAVE_GOLD),True)
CONFIGFLAGS_llvm += -DLLVM_USE_LINKER=gold 
CONFIGFLAGS_llvm += -DCMAKE_LINKER=${BINUTILS_SOURCE}/bin/bin/ld.gold

CONFIGFLAGS_llvm_10 += -DLLVM_USE_LINKER=gold -DGOLD_EXECUTABLE:FILEPATH=${BINUTILS_SOURCE}/bin/bin/ld.gold -DCMAKE_BUILD_TYPE=Release 
CONFIGFLAGS_llvm_10 += -DCMAKE_LINKER=${BINUTILS_SOURCE}/bin/bin/ld.gold
endif

CONFIGFLAGS_llvm += -DLLVM_BINUTILS_INCDIR=${BINUTILS_SOURCE}/bin/include
CONFIGFLAGS_llvm += ${BINUTILS_SOURCE}/llvm

CONFIGFLAGS_llvm_10 += -DLLVM_BINUTILS_INCDIR=${BINUTILS_SOURCE}/bin/include
CONFIGFLAGS_llvm_10 += ${BINUTILS_SOURCE}/llvm_10/llvm 

CONFIGFLAGS_binutils := --prefix=${BINUTILS_SOURCE}/bin --enable-gold --enable-plugins --disable-werror
INSTALLFLAGS_binutils := tooldir=${BINUTILS_SOURCE}/bin

INSTALLFLAGS_llvm_10 := -j8


make_binutils:
	make -C build_binutils tooldir=${BINUTILS_SOURCE}/bin
make_llvm:
	true
make_llvm_10:
	true

make: $(patsubst %,make_%,$(DIRS))

install: $(patsubst %,install_%,$(DIRS))


install_llvm: CONFIG_PATH = $(LLVM_SOURCE)/
install_llvm_10: CONFIG_PATH =  $(LLVM_SOURCE)/
$(patsubst %,install_%,$(DIRS)): BUILD_DIR = $(subst install_,build_,$@)
$(patsubst %,install_%,$(DIRS)): DIR = $(subst install_,,$@)
$(patsubst %,install_%,$(DIRS)):
	@echo Building LLVM $(DIR)...
	$(MAKE) -C $(CONFIG_PATH)$(BUILD_DIR)  $(INSTALLFLAGS_$(DIR)) install

clean:
	$(foreach DIR, $(DIRS), $(MAKE) -C $(DIR) distclean;)

distclean: clean

config: $(patsubst %,configure_%,$(DIRS))

configure_llvm: CONFIG_EXEC = cmake
configure_llvm_10: CONFIG_EXEC = cmake
configure_binutils: CONFIG_EXEC = ../binutils/configure
configure_llvm: CONFIG_PATH = $(LLVM_SOURCE)/
configure_llvm_10: CONFIG_PATH =  $(LLVM_SOURCE)/
$(patsubst %,configure_%,$(DIRS)): DIR = $(subst configure_,,$@)
$(patsubst %,configure_%,$(DIRS)):
	mkdir $(CONFIG_PATH)build_$(DIR) ; cd $(CONFIG_PATH)build_$(DIR) ; \
	$(CONFIG_EXEC) $(CONFIGFLAGS_$(DIR)); \
	cd $(BINUTILS_SOURCE)
configure_cont_llvm: CONFIG_EXEC = cmake
configure_cont_llvm_10: CONFIG_EXEC = cmake
configure_cont_binutils: CONFIG_EXEC = ../binutils/configure
configure_cont_llvm: CONFIG_PATH = $(LLVM_SOURCE)/
configure_cont_llvm_10: CONFIG_PATH =  $(LLVM_SOURCE)/
$(patsubst %,configure_cont_%,$(DIRS)): DIR = $(subst configure_cont_,,$@)
$(patsubst %,configure_cont_%,$(DIRS)):
	cd $(CONFIG_PATH)build_$(DIR) ; \
	$(CONFIG_EXEC) $(CONFIGFLAGS_$(DIR)); \
	cd $(BINUTILS_SOURCE) 



cleanconfig: $(patsubst %,cleanconfig_%,$(DIRS))

$(patsubst %,cleanconfig_%,$(DIRS)): DIR = $(subst cleanconfig_,,$@)
$(patsubst %,cleanconfig_%,$(DIRS)):
	rm -rf build_$(DIR)

all: configure_binutils make_binutils install_binutils configure_llvm_10 install_llvm_10

