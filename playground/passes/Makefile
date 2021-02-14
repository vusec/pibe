ROOT=../..

include $(ROOT)/Makefile.inc


extract_sys:
	#$(LLVM_EXTRACT) -S -func  printk  -o test.ll after_cfi.bc
	#$(LLVM_EXTRACT) -S -func  printk -o test_2.ll before.bc
	$(LLVM_EXTRACT) -S --func=do_sys_poll --func=do_select --func=vfs_write --func=do_syscall_64 --func=ksys_read --func=core_sys_select -o test_1.ll inlined_990000.bc

extract_help:
	$(OPT) --help-list-hidden

ldd_help:
	$(LD) --plugin-opt=--sample-profile-file=/projects/icp_kernel/playground/metadata/decoded.profdata1 preopt.bc -o dummy
extract_so:
	$(LLVM_EXTRACT) -S --func=vfs_write -o test_1.ll dummy.bc
	$(LLVM_EXTRACT) -S --func=vfs_write -o test_2.ll discriminator.bc
create_image:
	#$(OPT)  -add-discriminators -o discriminator.bc bare_kernel.bc
	$(OPT) -load=$(INSTALL_PASS_DIR)/icp_convert.so -icp_convert -o  dummy.bc discriminator.bc > icp_dump
	#$(OPT) -disable-inlining --static-func-full-module-prefix=false --static-func-strip-dirname-prefix=30 --sample-profile --sample-profile-file=/projects/icp_kernel/playground/metadata/5.1.0.workload/decoded.profdata.All  -pass-remarks-missed=SampleProfileLoaderLegacyPass -pass-remarks=SampleProfileLoaderLegacyPass  -pass-remarks-output=remarks_sampleprofile.kernel  discriminator.bc -o sample_profile.bc
	#$(OPT) -disable-inlining --profile-sample-accurate  --static-func-full-module-prefix=false --static-func-strip-dirname-prefix=30 --sample-profile --sample-profile-file=/projects/icp_kernel/playground/metadata/5.1.0.workload/decoded.profdata.All  -pass-remarks-missed=SampleProfileLoaderLegacyPass -pass-remarks=SampleProfileLoaderLegacyPass  -pass-remarks-output=remarks_sampleprofile.kernel  discriminator.bc -o sample_profile_accurate.bc

check_hotness:
	$(OPT) -disable-inlining --profile-sample-accurate --static-func-full-module-prefix=false --static-func-strip-dirname-prefix=30 --sample-profile --sample-profile-file=/projects/icp_kernel/playground/metadata/5.1.0.workload/decoded.profdata.All  -pass-remarks-missed=SampleProfileLoaderLegacyPass -pass-remarks=SampleProfileLoaderLegacyPass  -pass-remarks-output=remarks_sampleprofile.kernel  dummy.bc -o sample_profile_dummy.bc
	$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -profile-summary-cutoff-hot=999000 --profile-summary-cold-count=0 -analyze_hotness -o  res.bc sample_profile_accurate.bc  > analyze_dump_nomut
	$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -profile-summary-cutoff-hot=999000 --profile-summary-cold-count=0 -analyze_hotness -o  res.bc sample_profile_dummy.bc > analyze_dump_mut

check_hotness_2:
	$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -profile-summary-cutoff-hot=999999 --profile-summary-cold-count=0 -analyze_hotness -o  res.bc  sample_profile_rez.bc  > frequency_dump 
inliner_test:
	$(OPT) -load=$(INSTALL_PASS_DIR)/inliner.so -hotness_inliner --profile-summary-cutoff-hot=999999 -o  sso2.bc  sample_profile_rez.bc > some_dump_2

get_hot_stats:
	# $(OPT) -disable-inlining -load=$(INSTALL_PASS_DIR)/linearpromotion.so  -icp_linear --profile-summary-cutoff-hot=990000  -icp-samplepgo-linear --icp-max-prom=40  --icp-remaining-percent-threshold=0 --icp-total-percent-threshold=0  -pass-remarks=PGOIndirectCallPromotion   -pass-remarks-missed=PGOIndirectCallPromotion   -pass-remarks-output=icp_990000.remarks  testimage.bc -o sample_profile_icp.bc
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -profile-summary-cutoff-hot=990000 --profile-summary-cold-count=0 -analyze_hotness -o  res.bc sample_profile_icp.bc  > hotstats.990000
	#$(OPT) -disable-inlining -load=$(INSTALL_PASS_DIR)/linearpromotion.so  -icp_linear --profile-summary-cutoff-hot=999000  -icp-samplepgo-linear --icp-max-prom=40  --icp-remaining-percent-threshold=0 --icp-total-percent-threshold=0  -pass-remarks=PGOIndirectCallPromotion   -pass-remarks-missed=PGOIndirectCallPromotion   -pass-remarks-output=icp_999000.remarks  testimage.bc -o sample_profile_icp.bc
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -profile-summary-cutoff-hot=999000 --profile-summary-cold-count=0 -analyze_hotness -o  res.bc sample_profile_icp.bc  > hotstats.999000
	$(OPT) -disable-inlining -load=$(INSTALL_PASS_DIR)/linearpromotion.so  -icp_linear --profile-summary-cutoff-hot=999900  -icp-samplepgo-linear --icp-max-prom=40  --icp-remaining-percent-threshold=0 --icp-total-percent-threshold=0  -pass-remarks=PGOIndirectCallPromotion   -pass-remarks-missed=PGOIndirectCallPromotion   -pass-remarks-output=icp_999900.remarks  testimage.bc -o sample_profile_icp.bc
	$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -profile-summary-cutoff-hot=999900 --profile-summary-cold-count=0 -analyze_hotness -o  res.bc sample_profile_icp.bc  > hotstats.999900
	$(OPT) -disable-inlining -load=$(INSTALL_PASS_DIR)/linearpromotion.so  -icp_linear --profile-summary-cutoff-hot=999990  -icp-samplepgo-linear --icp-max-prom=40  --icp-remaining-percent-threshold=0 --icp-total-percent-threshold=0  -pass-remarks=PGOIndirectCallPromotion   -pass-remarks-missed=PGOIndirectCallPromotion   -pass-remarks-output=icp_999990.remarks  testimage.bc -o sample_profile_icp.bc
	$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -profile-summary-cutoff-hot=999990 --profile-summary-cold-count=0 -analyze_hotness -o  res.bc sample_profile_icp.bc  > hotstats.999990
	$(OPT) -disable-inlining -load=$(INSTALL_PASS_DIR)/linearpromotion.so  -icp_linear --profile-summary-cutoff-hot=999999  -icp-samplepgo-linear --icp-max-prom=40  --icp-remaining-percent-threshold=0 --icp-total-percent-threshold=0  -pass-remarks=PGOIndirectCallPromotion   -pass-remarks-missed=PGOIndirectCallPromotion   -pass-remarks-output=icp_999999.remarks  testimage.bc -o sample_profile_icp.bc
	$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -profile-summary-cutoff-hot=999999 --profile-summary-cold-count=0 -analyze_hotness -o  res.bc sample_profile_icp.bc  > hotstats.999999

get_target_distribution:
	$(OPT) -load=$(INSTALL_PASS_DIR)/icp_convert.so -icp_convert -o  lvi_mutated.bc lvi_discriminator.bc > lvi_icp_dump

