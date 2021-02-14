# sanitizer coverage
ROOT=..

include $(ROOT)/Makefile.inc

# Added backport from 5.3.2 to 5.1.0
DIRS=mainline backport kernel_5.4.0 baseline retpolines stackprotector combined combinedKASLR
TARGETS= build_kernel build_modules install_modules install_image
#CONFIG_TARGETS= clean_kernel clean def_config 
#CONFIG_TARGETS= clean def_config
CONFIG_TARGETS= clean def_preempt_config
KASLR_TARGETS= clean kaslr_config
WD=$(PWD)


ifeq ($(ENABLE_RETPOLINES),True)
#CONFIG_TARGETS+=retpolines
endif

ifeq ($(ENABLE_STACKPROTECTOR),True)
CONFIG_TARGETS+=stackprotector
KASLR_TARGETS+=stackprotector
endif

ifeq ($(ENABLE_CFI),True)
CONFIG_TARGETS+=cfi
endif

CORES=-j8


ifeq ($(LTO_ENABLED),True)


#LTO_LD_FLAGS=LTO_LD_FLAGS="-plugin $(LLVMLIB)/LLVMgold.so -plugin-opt=O0 -plugin-opt=emit-llvm -m elf_x86_64"
# must remove -ginline-tables-only and add it only to the have_icp custom
# Reminder  -flto  -fvisibility=default
#LTO_C_FLAGS=LTO_C_FLAGS="-gline-tables-only  -flto  -fvisibility=default"
COMPILATION_FLAGS=-gline-tables-only -flto -fvisibility=default

mainline_KCFLAGS +=-DHAVE_LLVM_LTO
mainline_KCFLAGS_MODULES +=-DHAVE_LLVM_LTO
backport_KCFLAGS +=-DHAVE_LLVM_LTO
backport_KCFLAGS_MODULES +=-DHAVE_LLVM_LTO
baseline_KCFLAGS +=-DHAVE_LLVM_LTO
baseline_KCFLAGS_MODULES +=-DHAVE_LLVM_LTO
retpolines_KCFLAGS +=-DHAVE_LLVM_LTO
retpolines_KCFLAGS_MODULES +=-DHAVE_LLVM_LTO
stackprotector_KCFLAGS +=-DHAVE_LLVM_LTO
stackprotector_KCFLAGS_MODULES +=-DHAVE_LLVM_LTO
combined_KCFLAGS +=-DHAVE_LLVM_LTO
combined_KCFLAGS_MODULES +=-DHAVE_LLVM_LTO
combinedKASLR_KCFLAGS +=-DHAVE_LLVM_LTO
combinedKASLR_KCFLAGS_MODULES +=-DHAVE_LLVM_LTO
kernel_5.4.0_KCFLAGS +=-DHAVE_LLVM_LTO
kernel_5.4.0_KCFLAGS_MODULES +=-DHAVE_LLVM_LTO


ifeq ($(ENABLE_STACKPROTECTOR),True)
backport_KCFLAGS +=-fstack-protector-all
backport_KCFLAGS_MODULES +=-fstack-protector-all
stackprotector_KCFLAGS +=-fstack-protector-all
stackprotector_KCFLAGS_MODULES +=-fstack-protector-all
combined_KCFLAGS +=-fstack-protector-all
combined_KCFLAGS_MODULES +=-fstack-protector-all
combinedKASLR_KCFLAGS +=-fstack-protector-all
combinedKASLR_KCFLAGS_MODULES +=-fstack-protector-all
endif

ifeq ($(ENABLE_RETPOLINES),True)
#CONFIG_TARGETS+=retpolines
#LTO_C_FLAGS=LTO_C_FLAGS="-gline-tables-only  -flto  -fvisibility=default -mretpoline"
COMPILATION_FLAGS+=-mretpoline
endif

ifeq ($(ENABLE_RETSPEC),True)
#LTO_C_FLAGS=LTO_C_FLAGS="-gline-tables-only  -flto  -fvisibility=default -mret-spec "
COMPILATION_FLAGS+=-mret-spec
endif



