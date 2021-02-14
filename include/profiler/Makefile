ROOT=../..
include $(ROOT)/Makefile.inc


DFLAGS +=-DHAVE_VUPROFILER -DHAVE_LLVM_LTO -DNUM_UNITS=$(CORE_UNITS) -DLBR_ENTRIES=$(LBR_UNITS)


KINCLUDES= -nostdinc -isystem $(LLVMPREFIX)/lib/clang/10.0.0/include -I$(KERNEL_MAIN_DIR)/arch/x86/include -I$(KERNEL_MAIN_DIR)/arch/x86/include/generated  -I$(KERNEL_MAIN_DIR)/include -I$(KERNEL_MAIN_DIR)/arch/x86/include/uapi -I$(KERNEL_MAIN_DIR)/arch/x86/include/generated/uapi -I$(KERNEL_MAIN_DIR)/include/uapi -I$(KERNEL_MAIN_DIR)/include/generated/uapi -include $(KERNEL_MAIN_DIR)/include/linux/kconfig.h -include $(KERNEL_MAIN_DIR)/include/linux/compiler_types.h -D__KERNEL__ -Qunused-arguments -Wall -Wundef -Werror=strict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -fshort-wchar -fno-PIE -Werror=implicit-function-declaration -Werror=implicit-int -Wno-format-security -std=gnu89 -no-integrated-as -Werror=unknown-warning-option -mno-sse -mno-mmx -mno-sse2 -mno-3dnow -mno-avx -m64 -mno-80387 -mstack-alignment=8 -mtune=generic -mno-red-zone -mcmodel=kernel -DCONFIG_AS_CFI=1 -DCONFIG_AS_CFI_SIGNAL_FRAME=1 -DCONFIG_AS_CFI_SECTIONS=1 -DCONFIG_AS_SSSE3=1 -DCONFIG_AS_AVX=1 -DCONFIG_AS_AVX2=1 -DCONFIG_AS_AVX512=1 -DCONFIG_AS_SHA1_NI=1 -DCONFIG_AS_SHA256_NI=1 -Wno-sign-compare -fno-asynchronous-unwind-tables -fno-delete-null-pointer-checks -Wno-address-of-packed-member -O2 -Wframe-larger-than=2048 -fstack-protector-strong -Wno-format-invalid-specifier -Wno-gnu -Wno-tautological-compare -mno-global-merge -Wno-unused-const-variable -fno-omit-frame-pointer -fno-optimize-sibling-calls -Wdeclaration-after-statement -Wimplicit-fallthrough -Wvla -Wno-pointer-sign -fno-strict-overflow -fno-merge-all-constants -fno-stack-check -Werror=date-time -Werror=incompatible-pointer-types -Wno-initializer-overrides -Wno-format -Wno-sign-compare -Wno-format-zero-length -I$(INCLUDE_LBR_DIR)

tag_callbacks.bc: tag_callbacks.c
	$(CXX) -flto  $(DFLAGS) $(KINCLUDES) tag_callbacks.c -c -o tag_callbacks.bc

test_callbacks: tag_callbacks.c
	$(CXX) -flto $(KINCLUDES) $(DFLAGS) tag_callbacks.c -E -o tag_callbacks.tst

tagLib.bc: tag_callbacks.bc
	$(AR) -rcsTPD tagLib.bc tag_callbacks.bc

install: $(INSTALL_TOP_DIR)/tagLib.bc $(INSTALL_TOP_DIR)/tag_callbacks.bc

$(INSTALL_TOP_DIR)/tagLib.bc: tagLib.bc
	 install -c -D -m 744 $? $@

$(INSTALL_TOP_DIR)/tag_callbacks.bc: tag_callbacks.bc
	 install -c -D -m 744 $? $@

all: install

clean:
	rm -f tagLib.bc tag_callbacks.bc $(INSTALL_TOP_DIR)/tagLib.bc $(INSTALL_TOP_DIR)/tag_callbacks.bc