some_statistics:
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -analyze_hotness --profile-summary-hot-count=271973 --profile-summary-cold-count=20277  -o  dummy.bc sample_profile_mutated.bc > bif.990000
	#$(OPT) --inline --profile-summary-cutoff-cold=990000 --profile-summary-cutoff-hot=990000 -pass-remarks=InlinerPass  -pass-remarks-output=remarks_d_inline_990000.kernel  -o inlinedd_990000.bc sample_profile_mutated.bc
	#$(OPT) --inline --profile-summary-cutoff-cold=999000 --profile-summary-cutoff-hot=999000 -pass-remarks=InlinerPass  -pass-remarks-output=remarks_d_inline_999000.kernel  -o inlinedd_999000.bc sample_profile_mutated.bc
	#$(OPT) --inline --profile-summary-cutoff-cold=999900 --profile-summary-cutoff-hot=999900 -pass-remarks=InlinerPass  -pass-remarks-output=remarks_d_inline_999900.kernel  -o inlinedd_999900.bc sample_profile_mutated.bc
	#$(OPT) --inline --profile-summary-cutoff-cold=999990 --profile-summary-cutoff-hot=999990 -pass-remarks=InlinerPass  -pass-remarks-output=remarks_d_inline_999990.kernel  -o inlinedd_999990.bc sample_profile_mutated.bc
	#$(OPT) --inline --profile-summary-cutoff-cold=999999 --profile-summary-cutoff-hot=999999 -pass-remarks=InlinerPass  -pass-remarks-output=remarks_d_inline_999999.kernel  -o inlinedd_999999.bc sample_profile_mutated.bc  
	#$(OPT) --inline --profile-summary-cutoff-cold=995000 --profile-summary-cutoff-hot=995000 -pass-remarks=InlinerPass  -pass-remarks-output=remarks_d_inline_995000.kernel  -o inlinedd_995000.bc sample_profile_mutated.bc
	$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so --profile-summary-hot-count=271973 --profile-summary-cold-count=20277 -analyze_hotness -o  res.bc inlinedd_995000.bc > surplus.995000
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so --profile-summary-hot-count=271973 --profile-summary-cold-count=20277 -analyze_hotness -o  res.bc inlinedd_990000.bc > surplus.990000
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so --profile-summary-hot-count=271973 --profile-summary-cold-count=20277 -analyze_hotness -o  res.bc inlinedd_999000.bc > surplus.999000
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so --profile-summary-hot-count=271973 --profile-summary-cold-count=20277 -analyze_hotness -o  res.bc inlinedd_999900.bc > surplus.999900
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so --profile-summary-hot-count=271973 --profile-summary-cold-count=20277 -analyze_hotness -o  res.bc inlinedd_999990.bc > surplus.999990
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so --profile-summary-hot-count=271973 --profile-summary-cold-count=20277 -analyze_hotness -o  res.bc inlinedd_999999.bc > surplus.999999


nt:
	#$(LLVM_EXTRACT) -S  --func=copy_process  -o extract_1.ll stackpr.bc
	$(LLVM_EXTRACT) -S  --func=inet_gro_receive --func=dev_gro_receive -o extract_2.ll  moko.bc
	
test_linear_icp:
	#$(OPT) --profile-sample-accurate  --static-func-full-module-prefix=false --static-func-strip-dirname-prefix=30 --sample-profile --sample-profile-file=/projects/icp_kernel/playground/metadata/5.1.0.workload/decoded.profdata.All  -pass-remarks=SampleProfileLoaderLegacyPass  -pass-remarks-missed=SampleProfileLoaderLegacyPass  -pass-remarks-output=remarks_sampleprofile.kernel  mutated.bc -o sample_profile.bc
	$(OPT) -load=$(INSTALL_PASS_DIR)/linearpromotion.so -disable-inlining -icp-samplepgo-linear -icp_linear -profile-summary-cutoff-hot=990000 --icp-max-prom=40  --icp-remaining-percent-threshold=0 --icp-total-percent-threshold=0  -pass-remarks=PGOIndirectCallPromotion   -pass-remarks-missed=PGOIndirectCallPromotion   -pass-remarks-output=remarks_icp.kernel  sample_profile.bc -o sample_profile_rez.bc

test_rez:
	$(LLVM_EXTRACT) -S  --func=do_syscall_64  -o extract_1.ll sample_profile_rez.bc
	$(LLVM_EXTRACT) -S  --func=do_syscall_64  -o extract_2.ll sample_profile.bc 

test_nonlinear_icp:
	$(OPT) -disable-inlining -icp-samplepgo --pgo-icall-prom -profile-summary-cutoff-hot=999999  --icp-max-prom=40  --icp-remaining-percent-threshold=0 --icp-total-percent-threshold=0  -pass-remarks=PGOIndirectCallPromotion   -pass-remarks-missed=PGOIndirectCallPromotion   -pass-remarks-output=remarks_nonicp.kernel  sample_profile.bc -o sample_profile_ofoc.bc	
	

test_icp:
	$(OPT) --profile-sample-accurate  --static-func-full-module-prefix=false --static-func-strip-dirname-prefix=30 --sample-profile --sample-profile-file=/projects/icp_kernel/playground/metadata/5.1.0.workload/decoded.profdata.All.test  -pass-remarks=SampleProfileLoaderLegacyPass  -pass-remarks-missed=SampleProfileLoaderLegacyPass  -pass-remarks-output=remarks_sampleprofile.kernel  mutated.bc -o sample_profile.bc
	$(OPT) -disable-inlining -icp-samplepgo --pgo-icall-prom --profile-summary-hot-count=0  --profile-summary-cold-count=0 --icp-max-prom=40  --icp-remaining-percent-threshold=0 --icp-total-percent-threshold=0  -pass-remarks=PGOIndirectCallPromotion   -pass-remarks-missed=PGOIndirectCallPromotion   -pass-remarks-output=remarks_icp.kernel  sample_profile.bc -o sample_profile_icp.bc
	#${OPT}  --inline --pgo-icall-prom --profile-summary-hot-count=0  --profile-summary-cold-count=0 --icp-max-prom=40  --icp-remaining-percent-threshold=0 --icp-total-percent-threshold=0    -pass-remarks-missed=PGOIndirectCallPromotion -pass-remarks=PGOIndirectCallPromotion  -pass-remarks-output=remarks_icp_1.kernel  sample_profile.bc -o sample_profile_icp_1.bc	
	$(LLVM_EXTRACT) -S --func=do_sys_poll --func=do_syscall_64 --func=__x64_sys_rt_sigaction --func=__x64_sys_select -o test_1.ll sample_profile_icp.bc
	#$(LLVM_EXTRACT) -S --func=do_sys_poll --func=do_syscall_64 --func=__x64_sys_rt_sigaction -o test_2.ll sample_profile_icp_1.bc
simple_tryout:
	$(OPT) -load=$(INSTALL_PASS_DIR)/icp_convert.so -icp_convert -o  mutated.bc discriminator.bc > icp_dump

new_tryout:
	$(OPT) -load=$(INSTALL_PASS_DIR)/inliner.so -hotness_inliner -o  sso.bc sample_profile_mutated.bc > icp_dump
	$(OPT) -load=$(INSTALL_PASS_DIR)/inliner.so -hotness_inliner --profile-summary-cutoff-hot=999000 -o  sso2.bc sample_profile_mutated.bc > icp_dump2
	$(OPT) -load=$(INSTALL_PASS_DIR)/inliner.so -hotness_inliner --profile-summary-cutoff-hot=999999 -o  sso3.bc sample_profile_mutated.bc > icp_dump3