ifeq ($(ENABLE_LVI),True)
#backport_KCFLAGS +=-mlvi-cfi
#backport_KCFLAGS_MODULES +=-mlvi-cfi
backport_PER_FILE_FLAGS += CFLAGS_REMOVE_vclock_gettime.o+="-mlvi-cfi"
combined_PER_FILE_FLAGS += CFLAGS_REMOVE_vclock_gettime.o+="-mlvi-cfi"
combinedKASLR_PER_FILE_FLAGS += CFLAGS_REMOVE_vclock_gettime.o+="-mlvi-cfi"
#LTO_C_FLAGS=LTO_C_FLAGS="-gline-tables-only  -flto  -fvisibility=default -mlvi-cfi"
COMPILATION_FLAGS+=-mlvi-cfi


#backport_PER_FILE_FLAGS += CFLAGS_REMOVE_head64.o+="-mlvi-cfi"
#backport_PER_FILE_FLAGS += CFLAGS_REMOVE_alternative.o+="-mlvi-cfi"
#backport_PER_FILE_FLAGS += CFLAGS_REMOVE_extable.o+="-mlvi-cfi"
#backport_PER_FILE_FLAGS += CFLAGS_REMOVE_extable.o+="-mlvi-cfi"
#backport_PER_FILE_FLAGS += CFLAGS_REMOVE_traps.o+="-mlvi-cfi"


#backport_PER_FILE_FLAGS += CFLAGS_REMOVE_therm_throt.o+="-mlvi-cfi"
#backport_PER_FILE_FLAGS += CFLAGS_REMOVE_signal.o+="-mlvi-cfi"
#backport_PER_FILE_FLAGS += CFLAGS_REMOVE_uncore_nhmex.o+="-mlvi-cfi"
#backport_PER_FILE_FLAGS += CFLAGS_REMOVE_pgtable.o+="-mlvi-cfi"
#backport_PER_FILE_FLAGS += CFLAGS_REMOVE_ldt.o+="-mlvi-cfi"
#backport_PER_FILE_FLAGS += CFLAGS_REMOVE_srcutree.o+="-mlvi-cfi"
#backport_PER_FILE_FLAGS += CFLAGS_REMOVE_fair.o+="-mlvi-cfi"
#backport_PER_FILE_FLAGS += CFLAGS_REMOVE_hugetlb.o+="-mlvi-cfi"

LTO_LVI=LTO_LVI=-mattr=+lvi-cfi
PIBE_RUN_LVI_BLACKLIST=PIBE_RUN_LVI_BLACKLIST=Yes
endif


LTO_PASS_PATH=PASS_PATH=$(INSTALL_PASS_DIR)

ifeq ($(HAVE_VUPROFILER),True)
mainline_KCFLAGS +=-DHAVE_VUPROFILER
mainline_KCFLAGS_MODULES +=-DHAVE_VUPROFILER
backport_KCFLAGS +=-DHAVE_VUPROFILER
backport_KCFLAGS_MODULES +=-DHAVE_VUPROFILER
kernel_5.4.0_KCFLAGS +=-DHAVE_VUPROFILER
kernel_5.4.0_KCFLAGS_MODULES +=-DHAVE_VUPROFILER
LTO_HAVE_PROFILER=HAVE_VUPROFILER=True
endif

ifeq ($(HAVE_ICP),Custom)
LTO_HAVE_ICP=HAVE_ICP=Custom
endif

ifeq ($(HAVE_ICP),LLVM)
LTO_HAVE_ICP=HAVE_ICP=LLVM
endif

ifeq ($(HAVE_ICP),NOICP)
LTO_HAVE_ICP=HAVE_ICP=NOICP
endif

ifeq ($(COMPILE_FAST),True)
LTO_COMPILE_FAST=COMPILE_FAST=True
endif

