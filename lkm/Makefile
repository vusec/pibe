
# Run me with GNU make
WD := $(PWD)
ROOT=$(WD)/..


include $(ROOT)/Makefile.inc


MODULE = vuprofiler-module
OBJS = vuprofiler_module.o  \
	   lbr.o 


MODULE_NAME=$(MODULE).ko

KERNEL_VERSION=$(shell uname -r)


KDIR ?= /lib/modules/$(KERNEL_VERSION)/build

MCFLAGS += -I$(INCLUDE_PROFILER_DIR) -I$(INCLUDE_LBR_DIR) -I$(INCLUDE_MODULE_DIR)  -DSHADOW_DEBUG  -fno-builtin  -DNUM_UNITS=$(CORE_UNITS) -DLBR_ENTRIES=$(LBR_UNITS)
UCFLAGS = -I$(INCLUDE_MODULE_DIR) -I$(INCLUDE_PROFILER_DIR)

ifeq ($(ACTIVATE_LBR_CSWITCH),True)
MCFLAGS +=-DINCLUDE_LBR_CONTEXT_SWITCHING
endif

all: $(MODULE_NAME) vuprof_ioctl

install:
	true

obj-m += $(MODULE).o
$(MODULE)-objs := $(OBJS)

$(MODULE_NAME): $(HEADERS) $(patsubst %.o,%.c,$(OBJS))
	$(QUIET) make -C $(KDIR) M=$(CURDIR) modules EXTRA_CFLAGS="$(MCFLAGS)" CC=$(LLVMBIN)/clang $(VERBOSE) LD=$(LD)

vuprof_ioctl: vuprof_ioctl.c
	$(LLVMBIN)/clang  $(UCFLAGS) vuprof_ioctl.c -o vuprof_ioctl

clean:
	$(RM) -rf *.o $(MODULE_NAME) .tmp_versions Module.symvers modules.order vuprofiler-module.mod.c .*.o.cmd .*.ko.cmd vuprof_ioctl