new_extract:
	#$(LLVM_EXTRACT) -S  --func=page_remove_rmap --func=unmap_page_range -o extract_1.ll sample_profile_mutated.bc
	$(LLVM_EXTRACT) -S  --func=pipe_read -o extract_2.ll sso.bc
compute_hot_counts:
	#$(OPT)  -add-discriminators -o discriminator.bc bare_kernel.bc
	#$(OPT) -load=$(INSTALL_PASS_DIR)/icp_convert.so -icp_convert -o  mutated.bc discriminator.bc > icp_dump
	#$(OPT) -disable-inlining --pgo-icall-prom --profile-summary-cutoff-hot=999999 --icp-max-prom=7  --icp-remaining-percent-threshold=10 --icp-total-percent-threshold=2 --profile-sample-accurate  --static-func-full-module-prefix=false --static-func-strip-dirname-prefix=30 --sample-profile --sample-profile-file=/projects/icp_kernel/playground/metadata/5.1.0.workload/decoded.profdata.All  -pass-remarks-missed=SampleProfileLoaderLegacyPass -pass-remarks=SampleProfileLoaderLegacyPass  -pass-remarks-output=remarks_sampleprofile.kernel  mutated.bc -o sample_profile_mutated.bc
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -analyze_hotness -profile-summary-cutoff-hot=950000  --profile-summary-cold-count=0 -o  dummy.bc sample_profile_mutated.bc > sum.950000
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -analyze_hotness -profile-summary-cutoff-hot=980000  --profile-summary-cold-count=0 -o  dummy.bc sample_profile_mutated.bc > sum.980000
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -analyze_hotness -profile-summary-cutoff-hot=990000  --profile-summary-cold-count=0 -o  dummy.bc sample_profile_mutated.bc > sum.990000
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -analyze_hotness -profile-summary-cutoff-hot=999000  --profile-summary-cold-count=0 -o  dummy.bc sample_profile_mutated.bc > sum.999000
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -analyze_hotness -profile-summary-cutoff-hot=999900  --profile-summary-cold-count=0 -o  dummy.bc sample_profile_mutated.bc > sum.999900
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -analyze_hotness -profile-summary-cutoff-hot=999990  --profile-summary-cold-count=0 -o  dummy.bc sample_profile_mutated.bc > sum.999990
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -analyze_hotness -profile-summary-cutoff-hot=999999  --profile-summary-cold-count=0 -o  dummy.bc sample_profile_mutated.bc > sum.999999
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -analyze_hotness --profile-summary-hot-count=0  --profile-summary-cold-count=0 -o  dummy.bc sample_profile_mutated.bc > sum.full
	#$(OPT) --inline --profile-summary-cutoff-cold=950000 --profile-summary-cutoff-hot=950000 -inlinecold-threshold=0 -hot-callsite-threshold=200000 -pass-remarks=InlinerPass  -pass-remarks-output=remarks_inline_950000.kernel  -o inlined_950000.bc sample_profile_mutated.bc
	#$(OPT) --inline --profile-summary-cutoff-cold=980000 --profile-summary-cutoff-hot=980000 -inlinecold-threshold=0 -hot-callsite-threshold=200000 -pass-remarks=InlinerPass  -pass-remarks-output=remarks_inline_980000.kernel  -o inlined_980000.bc sample_profile_mutated.bc
	#$(OPT) --inline --profile-summary-cutoff-cold=990000 --profile-summary-cutoff-hot=990000 -inlinecold-threshold=0 -hot-callsite-threshold=200000 -pass-remarks=InlinerPass  -pass-remarks-output=remarks_inline_990000.kernel  -o inlined_990000.bc sample_profile_mutated.bc
	#$(OPT) --inline --profile-summary-cutoff-cold=990000 --profile-summary-cutoff-hot=990000 -inlinecold-threshold=0 -hot-callsite-threshold=400000 -pass-remarks=InlinerPass  -pass-remarks-output=remarks_inline_990000_bigtresh.kernel  -o inlined_990000_bigtresh.bc sample_profile_mutated.bc
	#$(OPT) --inline --profile-summary-cutoff-cold=999000 --profile-summary-cutoff-hot=999000 -inlinecold-threshold=0 -hot-callsite-threshold=200000 -pass-remarks=InlinerPass  -pass-remarks-output=remarks_inline_999000.kernel  -o inlined_999000.bc sample_profile_mutated.bc
	#$(OPT) --inline --profile-summary-cutoff-cold=999900 --profile-summary-cutoff-hot=999900 -inlinecold-threshold=0 -hot-callsite-threshold=200000 -pass-remarks=InlinerPass  -pass-remarks-output=remarks_inline_999900.kernel  -o inlined_999900.bc sample_profile_mutated.bc
	#$(OPT) --inline --profile-summary-cutoff-cold=999990 --profile-summary-cutoff-hot=999990 -inlinecold-threshold=0 -hot-callsite-threshold=200000 -pass-remarks=InlinerPass  -pass-remarks-output=remarks_inline_999990.kernel  -o inlined_999990.bc sample_profile_mutated.bc
	$(OPT) --inline --inliner-function-import-stats=verbose --profile-summary-cutoff-cold=999999 --profile-summary-cutoff-hot=999999 -inlinecold-threshold=0 -hot-callsite-threshold=200000 -pass-remarks=InlinerPass  -pass-remarks-output=remarks_inline_99999.kernel  -o inlined_99999.bc sample_profile_mutated.bc
	#$(OPT) --inline --profile-summary-hot-count=0  --profile-summary-cold-count=0  -inlinecold-threshold=0 -hot-callsite-threshold=200000 -pass-remarks=InlinerPass  -pass-remarks-output=remarks_inline_full.kernel  -o inlined_full.bc sample_profile_mutated.bc

more_analysis:
	$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -analyze_hotness -profile-summary-cutoff-hot=750000  --profile-summary-cold-count=0 -o  dummy.bc sample_profile_mutated.bc > sumer.750000
	$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -analyze_hotness -profile-summary-cutoff-hot=500000  --profile-summary-cold-count=0 -o  dummy.bc sample_profile_mutated.bc > sumer.500000