PIBE_ANNOTATE_PROFILE:=PIBE_ANNOTATE_PROFILE=$(PIBE_ANNOTATE_PROFILE)
LTO_PIBE_OPTIMIZATIONS=PIBE_OPTIMIZATIONS=$(PIBE_OPTIMIZATIONS)
PIBE_WORKLOAD_FILE:=PIBE_WORKLOAD_FILE=$(PIBE_WORKLOAD_FILE)
LTO_PIBE_BUDGET=PIBE_BUDGET=$(PIBE_BUDGET)
LTO_DEFENSE_CONFIGURATION=DEFENSE_CONFIGURATION=$(DEFENSE_CONFIGURATION)
LTO_WORKLOAD_CONFIGURATION=WORKLOAD_CONFIGURATION=$(WORKLOAD_FOLDER)



LTO_DIS=LLVM_DIS=$(LLVM_DIS)
LTO_OPT=OPT=$(OPT)
LTO_STRIP=STRIP=$(LLVM_STRIP)

LTO_C_FLAGS=LTO_C_FLAGS="$(COMPILATION_FLAGS)"
#LTO_FLAGS+=$(LTO_LD_FLAGS)
LTO_FLAGS+=$(LTO_C_FLAGS)
LTO_FLAGS+=$(LTO_PASS_PATH)
LTO_FLAGS+=$(LTO_HAVE_ICP)
LTO_FLAGS+=$(LTO_HAVE_PROFILER)
LTO_FLAGS+=$(LTO_DIS)
LTO_FLAGS+=$(LTO_OPT)
LTO_FLAGS+=$(LTO_STRIP)
LTO_FLAGS+=$(LTO_COMPILE_FAST)
LTO_FLAGS+=$(LTO_LVI)
LTO_FLAGS+=$(PIBE_ANNOTATE_PROFILE)
LTO_FLAGS+=$(LTO_PIBE_OPTIMIZATIONS)
LTO_FLAGS+=$(PIBE_WORKLOAD_FILE)
LTO_FLAGS+=$(LTO_PIBE_BUDGET)
LTO_FLAGS+=$(PIBE_RUN_LVI_BLACKLIST)
LTO_FLAGS+=$(LTO_DEFENSE_CONFIGURATION)
LTO_FLAGS+=$(LTO_WORKLOAD_CONFIGURATION)


#CONFIG_TARGETS+=lto_config

endif

#CONFIG_TARGETS+=lto_config

mainline_REDIRECT_BUILD_KERNEL= 1> mainline_kernel_dump 2> mainline_kernel_dump
mainline_REDIRECT_BUILD_MODULES= 1> mainline_kernel_module_dump 2> mainline_kernel_module_dump
mainline_REDIRECT_INSTALL_IMAGE= 1> mainline_kernel_install_dump 2> mainline_kernel_install_dump
mainline_REDIRECT_INSTALL_MODULES= 1> mainline_kernel_module_install_dump 2> mainline_kernel_module_install_dump

backport_REDIRECT_BUILD_KERNEL= 1> backport_kernel_dump 2> backport_kernel_dump
backport_REDIRECT_BUILD_MODULES= 1> backport_kernel_module_dump 2> backport_kernel_module_dump
backport_REDIRECT_INSTALL_IMAGE= 1> backport_kernel_install_dump 2> backport_kernel_install_dump
backport_REDIRECT_INSTALL_MODULES= 1> backport_kernel_module_install_dump 2> backport_kernel_module_install_dump

baseline_REDIRECT_BUILD_KERNEL= 1> baseline_kernel_dump 2> baseline_kernel_dump
baseline_REDIRECT_BUILD_MODULES= 1> baseline_kernel_module_dump 2> baseline_kernel_module_dump
baseline_REDIRECT_INSTALL_IMAGE= 1> baseline_kernel_install_dump 2> baseline_kernel_install_dump
baseline_REDIRECT_INSTALL_MODULES= 1> baseline_kernel_module_install_dump 2> baseline_kernel_module_install_dump

