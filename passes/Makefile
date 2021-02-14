DIRS := vuprofiler static_symrez asm_pass icp_convert vuinliner linearpromotion bl_lvi analyze_hotness cb_inliner

ROOT=..
include $(ROOT)/Makefile.inc

all: $(patsubst %,build_%,$(DIRS))

$(patsubst %,build_%,$(DIRS)): DIR = $(subst build_,,$@)
$(patsubst %,build_%,$(DIRS)):
	@echo Building LLVM $(DIR)...
	$(MAKE) -C $(DIR) install

clean:
	$(foreach DIR, $(DIRS), $(MAKE) -C $(DIR) distclean;)

distclean: clean

install: all