compute_hot_counts_2:
	#$(OPT)  -add-discriminators -o discriminator.bc bare_kernel.bc
	$(OPT) -load=$(INSTALL_PASS_DIR)/icp_convert.so -icp_convert -o  mutated.bc discriminator.bc > icp_dump
	$(OPT) -disable-inlining --profile-sample-accurate --static-func-full-module-prefix=false --static-func-strip-dirname-prefix=30 --sample-profile --sample-profile-file=/projects/icp_kernel/playground/metadata/5.1.0.workload/decoded.profdata.All  -pass-remarks-missed=SampleProfileLoaderLegacyPass -pass-remarks=SampleProfileLoaderLegacyPass  -pass-remarks-output=remarks_sampleprofile.kernel  mutated.bc -o mutated_withprof.bc
	$(OPT) -disable-inlining -icp-samplepgo --pgo-icall-prom --profile-summary-cutoff-hot=999999 --icp-max-prom=7  --icp-remaining-percent-threshold=1 --icp-total-percent-threshold=1   -pass-remarks-missed=PGOIndirectCallPromotion -pass-remarks=PGOIndirectCallPromotion  -pass-remarks-output=remarks_pgo.kernel  mutated_withprof.bc -o sample_profile_mutated.bc
	$(LLVM_EXTRACT) -S --func=do_sys_poll --func=vfs_write --func=do_syscall_64 --func=core_sys_select -o test_1.ll sample_profile_mutated.bc
	$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -analyze_hotness -profile-summary-cutoff-hot=999000  --profile-summary-cold-count=0 -o  dummy.bc sample_profile_mutated.bc > sumer.999000
	$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -analyze_hotness -profile-summary-cutoff-hot=999900  --profile-summary-cold-count=0 -o  dummy.bc sample_profile_mutated.bc > sumer.999900
	$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -analyze_hotness -profile-summary-cutoff-hot=999990  --profile-summary-cold-count=0 -o  dummy.bc sample_profile_mutated.bc > sumer.999990
	$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -analyze_hotness -profile-summary-cutoff-hot=999999  --profile-summary-cold-count=0 -o  dummy.bc sample_profile_mutated.bc > sumer.999999
	$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -analyze_hotness -profile-summary-cutoff-hot=950000  --profile-summary-cold-count=0 -o  dummy.bc sample_profile_mutated.bc > sumer.950000
	$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -analyze_hotness -profile-summary-cutoff-hot=980000  --profile-summary-cold-count=0 -o  dummy.bc sample_profile_mutated.bc > sumer.980000
	$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -analyze_hotness -profile-summary-cutoff-hot=990000  --profile-summary-cold-count=0 -o  dummy.bc sample_profile_mutated.bc > sumer.990000
	$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -analyze_hotness --profile-summary-hot-count=0  --profile-summary-cold-count=0 -o  dummy.bc sample_profile_mutated.bc > sumer.full
	$(OPT) --inline --profile-summary-cutoff-cold=950000 --profile-summary-cutoff-hot=950000 -pass-remarks=InlinerPass  -pass-remarks-output=remarks_inline_950000.kernel  -o inlined_950000.bc sample_profile_mutated.bc
	$(OPT) --inline --profile-summary-cutoff-cold=980000 --profile-summary-cutoff-hot=980000  -pass-remarks=InlinerPass  -pass-remarks-output=remarks_inline_980000.kernel  -o inlined_980000.bc sample_profile_mutated.bc
	$(OPT) --inline --profile-summary-cutoff-cold=990000 --profile-summary-cutoff-hot=990000 -pass-remarks=InlinerPass  -pass-remarks-output=remarks_inline_990000.kernel  -o inlined_990000.bc sample_profile_mutated.bc
	$(OPT) --inline --profile-summary-cutoff-cold=990000 --profile-summary-cutoff-hot=990000  -pass-remarks=InlinerPass  -pass-remarks-output=remarks_inline_990000_bigtresh.kernel  -o inlined_990000_bigtresh.bc sample_profile_mutated.bc
	$(OPT) --inline --profile-summary-cutoff-cold=999000 --profile-summary-cutoff-hot=999000  -pass-remarks=InlinerPass  -pass-remarks-output=remarks_inline_999000.kernel  -o inlined_999000.bc sample_profile_mutated.bc
	$(OPT) --inline --profile-summary-cutoff-cold=999900 --profile-summary-cutoff-hot=999900 -pass-remarks=InlinerPass  -pass-remarks-output=remarks_inline_999900.kernel  -o inlined_999900.bc sample_profile_mutated.bc
	$(OPT) --inline --profile-summary-cutoff-cold=999990 --profile-summary-cutoff-hot=999990  -pass-remarks=InlinerPass  -pass-remarks-output=remarks_inline_999990.kernel  -o inlined_999990.bc sample_profile_mutated.bc
	$(OPT) --inline --profile-summary-cutoff-cold=999999 --profile-summary-cutoff-hot=999999  -pass-remarks=InlinerPass  -pass-remarks-output=remarks_inline_999999.kernel  -o inlined_999999.bc sample_profile_mutated.bc
	$(OPT) --inline --profile-summary-hot-count=0  --profile-summary-cold-count=0  -pass-remarks=InlinerPass  -pass-remarks-output=remarks_inline_full.kernel  -o inlined_full.bc sample_profile_mutated.bc

print_scc:
	$(OPT) -print-cfg-sccs  sample_profile_mutated.bc 

apply_hot_counts:
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -analyze_hotness -profile-summary-cutoff-hot=500000  --profile-summary-cold-count=0 -o  dummy.bc sample_profile_mutated.bc > sum.500000
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -analyze_hotness -profile-summary-cutoff-hot=750000  --profile-summary-cold-count=0 -o  dummy.bc sample_profile_mutated.bc > sum.750000
	#$(OPT) --inline --profile-summary-cutoff-cold=500000 --profile-summary-cutoff-hot=500000 -inlinecold-threshold=0 -hot-callsite-threshold=200000 -pass-remarks=InlinerPass  -pass-remarks-output=remarks_inline_500000.kernel  -o inlined_500000.bc sample_profile_mutated.bc
	#$(OPT) --inline --profile-summary-cutoff-cold=750000 --profile-summary-cutoff-hot=750000 -inlinecold-threshold=0 -hot-callsite-threshold=200000 -pass-remarks=InlinerPass  -pass-remarks-output=remarks_inline_750000.kernel  -o inlined_750000.bc sample_profile_mutated.bc
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -analyze_hotness --profile-summary-hot-count=2607511 --profile-summary-cold-count=0 -o  dummy.bc inlined_950000.bc > rez.950000
	$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -analyze_hotness --profile-summary-hot-count=271973 --profile-summary-cold-count=0 -o  dummy.bc inlined_990000.bc > rez.990000
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -analyze_hotness --profile-summary-hot-count=271973 --profile-summary-cold-count=0 -o  dummy.bc inlined_990000_bigtresh.bc > rez.990000.bt
	$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -analyze_hotness --profile-summary-hot-count=20277 --profile-summary-cold-count=0 -o  dummy.bc inlined_999000.bc > rez.999000
	$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -analyze_hotness --profile-summary-hot-count=1299 --profile-summary-cold-count=0 -o  dummy.bc inlined_999900.bc > rez.999900
	$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -analyze_hotness --profile-summary-hot-count=126 --profile-summary-cold-count=0 -o  dummy.bc inlined_999990.bc > rez.999990
	$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -analyze_hotness --profile-summary-hot-count=23 --profile-summary-cold-count=0 -o  dummy.bc inlined_999999.bc > rez.999999
	$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -analyze_hotness --profile-summary-hot-count=0 --profile-summary-cold-count=0 -o  dummy.bc inlined_full.bc > rez.full
	

