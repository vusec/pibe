PREFIX:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
#PREFIX:=/projects/icp_kernel

CORE_UNITS=$(shell lscpu | grep -m1 CPU\(s\) | awk '{print $$2}')
LBR_UNITS=$(shell dmesg | grep -i lbr| python3 $(PREFIX)/determine_lbr_size.py)
HAVE_LLVM=1

##########################################
#  Project specific defines
##########################################

BUILD_LLVM_TINY=build_llvm_tiny


# Defines used by module and pass build system
# KERNEL_BUILD_TYPE = debug/release
KERNEL_BUILD_TYPE=release
KERNEL_VERSION=5.1.0

CHECK_FOR_LOCAL_BUFFERS=False

# Defines used by kernel build system

LTO_ENABLED=True
HAVE_GOLD=True

# Disable repolines in the kernel
# This configuration also disables stackcheck and orc unwinder
# by setting CONFIG_UNWINDER_FRAME_POINTER=y and CONFIG_STACK_VALIDATION=n
ifeq ($(ENABLE_RETPOLINES),)
ENABLE_RETPOLINES=False
endif
ENABLE_STACKPROTECTOR=False
ifeq ($(ENABLE_LVI),)
ENABLE_LVI=False
endif
ENABLE_CFI=False
ifeq ($(ENABLE_RETSPEC),)
ENABLE_RETSPEC=False
endif

ifeq ($(HAVE_VUPROFILER),)
HAVE_VUPROFILER=False
endif

# must be either Custom or LLVM NOICP
ifeq ($(HAVE_ICP),)
HAVE_ICP=NOICP
endif

#Skip blob assembly and apply passes directly on the previous blob.
COMPILE_FAST=False

#Use for annotation and running optimizations
#Either Yes to annotate or anything else to run with previous annotated blob
PIBE_ANNOTATE_PROFILE=Yes
#Either Promote Inline or PromoteAndInline (any other value and optimizations aren't run)
ifeq ($(PIBE_OPTIMIZATIONS),)
PIBE_OPTIMIZATIONS=PromoteAndInline
endif
# What budget to run optimizations with

ifeq ($(PIBE_BUDGET),)
PIBE_BUDGET=999999
endif


ifeq ($(WORKLOAD_FOLDER),)
WORKLOAD_FOLDER=lmbench3
endif

ifeq ($(WORKLOAD_FILE),)
WORKLOAD_FILE=Decoded.Profiles
endif

ifeq ($(DEFENSE_CONFIGURATION),)
DEFENSE_CONFIGURATION=default
endif

WORKLOAD_SAMPLE_FILE=decoded.profdata

PIBE_WORKLOAD_FOLDER=$(PREFIX)/playground/metadata/${WORKLOAD_FOLDER}


PIBE_WORKLOAD_FILE=${PIBE_WORKLOAD_FOLDER}/${WORKLOAD_SAMPLE_FILE}

export PROFILE_FILE=$(PIBE_WORKLOAD_FOLDER)/$(WORKLOAD_FILE)
export SAMPLE_FILE=$(PIBE_WORKLOAD_FOLDER)/$(WORKLOAD_SAMPLE_FILE)

#
# Note: you should move all main projects: 
# kernel, llvm toolchain, passes, 
# modules must have a similar entry
# around here
#

INCLUDE_TOP_DIR=${PREFIX}/include
INCLUDE_MODULE_DIR=${INCLUDE_TOP_DIR}/lkm
INCLUDE_LBR_DIR=${INCLUDE_TOP_DIR}/lbr
INCLUDE_UTILS_DIR=${INCLUDE_TOP_DIR}/utils
INCLUDE_PROFILER_DIR=${INCLUDE_TOP_DIR}/profiler

INSTALL_TOP_DIR=${PREFIX}/passes/install
INSTALL_PASS_DIR=${INSTALL_TOP_DIR}
INSTALL_VUPROF_DIR=${INSTALL_TOP_DIR}

KERNEL_TOP_DIR=${PREFIX}/kernel
KERNEL_MAIN_DIR=${KERNEL_TOP_DIR}/backport

DEBUG_DIR=${PREFIX}/playground


#BACKUP_DIR=${PREFIX}/backup

ECHO=echo
QUIET=
VERBOSE=V=1


##################################################################################################
# Clang related configuration (Default)
#
BINUTILS_SOURCE=${PREFIX}/tools
LLVM_SOURCE=${PREFIX}/tools
LLVM_MODULES=${PREFIX}/lkm
LLVM_PASSES=${PREFIX}/passes

LLVMPREFIX=$(LLVM_SOURCE)/bin
LLVMBIN=$(LLVMPREFIX)/bin
LLVMINC=$(LLVMPREFIX)/include
LLVMLIB=$(LLVMPREFIX)/lib

PASSDIR=${PREFIX}/passes
##################################################################################################
# Compiler specific (Default)
#

CXX=$(LLVMBIN)/clang
OPT=$(LLVMBIN)/opt
LLC=$(LLVMBIN)/llc
AS=$(BINUTILS_SOURCE)/bin/bin/as
AR=$(LLVMBIN)/llvm-ar
LD=$(LLVMBIN)/ld.lld
LLVM_AS=$(LLVMBIN)/llvm-as
LLVM_DIS=$(LLVMBIN)/llvm-dis
DEFAULT_LD=$(LLVMBIN)/ld
LLVMEXTRACT=$(LLVMPREFIX)/bin/opt
LLVM_NM=$(LLVMBIN)/llvm-nm
LLVM_LINK=$(LLVMBIN)/llvm-link
LLVM_EXTRACT=$(LLVMBIN)/llvm-extract
LLVM_STRIP=$(LLVMBIN)/llvm-strip


#export CODEGEN_PASS_PATH=$(INSTALL_PASS_DIR)/pad_pass.so
export PATH=$(BINUTILS_SOURCE)/bin/bin:$(LLVMBIN):$(LLVM_SOURCE)/bin/bin:$(shell printenv PATH)
export LD_LIBRARY_PATH=$(LLVMLIB)
export LIBRARY_PATH=$(LLVMLIB)


##################################################################################################
# Configuration modifiers. 
# 


ifeq ($(HAVE_LLVM),1)
LLVM_VERSION=$(shell $(LLVMPREFIX)/bin/llvm-config --version | cut -d"." -f1,2 | sed "s/[^0-9]//g")
else
LLVM_VERSION=0
endif