retpolines_REDIRECT_BUILD_KERNEL= 1> retpolines_kernel_dump 2> retpolines_kernel_dump
retpolines_REDIRECT_BUILD_MODULES= 1> retpolines_kernel_module_dump 2> retpolines_kernel_module_dump
retpolines_REDIRECT_INSTALL_IMAGE= 1> retpolines_kernel_install_dump 2> retpolines_kernel_install_dump
retpolines_REDIRECT_INSTALL_MODULES= 1> retpolines_kernel_module_install_dump 2> retpolines_kernel_module_install_dump

stackprotector_REDIRECT_BUILD_KERNEL= 1> stackprotector_kernel_dump 2> stackprotector_kernel_dump
stackprotector_REDIRECT_BUILD_MODULES= 1> stackprotector_kernel_module_dump 2> stackprotector_kernel_module_dump
stackprotector_REDIRECT_INSTALL_IMAGE= 1> stackprotector_kernel_install_dump 2> stackprotector_kernel_install_dump
stackprotector_REDIRECT_INSTALL_MODULES= 1> stackprotector_kernel_module_install_dump 2> stackprotector_kernel_module_install_dump

combined_REDIRECT_BUILD_KERNEL= 1> combined_kernel_dump 2> combined_kernel_dump
combined_REDIRECT_BUILD_MODULES= 1> combined_kernel_module_dump 2> combined_kernel_module_dump
combined_REDIRECT_INSTALL_IMAGE= 1> combined_kernel_install_dump 2> combined_kernel_install_dump
combined_REDIRECT_INSTALL_MODULES= 1> combined_kernel_module_install_dump 2> combined_kernel_module_install_dump

combinedKASLR_REDIRECT_BUILD_KERNEL= 1> combinedKASLR_kernel_dump 2> combinedKASLR_kernel_dump
combinedKASLR_REDIRECT_BUILD_MODULES= 1> combinedKASLR_kernel_module_dump 2> combinedKASLR_kernel_module_dump
combinedKASLR_REDIRECT_INSTALL_IMAGE= 1> combinedKASLR_kernel_install_dump 2> combinedKASLR_kernel_install_dump
combinedKASLR_REDIRECT_INSTALL_MODULES= 1> combinedKASLR_kernel_module_install_dump 2> combinedKASLR_kernel_module_install_dump

kernel_5.4.0_REDIRECT_BUILD_KERNEL= 1> kernel_5.4.0_kernel_dump 2> kernel_5.4.0_kernel_dump
kernel_5.4.0_REDIRECT_BUILD_MODULES= 1> kernel_5.4.0_kernel_module_dump 2> kernel_5.4.0_kernel_module_dump
kernel_5.4.0_REDIRECT_INSTALL_IMAGE= 1> kernel_5.4.0_kernel_install_dump 2> kernel_5.4.0_kernel_install_dump
kernel_5.4.0_REDIRECT_INSTALL_MODULES= 1> kernel_5.4.0_kernel_module_install_dump 2> kernel_5.4.0_kernel_module_install_dump


all: $(patsubst %,all_%,$(DIRS))

all_baseKernel: $(patsubst %,%_baseKernel,$(TARGETS))

all_llvmLinux: $(patsubst %,%_llvmLinux,$(TARGETS))

all_mainline: $(patsubst %,%_mainline,$(TARGETS))

all_backport: $(patsubst %,%_backport,$(TARGETS))

install_llvmLinux:
	cd $(ROOT); ./copy_kernel.sh; cd $(WD)
install_baseKernel:
	true

#HOSTCC=$(CXX)
$(patsubst %,install_image_%,$(DIRS)): DIR = $(subst install_image_,,$@)
$(patsubst %,install_image_%,$(DIRS)): KCFLAGS = $($(DIR)_KCFLAGS_MODULES)
$(patsubst %,install_image_%,$(DIRS)): REDIRECT_INSTALL_IMAGE = $($(DIR)_REDIRECT_INSTALL_IMAGE)
$(patsubst %,install_image_%,$(DIRS)):
	sudo $(MAKE) -C $(DIR) install CC=$(CXX)  AS=$(AS) LD=$(LD)  KCFLAGS="$(KCFLAGS)" $(CORES) $(VERBOSE) $(REDIRECT_INSTALL_IMAGE)