check_image:
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -analyze_hotness -o  dummy.bc discriminator.bc > analyze_dump_1
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -analyze_hotness -o  dummy.bc sample_profile.bc > analyze_dump_2
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -analyze_hotness -o  dummy.bc sample_profile_accurate.bc > analyze_dump_3 
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -profile-summary-cutoff-cold=990000 -analyze_hotness -o  dummy.bc sample_profile_accurate.bc > analyze_dump_4
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -profile-summary-cutoff-hot=999900 -analyze_hotness -o  dummy.bc sample_profile_accurate.bc > analyze_dump_5
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -profile-summary-cutoff-cold=990000  -analyze_hotness -o  dummy.bc sample_profile.bc  > analyze_dump_6
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -profile-summary-cutoff-hot=999900 -analyze_hotness -o  dummy.bc sample_profile.bc  > analyze_dump_9
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so --profile-summary-cold-count=0 --profile-summary-hot-count=0  -analyze_hotness -o  dummy.bc sample_profile.bc  > analyze_dump_7
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so --profile-summary-cold-count=0 --profile-summary-hot-count=0  -analyze_hotness -o  dummy.bc sample_profile_accurate.bc  > analyze_dump_8
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -analyze_hotness -o  dummy.bc inlined_kernel.bc > analyze_dump_11
	$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -profile-summary-cutoff-hot=999000 -analyze_hotness -o  dummy.bc sample_profile_accurate.bc  > analyze_dump_12
	$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -profile-summary-cutoff-hot=999900 -analyze_hotness -o  dummy.bc sample_profile_accurate.bc  > analyze_dump_13
	$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -profile-summary-cutoff-hot=999990 -analyze_hotness -o  dummy.bc sample_profile_accurate.bc  > analyze_dump_14
	$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -profile-summary-cutoff-hot=999999 -analyze_hotness -o  dummy.bc sample_profile_accurate.bc  > analyze_dump_15
create_inline:
	$(OPT) --inline --profile-summary-cutoff-cold=990000 --profile-summary-cutoff-hot=990000 -pass-remarks=InlinerPass  -pass-remarks-output=remarks_inline_icp_full_momo_lvifull.kernel  -o inlined_kernel.bc sample_profile_accurate.bc
	$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -analyze_hotness -o  dummy.bc inlined_kernel.bc > analyze_dump_16
	$(OPT) --inline --profile-summary-cutoff-cold=999000 --profile-summary-cutoff-hot=999000 -pass-remarks=InlinerPass  -pass-remarks-output=remarks_inline_icp_full_momo_lvifull.kernel  -o inlined_kernel.bc sample_profile_accurate.bc
	$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -analyze_hotness -o  dummy.bc inlined_kernel.bc > analyze_dump_17
	$(OPT) --inline --profile-summary-cutoff-cold=999900 --profile-summary-cutoff-hot=999900 -pass-remarks=InlinerPass  -pass-remarks-output=remarks_inline_icp_full_momo_lvifull.kernel  -o inlined_kernel.bc sample_profile_accurate.bc
	$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -analyze_hotness -o  dummy.bc inlined_kernel.bc > analyze_dump_18
	$(OPT) --inline --profile-summary-cutoff-cold=999990 --profile-summary-cutoff-hot=999990 -pass-remarks=InlinerPass  -pass-remarks-output=remarks_inline_icp_full_momo_lvifull.kernel  -o inlined_kernel.bc sample_profile_accurate.bc
	$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -analyze_hotness -o  dummy.bc inlined_kernel.bc > analyze_dump_19
	$(OPT) --inline --profile-summary-cutoff-cold=999999 --profile-summary-cutoff-hot=999999 -pass-remarks=InlinerPass  -pass-remarks-output=remarks_inline_icp_full_momo_lvifull.kernel  -o inlined_kernel.bc sample_profile_accurate.bc
	$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -analyze_hotness -o  dummy.bc inlined_kernel.bc > analyze_dump_20

check_restricted:
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so  -profile-summary-cutoff-cold=990000 -profile-summary-cutoff-hot=990000 -analyze_hotness -o  dummy.bc sample_profile_accurate.bc  > analyze_dump_21
	#$(OPT) --inline --profile-summary-cutoff-cold=990000 --profile-summary-cutoff-hot=990000 -hot-callsite-threshold=200000 -pass-remarks=InlinerPass  -pass-remarks-output=remarks_inline_icp_full_momo_lvifull.kernel  -o inlined_kernel_2.bc sample_profile_accurate.bc
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so -analyze_hotness -o  dummy.bc inlined_kernel.bc > analyze_dump_22
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so  --profile-summary-cutoff-cold=950000 --profile-summary-cutoff-hot=950000 -analyze_hotness -o  dummy.bc sample_profile_accurate.bc  > analyze_dump_0b
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so  --profile-summary-cutoff-cold=900000 --profile-summary-cutoff-hot=900000 -analyze_hotness -o  dummy.bc sample_profile_accurate.bc  > analyze_dump_1b
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so  --profile-summary-cutoff-cold=850000 --profile-summary-cutoff-hot=850000 -analyze_hotness -o  dummy.bc sample_profile_accurate.bc  > analyze_dump_2b
	$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so  --profile-summary-cutoff-cold=500000 --profile-summary-cutoff-hot=500000 -analyze_hotness -o  dummy.bc sample_profile_accurate.bc  > analyze_dump_3b
	$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so  --profile-summary-cutoff-cold=960000 --profile-summary-cutoff-hot=960000 -analyze_hotness -o  dummy.bc sample_profile_accurate.bc  > analyze_dump_4b
	$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so  --profile-summary-cutoff-cold=960000 --profile-summary-cutoff-hot=960000 -analyze_hotness -o  dummy.bc sample_profile_accurate.bc  > analyze_dump_5b
	$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so  --profile-summary-cutoff-cold=970000 --profile-summary-cutoff-hot=970000 -analyze_hotness -o  dummy.bc sample_profile_accurate.bc  > analyze_dump_6b
	$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so  --profile-summary-cutoff-cold=980000 --profile-summary-cutoff-hot=980000 -analyze_hotness -o  dummy.bc sample_profile_accurate.bc  > analyze_dump_7b
	#$(OPT) --inline --profile-summary-cutoff-cold=990000 --profile-summary-cutoff-hot=990000 -hot-callsite-threshold=200000 -pass-remarks=InlinerPass  -pass-remarks-output=remarks_inline_icp_full_momo_lvifull_1.kernel  -o inlined_kernel_1.bc inlined_kernel.bc
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so  --profile-summary-cutoff-cold=990000 --profile-summary-cutoff-hot=990000 -analyze_hotness -o  dummy.bc inlined_kernel_1.bc > analyze_dump_26

check_with_indirect:
	#$(OPT) -disable-inlining --pgo-icall-prom --profile-summary-cutoff-hot=999999 --icp-remaining-percent-threshold=0 --icp-total-percent-threshold=0   -pass-remarks-missed=PGOIndirectCallPromotion -pass-remarks=PGOIndirectCallPromotion  -pass-remarks-output=remarks_icp.kernel  sample_profile_accurate.bc -o icp.bc
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so  -profile-summary-cutoff-cold=990000 -profile-summary-cutoff-hot=990000 -analyze_hotness -o  dummy.bc icp.bc  > analyze_dumpy_2a
	#$(OPT) --inline --profile-summary-cutoff-cold=990000 --profile-summary-cutoff-hot=990000 -hot-callsite-threshold=200000 -pass-remarks=InlinerPass  -pass-remarks-output=remarks_inline_icp_full_momo_lvifull_1.kernel  -o inlined_kernel_1.bc  icp.bc
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so  -profile-summary-cutoff-cold=990000 -profile-summary-cutoff-hot=990000 -analyze_hotness -o  dummy.bc inlined_kernel_1.bc  > analyze_dumpy_2
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so  --profile-summary-cutoff-cold=999000 --profile-summary-cutoff-hot=999000 -analyze_hotness -o  dummy.bc icp.bc  > analyze_dumpy_3a
	#$(OPT) --inline --profile-summary-cutoff-cold=999000 --profile-summary-cutoff-hot=999000 -hot-callsite-threshold=200000 -pass-remarks=InlinerPass  -pass-remarks-output=remarks_inline_icp_full_momo_lvifull_1.kernel  -o inlined_kernel_2.bc  icp.bc
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so  -profile-summary-cutoff-cold=999000 -profile-summary-cutoff-hot=999000 -analyze_hotness -o  dummy.bc inlined_kernel_2.bc  > analyze_dumpy_3
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so  --profile-summary-cutoff-cold=999900 --profile-summary-cutoff-hot=999900 -analyze_hotness -o  dummy.bc icp.bc  > analyze_dumpy_4a
	#$(OPT) --inline --profile-summary-cutoff-cold=999900 --profile-summary-cutoff-hot=999900 -hot-callsite-threshold=200000 -pass-remarks=InlinerPass  -pass-remarks-output=remarks_inline_icp_full_momo_lvifull_1.kernel  -o inlined_kernel_3.bc  icp.bc
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so  -profile-summary-cutoff-cold=999900 -profile-summary-cutoff-hot=999900 -analyze_hotness -o  dummy.bc inlined_kernel_3.bc  > analyze_dumpy_4
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so  --profile-summary-cutoff-cold=999990 --profile-summary-cutoff-hot=999990 -analyze_hotness -o  dummy.bc icp.bc  > analyze_dumpy_5a
	#$(OPT) --inline --profile-summary-cutoff-cold=999990 --profile-summary-cutoff-hot=999990 -hot-callsite-threshold=200000 -pass-remarks=InlinerPass  -pass-remarks-output=remarks_inline_icp_full_momo_lvifull_1.kernel  -o inlined_kernel_4.bc  icp.bc
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so  -profile-summary-cutoff-cold=999990 -profile-summary-cutoff-hot=999990 -analyze_hotness -o  dummy.bc inlined_kernel_4.bc  > analyze_dumpy_5
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so  --profile-summary-cutoff-cold=999999 --profile-summary-cutoff-hot=999999 -analyze_hotness -o  dummy.bc icp.bc  > analyze_dumpy_11a
	#$(OPT) --inline --profile-summary-cutoff-cold=999999 --profile-summary-cutoff-hot=999999 -hot-callsite-threshold=600000 -pass-remarks=InlinerPass  -pass-remarks-output=remarks_inline_icp_full_momo_lvifull_1.kernel  -o inlined_kernel_5.bc  icp.bc
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so  -profile-summary-cutoff-cold=999999 -profile-summary-cutoff-hot=999999 -analyze_hotness -o  dummy.bc inlined_kernel_5.bc  > analyze_dumpy_11
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so  --profile-summary-cold-count=0 --profile-summary-hot-count=0 -analyze_hotness -o  dummy.bc icp.bc  > analyze_dumpy_7a
	#$(OPT) --inline  --profile-summary-cold-count=0 --profile-summary-hot-count=0 -hot-callsite-threshold=200000 -pass-remarks=InlinerPass  -pass-remarks-output=remarks_inline_icp_full_momo_lvifull_1.kernel  -o inlined_kernel_6.bc  icp.bc
	#$(OPT) -load=$(INSTALL_PASS_DIR)/analyze_hotness.so   --profile-summary-cold-count=0 --profile-summary-hot-count=0 -analyze_hotness -o  dummy.bc inlined_kernel_6.bc  > analyze_dumpy_7
	$(LLVM_EXTRACT) -S --func=tcp_recvmsg -o test_1.ll inlined_kernel_1.bc
# skb_copy_datagram_iter
test_discriminators:
	$(CXX) -g -flto -femit_discriminators -c -o test.bc test_disc.c
	$(OPT) -add-discriminators -o test_d.bc test.bc
	$(LLVM_DIS) test.bc
	$(LLVM_DIS) test_d.bc
test_stack_protector:
	$(CXX) -flto  -fstack-protector-all -fstack-protector-strong -c -o test.bc test_disc.c
	$(CXX) -o test.bc2 test_disc.c
	$(LLVM_DIS) test.bc

test_cf_protector:
	$(CXX)-cl -Wmsvc-not-found /guard:cf -o test.bc test_disc.c

test_lvi:
	$(CXX) -flto -c -mlvi-cfi -o test.bc test.c
	$(CXX) -flto -mretpoline -o test_11.o test.c test1.c
	$(LLC) test.bc -o tests.o
	$(CXX) -mlvi-cfi -O2 -o test.o test.c test1.c
	$(LLVM_DIS) test.bc

test_lllc:
	$(LLC) -help-list-hidden


blacklist_lvi:
	$(OPT) -load=$(INSTALL_PASS_DIR)/bl_lvi.so -blacklist_lvi -o  dummy.bc lvi.bc > blacklist_dump
	$(LLVM_EXTRACT) -S --func=get_next_block --func=__startup_64  -o test.ll dummy.bc



test_2cfi:
	$(CXX) -flto  -c -o test.bc test_disc.c
	$(CXX)  -fstack-protector-all -o test test.bc

test_cfi_dso:
	$(OPT) --O3 --cross-dso-cfi  -o mainline_o.bc mainline.bc

extract_dso:
	$(LLVM_EXTRACT) -S --func=do_syscall_64  -o test_1.ll mainline_o.bc
	$(LLVM_EXTRACT) -S --func=do_syscall_64  -o test_0.ll mainline.bc

test_add_discriminators:
	${OPT}  -add-discriminators -o bare_kernel_discriminators.bc bare_kernel.bc

test_some_backport:
	$(OPT) -load=$(INSTALL_PASS_DIR)/icp_convert.so -icp_convert -o  dummy.bc discriminator.bc > backport_icp_dump

test_cfi_linux:
	${OPT}  -add-discriminators -o cfilinux_discriminators.bc cfilinux.bc
	$(OPT) -load=$(INSTALL_PASS_DIR)/icp_convert.so -icp_convert -o  dummy.bc cfilinux_discriminators.bc > cfi_icp_dump
test_application:
	$(OPT) -disable-inlining --icp-remaining-percent-threshold=20 --icp-total-percent-threshold=3 --profile-summary-cutoff-hot=990000 --static-func-full-module-prefix=false --static-func-strip-dirname-prefix=30 --sample-profile --sample-profile-file=/projects/icp_kernel/playground/metadata/5.4.0/decoded.profdata.Indirect --pgo-icall-prom  -pass-remarks-missed=PGOIndirectCallPromotion -pass-remarks=PGOIndirectCallPromotion  -pass-remarks-output=remarks_all.kernel built-in.a -o result.a

test_icp_conversion:
	$(OPT) -load=$(INSTALL_PASS_DIR)/icp_convert.so -icp_convert -o  dummy.bc bare_kernel_discriminators.bc > icp_dump

test_dominator:
	$(OPT)  --domtree  -o  dom_tree.bc bare_kernel.bc > icp_dump

test_fixup_cfg:
	$(OPT) -load=$(INSTALL_PASS_DIR)/fixup_cfg.so -fixupcfg -o  fixup.bc  converted_bare_kernel.bc > fixup_dump