#HOSTCC=$(CXX)
$(patsubst %,install_modules_%,$(DIRS)): DIR = $(subst install_modules_,,$@)
$(patsubst %,install_modules_%,$(DIRS)): KCFLAGS_MODULES = $($(DIR)_KCFLAGS_MODULES)
$(patsubst %,install_modules_%,$(DIRS)): REDIRECT_INSTALL_MODULES = $($(DIR)_REDIRECT_INSTALL_MODULES)
$(patsubst %,install_modules_%,$(DIRS)):
	sudo $(MAKE) -C $(DIR) modules_install CC=$(CXX)  AS=$(AS) LD=$(LD)  KCFLAGS="$(KCFLAGS_MODULES)" $(CORES) $(VERBOSE)  $(REDIRECT_INSTALL_MODULES)

#HOSTCC=$(CXX)
$(patsubst %,build_modules_%,$(DIRS)): DIR = $(subst build_modules_,,$@)
$(patsubst %,build_modules_%,$(DIRS)): KCFLAGS_MODULES = $($(DIR)_KCFLAGS_MODULES)
$(patsubst %,build_modules_%,$(DIRS)): REDIRECT_BUILD_MODULES = $($(DIR)_REDIRECT_BUILD_MODULES)
$(patsubst %,build_modules_%,$(DIRS)):
	$(MAKE) -C $(DIR) modules CC=$(CXX) AS=$(AS) LD=$(LD) KCFLAGS="$(KCFLAGS_MODULES)" $(CORES) $(VERBOSE) $(REDIRECT_BUILD_MODULES)

#HOSTCC=$(CXX)
$(patsubst %,build_kernel_%,$(DIRS)): DIR = $(subst build_kernel_,,$@)
$(patsubst %,build_kernel_%,$(DIRS)): KCFLAGS = $($(DIR)_KCFLAGS)
$(patsubst %,build_kernel_%,$(DIRS)): KAFLAGS = $($(DIR)_KAFLAGS)
$(patsubst %,build_kernel_%,$(DIRS)): KLDFLAGS = $($(DIR)_KLDFLAGS)
$(patsubst %,build_kernel_%,$(DIRS)): PER_FILE_FLAGS = $($(DIR)_PER_FILE_FLAGS)
$(patsubst %,build_kernel_%,$(DIRS)): REDIRECT_BUILD_KERNEL = $($(DIR)_REDIRECT_BUILD_KERNEL)
$(patsubst %,build_kernel_%,$(DIRS)):
	$(MAKE) -C $(DIR) LLVM_NM=$(LLVM_NM) LLC=$(LLC) CC=$(CXX)  AR=$(AR) AS=$(AS) LD=$(LD) HAVE_LTO=$(LTO_ENABLED) $(LTO_FLAGS) $(PER_FILE_FLAGS) KCFLAGS="$(KCFLAGS)" KAFLAGS="$(KAFLAGS)" $(CORES) $(VERBOSE) $(REDIRECT_BUILD_KERNEL) 
	true



config_backport: $(patsubst %,%_backport,$(CONFIG_TARGETS))

config_baseline: $(patsubst %,%_baseline,$(CONFIG_TARGETS))
config_stackprotector: $(patsubst %,%_stackprotector,$(CONFIG_TARGETS))
config_retpolines: $(patsubst %,%_retpolines,$(CONFIG_TARGETS))
config_combined: $(patsubst %,%_combined,$(CONFIG_TARGETS))
config_combinedKASLR: $(patsubst %,%_combinedKASLR,$(KASLR_TARGETS))


config_mainline: $(patsubst %,%_mainline,$(CONFIG_TARGETS))

config_kernel_5.4.0: $(patsubst %,%_kernel_5.4.0,$(CONFIG_TARGETS))