test_apply_conversion:
	$(OPT) -disable-inlining --icp-remaining-percent-threshold=20 --icp-total-percent-threshold=3 --profile-summary-cutoff-hot=990000 --static-func-full-module-prefix=false --static-func-strip-dirname-prefix=30 --sample-profile --sample-profile-file=/projects/icp_kernel/playground/metadata/5.4.0/decoded.profdata.Indirect.workingset.2 --pgo-icall-prom  -pass-remarks-missed=PGOIndirectCallPromotion -pass-remarks=PGOIndirectCallPromotion  -pass-remarks-output=remarks.kernel bare_kernel_discriminators.bc -o converted_bare_kernel.bc

test_apply_conversion_fixed:
	$(OPT) -disable-inlining --icp-remaining-percent-threshold=20 --icp-total-percent-threshold=3 --profile-summary-cutoff-hot=990000 --static-func-full-module-prefix=false --static-func-strip-dirname-prefix=30 --sample-profile --sample-profile-file=/projects/icp_kernel/playground/metadata/5.4.0/decoded.profdata.Indirect.fixup3 --pgo-icall-prom  -pass-remarks-missed=PGOIndirectCallPromotion -pass-remarks=PGOIndirectCallPromotion  -pass-remarks-output=remarks_fixup.kernel bare_kernel_discriminators.bc -o converted_bare_kernel.bc

test_new_lvi:
	 $(OPT) -disable-inlining --static-func-full-module-prefix=false --static-func-strip-dirname-prefix=30 --sample-profile --sample-profile-file=/projects/icp_kernel/playground/metadata/5.1.0.lvi/decoded.profdata.All  -pass-remarks-missed=SampleProfileLoaderLegacyPass -pass-remarks=SampleProfileLoaderLegacyPass  -pass-remarks-output=remarks_sampleprofile_lvi_ret.kernel  discriminator_lvi.bc -o sample_profile.bc
	$(OPT) --inline  -profile-summary-cutoff-hot=999999   -pass-remarks=InlinerPass  -pass-remarks-output=remarks_999999.kernel  -o inlined_kernel.bc sample_profile.bc
	$(OPT) --inline  -profile-summary-cutoff-hot=950000   -pass-remarks=InlinerPass  -pass-remarks-output=remarks_950000.kernel  -o inlined_kernel.bc sample_profile.bc
	$(OPT) --inline  -profile-summary-cutoff-hot=990000   -pass-remarks=InlinerPass  -pass-remarks-output=remarks_990000.kernel  -o inlined_kernel.bc sample_profile.bc
	$(OPT) --inline  -profile-summary-cutoff-hot=999900   -pass-remarks=InlinerPass  -pass-remarks-output=remarks_999900.kernel  -o inlined_kernel.bc sample_profile.bc
	$(OPT) -disable-inlining --static-func-full-module-prefix=false --static-func-strip-dirname-prefix=30 --sample-profile --sample-profile-file=/projects/icp_kernel/playground/metadata/5.1.0.lvi/decoded.profdata.All -profile-summary-cutoff-hot=999999 -pass-remarks-missed=SampleProfileLoaderLegacyPass -pass-remarks=SampleProfileLoaderLegacyPass  -pass-remarks-output=remarks_sampleprofile_lvi_ret.kernel  discriminator_lvi.bc -o sample_profile.bc
	$(OPT) --inline  -pass-remarks=InlinerPass  -pass-remarks-output=remarks_alternating_999999.kernel  -o inlined_kernel.bc sample_profile.bc
	$(OPT) -disable-inlining --static-func-full-module-prefix=false --static-func-strip-dirname-prefix=30 --sample-profile --sample-profile-file=/projects/icp_kernel/playground/metadata/5.1.0.lvi/decoded.profdata.All -profile-summary-cutoff-hot=999900 -pass-remarks-missed=SampleProfileLoaderLegacyPass -pass-remarks=SampleProfileLoaderLegacyPass  -pass-remarks-output=remarks_sampleprofile_lvi_ret.kernel  discriminator_lvi.bc -o sample_profile.bc
	$(OPT) --inline  -pass-remarks=InlinerPass  -pass-remarks-output=remarks_alternating_999900.kernel  -o inlined_kernel.bc sample_profile.bc
	$(OPT) -disable-inlining --static-func-full-module-prefix=false --static-func-strip-dirname-prefix=30 --sample-profile --sample-profile-file=/projects/icp_kernel/playground/metadata/5.1.0.lvi/decoded.profdata.All -profile-summary-cutoff-hot=990000 -pass-remarks-missed=SampleProfileLoaderLegacyPass -pass-remarks=SampleProfileLoaderLegacyPass  -pass-remarks-output=remarks_sampleprofile_lvi_ret.kernel  discriminator_lvi.bc -o sample_profile.bc
	$(OPT) --inline  -pass-remarks=InlinerPass  -pass-remarks-output=remarks_alternating_990000.kernel  -o inlined_kernel.bc sample_profile.bc



get_working_sets:
	$(OPT) -disable-inlining --icp-remaining-percent-threshold=20 --icp-total-percent-threshold=3 --profile-summary-cutoff-hot=990000 --static-func-full-module-prefix=false --static-func-strip-dirname-prefix=30 --sample-profile --sample-profile-file=/projects/icp_kernel/playground/metadata/5.4.0/decoded.profdata.Indirect --pgo-icall-prom  -pass-remarks-missed=PGOIndirectCallPromotion -pass-remarks=PGOIndirectCallPromotion  -pass-remarks-output=remarks_all.kernel bare_kernel_discriminators.bc -o converted_bare_kernel.bc
	$(OPT) -disable-inlining --icp-remaining-percent-threshold=20 --icp-total-percent-threshold=3 --profile-summary-cutoff-hot=990000 --static-func-full-module-prefix=false --static-func-strip-dirname-prefix=30 --sample-profile --sample-profile-file=/projects/icp_kernel/playground/metadata/5.4.0/decoded.profdata.Indirect.workingset --pgo-icall-prom  -pass-remarks-missed=PGOIndirectCallPromotion -pass-remarks=PGOIndirectCallPromotion  -pass-remarks-output=remarks_workingset1.kernel bare_kernel_discriminators.bc -o converted_bare_kernel.bc
	$(OPT) -disable-inlining --icp-remaining-percent-threshold=20 --icp-total-percent-threshold=3 --profile-summary-cutoff-hot=990000 --static-func-full-module-prefix=false --static-func-strip-dirname-prefix=30 --sample-profile --sample-profile-file=/projects/icp_kernel/playground/metadata/5.4.0/decoded.profdata.Indirect.workingset.2 --pgo-icall-prom  -pass-remarks-missed=PGOIndirectCallPromotion -pass-remarks=PGOIndirectCallPromotion  -pass-remarks-output=remarks_workingset2.kernel bare_kernel_discriminators.bc -o converted_bare_kernel.bc
	$(OPT) -disable-inlining --icp-remaining-percent-threshold=20 --icp-total-percent-threshold=3 --profile-summary-cutoff-hot=990000 --static-func-full-module-prefix=false --static-func-strip-dirname-prefix=30 --sample-profile --sample-profile-file=/projects/icp_kernel/playground/metadata/5.4.0/decoded.profdata.Indirect.workingset.3 --pgo-icall-prom  -pass-remarks-missed=PGOIndirectCallPromotion -pass-remarks=PGOIndirectCallPromotion  -pass-remarks-output=remarks_workingset3.kernel bare_kernel_discriminators.bc -o converted_bare_kernel.bc

get_working_set_2:
	$(OPT) -disable-inlining --icp-remaining-percent-threshold=20 --icp-total-percent-threshold=3 --profile-summary-cutoff-hot=990000 --static-func-full-module-prefix=false --static-func-strip-dirname-prefix=30 --sample-profile --sample-profile-file=/projects/icp_kernel/playground/metadata/5.4.0/decoded.profdata.Indirect.splitfault.workingset.1 --pgo-icall-prom  -pass-remarks-missed=PGOIndirectCallPromotion -pass-remarks=PGOIndirectCallPromotion  -pass-remarks-output=remarks_splitfault_workingset1.kernel bare_kernel_discriminators.bc -o converted_bare_kernel_fault.bc
	#$(OPT) -disable-inlining --icp-remaining-percent-threshold=20 --icp-total-percent-threshold=3 --profile-summary-cutoff-hot=990000 --static-func-full-module-prefix=false --static-func-strip-dirname-prefix=30 --sample-profile --sample-profile-file=/projects/icp_kernel/playground/metadata/5.4.0/decoded.profdata.Indirect.splitfault.workingset.2 --pgo-icall-prom  -pass-remarks-missed=PGOIndirectCallPromotion -pass-remarks=PGOIndirectCallPromotion  -pass-remarks-output=remarks_splitfault_workingset2.kernel bare_kernel_discriminators.bc -o converted_bare_kernel.bc

get_working_set_3:
	$(OPT) -disable-inlining --icp-remaining-percent-threshold=20 --icp-total-percent-threshold=3 --profile-summary-cutoff-hot=990000 --static-func-full-module-prefix=false --static-func-strip-dirname-prefix=30 --sample-profile --sample-profile-file=/projects/icp_kernel/playground/metadata/5.4.0/decoded.profdata.Indirect.splitfault2.workingset.1 --pgo-icall-prom  -pass-remarks-missed=PGOIndirectCallPromotion -pass-remarks=PGOIndirectCallPromotion  -pass-remarks-output=remarks_splitfault2_workingset1.kernel bare_kernel_discriminators.bc -o converted_bare_kernel.bc

get_working_set_4:
	$(OPT) -disable-inlining --icp-remaining-percent-threshold=20 --icp-total-percent-threshold=3 --profile-summary-cutoff-hot=990000 --static-func-full-module-prefix=false --static-func-strip-dirname-prefix=30 --sample-profile --sample-profile-file=/projects/icp_kernel/playground/metadata/5.4.0/decoded.profdata.Indirect.splitfault2.workingset.2 --pgo-icall-prom  -pass-remarks-missed=PGOIndirectCallPromotion -pass-remarks=PGOIndirectCallPromotion  -pass-remarks-output=remarks_splitfault2_workingset2.kernel bare_kernel_discriminators.bc -o converted_bare_kernel.bc

get_working_set_5:
	$(OPT) -disable-inlining --icp-remaining-percent-threshold=20 --icp-total-percent-threshold=3 --profile-summary-cutoff-hot=990000 --static-func-full-module-prefix=false --static-func-strip-dirname-prefix=30 --sample-profile --sample-profile-file=/projects/icp_kernel/playground/metadata/5.4.0/decoded.profdata.Indirect.splitfault.workingset.1 --pgo-icall-prom  -pass-remarks-missed=PGOIndirectCallPromotion -pass-remarks=PGOIndirectCallPromotion  -pass-remarks-output=remarks_splitfault_workingset1.kernel bare_kernel_discriminators.bc -o converted_bare_kernel.bc
	$(OPT) -disable-inlining --icp-remaining-percent-threshold=20 --icp-total-percent-threshold=3 --profile-summary-cutoff-hot=990000 --static-func-full-module-prefix=false --static-func-strip-dirname-prefix=30 --sample-profile --sample-profile-file=/projects/icp_kernel/playground/metadata/5.4.0/decoded.profdata.Indirect.splitfault.workingset.1.fixed --pgo-icall-prom  -pass-remarks-missed=PGOIndirectCallPromotion -pass-remarks=PGOIndirectCallPromotion  -pass-remarks-output=remarks_splitfault_workingset1_fixed.kernel bare_kernel_discriminators.bc -o converted_bare_kernel.bc




test_extract:
	$(LLVM_EXTRACT) -S  --func=do_syscall_64 --func=ns_get_path --func=queued_spin_lock_slowpath --func=devtmpfsd --func=do_IRQ --func=handle_edge_irq --func=kthread -o test.ll bare_kernel_discriminators.bc
	$(LLVM_EXTRACT) -S  --func=do_syscall_64 --func=ns_get_path --func=queued_spin_lock_slowpath --func=devtmpfsd --func=do_IRQ --func=handle_edge_irq -func=kthread -o test_1.ll converted_bare_kernel.bc

test_extract_lvi:
	$(LLVM_EXTRACT) -S  --func=do_syscall_64 --func=ipv6_offload_init --func=lto_wrappedipv6_offload_init453 --func=ns_get_path --func=queued_spin_lock_slowpath --func=devtmpfsd --func=do_IRQ --func=handle_edge_irq --func=kthread -o test.ll lvi.bc



test_extract_1:
	$(LLVM_EXTRACT) -S  --func=security_socket_getpeersec_dgram -o test.ll bare_kernel_discriminators.bc
	$(LLVM_EXTRACT) -S  --func=security_socket_getpeersec_dgram -o test_1.ll converted_bare_kernel.bc
	$(LLVM_EXTRACT) -S  --func=security_socket_getpeersec_dgram -o test_2.ll discrim_backport.bc 
	$(LLVM_EXTRACT) -S  --func=security_socket_getpeersec_dgram -o test_3.ll intermediate_kernel_Phase2.bc

test_extract_2:
	$(LLVM_EXTRACT) -S  --func=set_page_dirty -o test.ll bare_kernel_discriminators.bc
	$(LLVM_EXTRACT) -S  --func=set_page_dirty -o test1.ll converted_bare_kernel_split_ws1.bc

test_extract_fault:
	$(LLVM_EXTRACT) -S  --func=try_to_release_page --func=generic_perform_write --func=set_page_dirty --func=__put_page --func=pagevec_lru_move_fn --func=truncate_cleanup_page --func=handle_mm_fault --func=security_sock_rcv_skb --func=security_inode_permission --func=security_file_permission --func=security_socket_sendmsg --func=security_socket_recvmsg -o test.ll bare_kernel_discriminators.bc
	$(LLVM_EXTRACT) -S  --func=try_to_release_page --func=generic_perform_write --func=set_page_dirty --func=__put_page --func=pagevec_lru_move_fn --func=truncate_cleanup_page --func=handle_mm_fault --func=security_sock_rcv_skb --func=security_inode_permission --func=security_file_permission --func=security_socket_sendmsg --func=security_socket_recvmsg -o test1.ll  converted_bare_kernel_fault.bc


test_sok:
	$(LLVM_EXTRACT) -S  --func=do_sys_poll -o test.ll bare_kernel.bc
test_sinking:
	$(OPT) --sink dummy.bc -o dummer.bc
	$(LLVM_EXTRACT) -S  --func=do_syscall_64 --func=ns_get_path -o test.ll dummer.bc
	$(LLVM_EXTRACT) -S  --func=do_syscall_64 --func=ns_get_path -o test_1.ll dummy.bc