#HOSTCC=$(CXX) -e CONFIG_STACKPROTECTOR -e CONFIG_STACKPROTECTOR_STRONG -e CONFIG_STACKPROTECTOR -e CONFIG_LTO_NONE
$(patsubst %,def_config_%,$(DIRS)): DIR = $(subst def_config_,,$@)
$(patsubst %,def_config_%,$(DIRS)):
	$(MAKE) -C $(DIR) x86_64_defconfig CC=$(CXX)  AS=$(AS) LD=$(LD) 
	$(MAKE) -C $(DIR) kvmconfig CC=$(CXX) AS=$(AS)  LD=$(LD)
	cd $(DIR); ./scripts/config -e CONFIG_UNWINDER_FRAME_POINTER -d CONFIG_STACK_VALIDATION -d CONFIG_RANDOMIZE_BASE -d CONFIG_PREEMPT_VOLUNTARY -d CONFIG_RETPOLINE -d CONFIG_STACKPROTECTOR  ; cd ..

$(patsubst %,kaslr_config_%,$(DIRS)): DIR = $(subst kaslr_config_,,$@)
$(patsubst %,kaslr_config_%,$(DIRS)):
	$(MAKE) -C $(DIR) x86_64_defconfig CC=$(CXX)  AS=$(AS) LD=$(LD) 
	$(MAKE) -C $(DIR) kvmconfig CC=$(CXX) AS=$(AS)  LD=$(LD)
	cd $(DIR); ./scripts/config -e CONFIG_UNWINDER_FRAME_POINTER -d CONFIG_STACK_VALIDATION -e CONFIG_RANDOMIZE_BASE -e CONFIG_PREEMPT_VOLUNTARY -d CONFIG_RETPOLINE -d CONFIG_STACKPROTECTOR  ; cd ..

$(patsubst %,def_preempt_config_%,$(DIRS)): DIR = $(subst def_preempt_config_,,$@)
$(patsubst %,def_preempt_config_%,$(DIRS)):
	$(MAKE) -C $(DIR) x86_64_defconfig CC=$(CXX)  AS=$(AS) LD=$(LD) 
	$(MAKE) -C $(DIR) kvmconfig CC=$(CXX) AS=$(AS)  LD=$(LD)
	cd $(DIR); ./scripts/config -e CONFIG_UNWINDER_FRAME_POINTER -d CONFIG_STACK_VALIDATION  -d CONFIG_RANDOMIZE_BASE -d CONFIG_RETPOLINE -d CONFIG_STACKPROTECTOR  ; cd .. 
	

$(patsubst %,clean_%,$(DIRS)): DIR = $(subst clean_,,$@)
$(patsubst %,clean_%,$(DIRS)):
	$(MAKE) -C $(DIR) mrproper

$(patsubst %,clean_kernel_%,$(DIRS)): DIR = $(subst clean_kernel_,,$@)
$(patsubst %,clean_kernel_%,$(DIRS)):
	cd $(ROOT);./clean_kernel.sh ${DIR}; cd $(WD)

$(patsubst %,retpolines_%,$(DIRS)): DIR = $(subst retpolines_,,$@)
$(patsubst %,retpolines_%,$(DIRS)):
#	cd $(DIR); ./scripts/kconfig/merge_config.sh -m -r .config .config-noretpolines; cd ..
	cd $(DIR); ./scripts/config -e CONFIG_RETPOLINE; cd ..

$(patsubst %,stackprotector_%,$(DIRS)): DIR = $(subst stackprotector_,,$@)
$(patsubst %,stackprotector_%,$(DIRS)):
	cd $(DIR); ./scripts/config -e CONFIG_STACKPROTECTOR; cd ..

$(patsubst %,cfi_%,$(DIRS)): DIR = $(subst cfi_,,$@)
$(patsubst %,cfi_%,$(DIRS)):
        # Reminder -e CONFIG_LTO -e CONFIG_THINLTO -d CONFIG_LTO_NONE  -e CONFIG_LTO_CLANG  
	cd $(DIR); ./scripts/config  -e CONFIG_CFI_CLANG -e CONFIG_CFI_PERMISSIVE -e CONFIG_CFI_CLANG_SHADOW -e CONFIG_LKDTM; cd ..

clean_dump:
	rm -f kernel_*
	rm -f base_kernel_*

#include Makefile.cfi




