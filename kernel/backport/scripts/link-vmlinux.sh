#!/bin/sh
# SPDX-License-Identifier: GPL-2.0
#
# link vmlinux
#
# vmlinux is linked from the objects selected by $(KBUILD_VMLINUX_OBJS) and
# $(KBUILD_VMLINUX_LIBS). Most are built-in.a files from top-level directories
# in the kernel tree, others are specified in arch/$(ARCH)/Makefile.
# $(KBUILD_VMLINUX_LIBS) are archives which are linked conditionally
# (not within --whole-archive), and do not require symbol indexes added.
#
# vmlinux 
#   ^
#   |
#   +--< $(KBUILD_VMLINUX_OBJS)
#   |    +--< init/built-in.a drivers/built-in.a mm/built-in.a + more
#   |
#   +--< $(KBUILD_VMLINUX_LIBS)
#   |    +--< lib/lib.a + more
#   |
#   +-< ${kallsymso} (see description in KALLSYMS section)
#
# vmlinux version (uname -v) cannot be updated during normal
# descending-into-subdirs phase since we do not yet know if we need to
# update vmlinux.
# Therefore this step is delayed until just before final link of vmlinux.
#
# System.map is generated to document addresses of all kernel symbols

# Error out on error
set -e

# Nice output in kbuild format
# Will be supressed by "make -s"
info()
{
	if [ "${quiet}" != "silent_" ]; then
		printf "  %-7s %s\n" ${1} ${2}
	fi
}


# LTO implementation directives
#

force_preserve_symbols()
{
   declare -a element_list=("${!1}")
   rm -f main_symbols
   touch main_symbols
   for i in "${element_list[@]}"
   do
       ${LLVM_NM} -g $i >> main_symbols
   done
   #nm_symbols=$(awk '$2=="T"{printf "-u %s ",$3}' main_symbols)
   nm_symbols=$(awk '{printf "-u %s ",$1}' main_symbols_test)

   # Remove previous preserved symbols
   rm -f empty.o
   rm -f begin.o

   ar rcsD empty.o
   ld -m elf_x86_64 ${nm_symbols} -r -o begin.o empty.o
}

get_undefined_symbols()
{
   local m_symbols
   m_symbols=$(awk '!$3{printf "-u %s ",$2} $3{printf "-u %s ",$3}' main_symbols)
   echo ${m_symbols}
}

replace_suffix()
{
     declare -a element_list=("${!1}")
     local from_prefix=${2}
     local to_prefix=${3}
     echo "${element_list[@]//$from_prefix/$to_prefix}"
}

diff(){
  awk 'BEGIN{RS=ORS=" "}
       {NR==FNR?a[$0]++:a[$0]--}
       END{for(k in a)if(a[k])print k}' <(echo -n "${!1}") <(echo -n "${!2}")
}

get_intersection()
{
   declare -a element_list_1=("${!1}")
   declare -a element_list_2=("${!2}")
   Array3=($(diff element_list_1[@] element_list_2[@]))
   echo ${Array3[@]}
}


get_existing()
{
   declare -a element_list=("${!1}")
   local counter=0
   files_array=
   for i in "${element_list[@]}"
   do
        if [ -e $i ]
        then
	    files_array[$counter]=$i
            let counter=counter+1
        fi
   done
   echo "${files_array[@]}"
}

get_matching_suffix()
{
  declare -a element_list=("${!1}")
  local suffix=${2}
  local counter=0
  files_array=
  for i in "${element_list[@]}"
   do
        if [[ $i == *${suffix} ]]
        then
	    files_array[$counter]=$i
            let counter=counter+1
        fi
   done
   echo "${files_array[@]}"
}

# Usage get_joi 'file1.suffix1 file2.suffix1 ... .suffix1 .suffix2'
# Returns a list of the form 'file1.suffix1 [file1.suffix2] file2.suffix1 [file2.suffix2] ...'
# Only returns .suffix2 elements if the specific files exist in the system
get_join()
{
    declare -a element_list=("${!1}")
    local suffix=${2}
    local newsuffix=${3}
    files_array=
    for i in "${element_list[@]}"
     do
        if [ -e ${i//$suffix/$newsuffix} ]
        then 
            files_array[$counter]=${i//$suffix/$newsuffix}
            let counter=counter+1
        fi
        files_array[$counter]=$i
        let counter=counter+1
     done
    echo "${files_array[@]}"   
}

get_ordered()
{
    declare -a element_list=("${!1}")
    local suffix=${2}
    local newsuffix=${3}
    files_array=
    for i in "${element_list[@]}"
     do
        if [ -e ${i//$suffix/$newsuffix} ]
        then 
            files_array[$counter]=${i//$suffix/$newsuffix}
            let counter=counter+1
        else
            files_array[$counter]=$i
            let counter=counter+1
        fi
     done 
     echo "${files_array[@]}"  
}

# Implements the vmlinux_link for Full LTO
lto_vmlinux_link()
{
     local modules
     local lto_objects
     local lto_archives
     local lto_libs
     local joined_objects
     local joined_archives
     local wrapper
     modules=(`echo ${KBUILD_VMLINUX_OBJS}`);
     lto_objects=`get_matching_suffix modules[@] .o`
     lto_archives=`get_matching_suffix modules[@] .a`

     wrapper=(`echo ${lto_objects}`);
     joined_objects=`get_ordered wrapper[@] .o .bc`
     echo ${joined_objects}

     wrapper=(`echo ${lto_archives}`); 
     joined_archives=`get_join wrapper[@] .a .bc`
     echo ${joined_archives}


     echo 'Entered lto_vmlinux_link'
     objects="--whole-archive			\
			begin.o  ${joined_objects} ${joined_archives}			\
			--no-whole-archive			\
			--start-group				\
			${KBUILD_VMLINUX_LIBS}			\
			--end-group				\
			${2}"

     ${LD} ${KBUILD_LDFLAGS} ${LDFLAGS_vmlinux} \
      -plugin-opt=O0 -plugin-opt=--code-model=kernel -plugin-opt=--disable-tail-calls \
                              -plugin-opt=-enable-lto-internalization=false -o ${1}	\
			-T ${lds} ${objects}

}

# Implements the modpost_link for Full LTO
lto_modpost_link()
{
     local modules
     local lto_objects
     local lto_archives
     local lto_libs
     local joined_objects
     local joined_archives
     local preserve_files
     local wrapper
     modules=(`echo ${KBUILD_VMLINUX_OBJS}`);
     lto_objects=`get_matching_suffix modules[@] .o`
     lto_archives=`get_matching_suffix modules[@] .a`

     wrapper=(`echo ${lto_objects}`);
     joined_objects=`get_ordered wrapper[@] .o .bc`
     echo ${joined_objects}

     wrapper=(`echo ${lto_archives}`); 
     joined_archives=`get_join wrapper[@] .a .bc`
     echo ${joined_archives}

     #This will be used by lto_vmlinux_link
     wrapper=(`echo ${joined_objects} ${joined_archives}`); 
     preserve_files=`get_matching_suffix wrapper[@] .bc`
     wrapper=(`echo ${preserve_files}`); 
     echo ${preserve_files}
    `force_preserve_symbols wrapper[@]`

     echo 'Entered lto_modpost_link'
     objects="--whole-archive				        \
		begin.o  ${joined_objects} ${joined_archives}	        \
		--no-whole-archive				\
		--start-group					\
		${KBUILD_VMLINUX_LIBS}				\
		--end-group"

     ${LD} ${KBUILD_LDFLAGS} -plugin-opt=O0 -plugin-opt=--code-model=kernel -plugin-opt=--disable-tail-calls -plugin-opt=-enable-lto-internalization=false -r -o ${1} ${objects}
     
}


# Implements the vmlinux_link for Full LTO
lto_incremental_vmlinux_link()
{
     local modules
     local lto_objects
     local lto_archives
     local lto_libs
     local joined_objects
     local joined_archives
     local wrapper
     modules=(`echo ${KBUILD_VMLINUX_OBJS}`);
     lto_objects=`get_matching_suffix modules[@] .o`
     lto_archives=`get_matching_suffix modules[@] .a`

     wrapper=(`echo ${lto_objects}`);
     joined_objects=`get_ordered wrapper[@] .o .bc`
     echo ${joined_objects}

     wrapper=(`echo ${lto_archives}`); 
     joined_archives=`get_join wrapper[@] .a .bc`
     echo ${joined_archives}

     wrapper=(`echo ${joined_objects}`);
     o_objects=`get_matching_suffix wrapper[@] .o` 


     echo 'Entered lto_vmlinux_link'
     objects="--whole-archive			\
		       ${o_objects} kernel.o ${lto_archives}	\
			--no-whole-archive			\
			--start-group				\
			${KBUILD_VMLINUX_LIBS//.a/.parc}			\
			--end-group				\
			${2}"

     ${LD} ${KBUILD_LDFLAGS} ${LDFLAGS_vmlinux} -o ${1}	\
			-T ${lds} ${objects}

}

# Implements the modpost_link for Full LTO
lto_incremental_modpost_link()
{
     local modules
     local lto_objects
     local lto_archives
     local lto_libs
     local joined_objects
     local joined_archives
     local preserve_files
     local o_objects
     local wrapper
     modules=(`echo ${KBUILD_VMLINUX_OBJS}`);
     lto_objects=`get_matching_suffix modules[@] .o`
     lto_archives=`get_matching_suffix modules[@] .a`

     wrapper=(`echo ${lto_objects}`);
     joined_objects=`get_ordered wrapper[@] .o .bc`
     echo ${joined_objects}

     wrapper=(`echo ${lto_archives}`); 
     joined_archives=`get_join wrapper[@] .a .bc`
     echo ${joined_archives}

     wrapper=(`echo ${joined_objects} ${joined_archives}`); 
     preserve_files=`get_matching_suffix wrapper[@] .bc`

     wrapper=(`echo ${joined_objects}`);
     o_objects=`get_matching_suffix wrapper[@] .o` 

     if [ "${HAVE_VUPROFILER}" == "True" ]; then
        VUPROFILER_LIB=${PASS_PATH}/tagLib.bc
     fi

     if [ "${COMPILE_FAST}" != "True" ]; then
         if [ "${HAVE_VUPROFILER}" == "True" ]; then
             # Compile Tag Lib
             make -C ../../include/profiler clean
             make -C ../../include/profiler all
         fi

         # We assemble the llvm blob (if we build the profiler also link in tagLib) -plugin-opt=--disable-tail-calls
         ${LD} ${KBUILD_LDFLAGS} -plugin-opt=emit-llvm -plugin-opt=O3  -plugin-opt=-enable-lto-internalization=false -r -o intermediate_kernel.bc --whole-archive ${preserve_files} ${VUPROFILER_LIB} --no-whole-archive --start-group ${KBUILD_VMLINUX_LIBS//.a/.bc} --end-group

         # Clean inline asm that initializes entries from the syscall table
         ${OPT} -load=${PASS_PATH}/asm_pass.so -sanitize_asm -o  intermediate_kernel_Phase2.bc intermediate_kernel.bc > asm.dump
         
         ${OPT} -load=${PASS_PATH}/static_symrez.so -remap_static_clash_symbols -o bare_kernel.bc intermediate_kernel_Phase2.bc > sym_rez.dump
     fi


     
     if [ "${HAVE_VUPROFILER}" == "True" ]; then
        # Instrument the callbacks supplied by tagLib
        ${OPT} -load=${PASS_PATH}/vuprofiler.so -vuprofiler_instrument -o  profiling_kernel.bc  bare_kernel.bc

        # Disassemble the instrumented kernel and generate the coverage map
        ${LLVM_DIS} profiling_kernel.bc
        
        # This will create a binding between binary level tags and the LLVM location of a call-site 
        awk '($1 ~ /^define$/){parent=$0;count=0} ($3 ~ /^@vuprof_setTag\(i64$/){tag=$0; getline; print parent"||||"tag"||||"$0"||||"count;count++}' profiling_kernel.ll > Coverage.Map  

        # Inline vuprofiler_setTag and vuprofiler_getTag calls to reduce runtime overhead        
        ${OPT} -load=${PASS_PATH}/cb_inliner.so -inline_callbacks -o  kernel.bc profiling_kernel.bc   
     else
        if [ "${HAVE_ICP}" == "Custom" ]; then
            # Autdated implementation (old optimization pipeline). Now whe're using a modified LLVM icp pass.
            ${OPT} -load=${PASS_PATH}/icp_pass.so -icp_promote -o  kernel.bc bare_kernel.bc > icp_dump
        elif [ "${HAVE_ICP}" == "LLVM" ]; then
            
           # PGO And stuff  --profile-sample-accurate --profile-sample-accurate
            # IF  PIBE_ANNOTATE_PROFILE=NO use the annotated profile from previous run
            if [ "${PIBE_ANNOTATE_PROFILE}" == "Yes" ]; then
                 ${OPT}  -add-discriminators -o discriminator.bc bare_kernel.bc
                 ${OPT} -load=${PASS_PATH}/icp_convert.so -icp_convert -o  mutated.bc discriminator.bc > icp_dump_${DEFENSE_CONFIGURATION}
                 ${OPT} -disable-inlining --profile-sample-accurate  --static-func-full-module-prefix=false --static-func-strip-dirname-prefix=30 --sample-profile --sample-profile-file=${PIBE_WORKLOAD_FILE}  -pass-remarks-missed=SampleProfileLoaderLegacyPass -pass-remarks=SampleProfileLoaderLegacyPass  -pass-remarks-output=remarks_sampleprofile.kernel  mutated.bc -o sample_profile.bc
            fi
            
            # PROMOTION AND INLINING
            if [ "${PIBE_OPTIMIZATIONS}" == "Promote" ]; then
                 ${OPT} -disable-inlining -load=${PASS_PATH}/linearpromotion.so  -icp_linear --profile-summary-cutoff-hot=${PIBE_BUDGET}  -icp-samplepgo-linear --icp-max-prom=40  --icp-remaining-percent-threshold=0 --icp-total-percent-threshold=0  -pass-remarks=PGOIndirectCallPromotion   -pass-remarks-missed=PGOIndirectCallPromotion   -pass-remarks-output=dummy.kernel  sample_profile.bc -o inlined_kernel.bc > icp_stats_${DEFENSE_CONFIGURATION}_${PIBE_BUDGET}
            elif [ "${PIBE_OPTIMIZATIONS}" == "Inline"  ]; then
                 ${OPT} -disable-inlining  -load=${PASS_PATH}/inliner.so  --profile-summary-cutoff-hot=${PIBE_BUDGET} -hotness_inliner -adce --globaldce -o inlined_kernel.bc sample_profile.bc > inlining_stats_${DEFENSE_CONFIGURATION}_${PIBE_BUDGET}
            elif [ "${PIBE_OPTIMIZATIONS}" == "PromoteAndInline"  ]; then
                 ${OPT} -disable-inlining -load=${PASS_PATH}/linearpromotion.so  -icp_linear --profile-summary-cutoff-hot=${PIBE_BUDGET}  -icp-samplepgo-linear --icp-max-prom=40  --icp-remaining-percent-threshold=0 --icp-total-percent-threshold=0  -pass-remarks=PGOIndirectCallPromotion   -pass-remarks-missed=PGOIndirectCallPromotion   -pass-remarks-output=icp_remarks_${PIBE_BUDGET}.kernel  sample_profile.bc -o sample_profile_icp.bc > icp_stats_${DEFENSE_CONFIGURATION}_${PIBE_BUDGET}
                 ${OPT} -disable-inlining  -load=${PASS_PATH}/inliner.so  --profile-summary-cutoff-hot=${PIBE_BUDGET} -hotness_inliner -adce --globaldce -o inlined_kernel.bc sample_profile_icp.bc > inlining_stats_${DEFENSE_CONFIGURATION}_${PIBE_BUDGET}
            else
                 cp sample_profile.bc inlined_kernel.bc 
            fi

            # JUST FOR LVI same as on the else branch
            if [ "${PIBE_RUN_LVI_BLACKLIST}" == "Yes" ]; then
                   ${OPT}  -load=${PASS_PATH}/bl_lvi.so -blacklist_lvi -o  kernel.bc   inlined_kernel.bc   > lvi_bl.dump
            else
                  cp inlined_kernel.bc kernel.bc
            fi 
            #cp bare_kernel.bc kernel.bc
            #echo "pass"
        else
             # JUST FOR LVI (blacklist .init section functions, only called during kernel initialization and hence not vulnerable to speculative attacks after kernel initialization
             # This saves us some additional instrumentation space.
            if [ "${PIBE_RUN_LVI_BLACKLIST}" == "Yes" ]; then
                   ${OPT}  -load=${PASS_PATH}/bl_lvi.so -blacklist_lvi -o  kernel.bc   bare_kernel.bc   > lvi_bl_duwo.dump
            else
                  cp bare_kernel.bc kernel.bc
            fi
            
        fi
     fi

     # --relocation-model=static -disable-tail-calls   weird ${LTO_LVI} 
     ${LLC} -O2 -code-model=kernel -o kernel.s kernel.bc

     #awk '!($2 ~/^.init.rodata,"aM",@progbits/){print $0} ($2 ~/^.init.rodata,"aM",@progbits/){print "\t.section\t.init.rodata,\"a\",@progbits"}' kernel.s > kernel.ss
     ${AS} --64 -o kernel.o kernel.s
         
    
     cp kernel.o back_kernel.o
     ${STRIP} -g -o intermediate.o kernel.o
     cp intermediate.o kernel.o


     #cp jos back_kernel.o
     #${STRIP} -g -o intermediate.o jos
     #cp intermediate.o kernel.o

     wrapper=(`echo ${preserve_files}`); 
     echo ${preserve_files}
    `force_preserve_symbols wrapper[@]`

     echo 'Entered lto_modpost_link'
     objects="--whole-archive				        \
		 ${o_objects} kernel.o ${lto_archives}	        \
		--no-whole-archive				\
		--start-group					\
		${KBUILD_VMLINUX_LIBS//.a/.parc}				\
		--end-group"

     ${LD} ${KBUILD_LDFLAGS} -r -o ${1} ${objects}
     
}


# Implements the vmlinux_link for Full LTO
lto_oldincremental_vmlinux_link()
{
     local modules
     local lto_objects
     local lto_archives
     local lto_libs
     local joined_objects
     local joined_archives
     local wrapper
     modules=(`echo ${KBUILD_VMLINUX_OBJS}`);
     lto_objects=`get_matching_suffix modules[@] .o`
     lto_archives=`get_matching_suffix modules[@] .a`

     wrapper=(`echo ${lto_objects}`);
     joined_objects=`get_ordered wrapper[@] .o .bc`
     echo ${joined_objects}

     wrapper=(`echo ${lto_archives}`); 
     joined_archives=`get_join wrapper[@] .a .bc`
     echo ${joined_archives}

     wrapper=(`echo ${joined_objects}`);
     o_objects=`get_matching_suffix wrapper[@] .o` 


     echo 'Entered lto_vmlinux_link'
     objects="--whole-archive			\
		       ${o_objects} kernel.o ${lto_archives}	\
			--no-whole-archive			\
			--start-group				\
			${KBUILD_VMLINUX_LIBS}			\
			--end-group				\
			${2}"

     ${LD} ${KBUILD_LDFLAGS} ${LDFLAGS_vmlinux} -o ${1}	\
			-T ${lds} ${objects}

}

# Implements the modpost_link for Full LTO
lto_oldincremental_modpost_link()
{
     local modules
     local lto_objects
     local lto_archives
     local lto_libs
     local joined_objects
     local joined_archives
     local preserve_files
     local o_objects
     local wrapper
     modules=(`echo ${KBUILD_VMLINUX_OBJS}`);
     lto_objects=`get_matching_suffix modules[@] .o`
     lto_archives=`get_matching_suffix modules[@] .a`

     wrapper=(`echo ${lto_objects}`);
     joined_objects=`get_ordered wrapper[@] .o .bc`
     echo ${joined_objects}

     wrapper=(`echo ${lto_archives}`); 
     joined_archives=`get_join wrapper[@] .a .bc`
     echo ${joined_archives}

     wrapper=(`echo ${joined_objects} ${joined_archives}`); 
     preserve_files=`get_matching_suffix wrapper[@] .bc`

     wrapper=(`echo ${joined_objects}`);
     o_objects=`get_matching_suffix wrapper[@] .o` 

     if [ "${HAVE_VUPROFILER}" == "True" ]; then
        VUPROFILER_LIB=${VUPROFILER_PATH}/tagLib.bc
     fi

     # We link the llvm .bc blob here (additionally also link in vuprofiler's tagLib)
     ${LD} ${KBUILD_LDFLAGS} -plugin-opt=emit-llvm -plugin-opt=O3 -plugin-opt=--disable-tail-calls -plugin-opt=-enable-lto-internalization=false -r -o bare_kernel.bc --whole-archive ${preserve_files} ${VUPROFILER_LIB}

     if [ "${HAVE_VUPROFILER}" == "True" ]; then

        # Instrument the callbacks supplied by tagLib
        ${OPT} -load=${VUPROFILER_PATH}/vuprofiler.so -vuprofiler_instrument -o  profiling_kernel.bc  bare_kernel.bc

        # Disassemble the instrumented kernel and generate the coverage map
        ${LLVM_DIS} profiling_kernel.bc
        
        awk '($1 ~ /^define$/){parent=$0;count=0} ($3 ~ /^@vuprof_setTag\(i64$/){tag=$0; getline; print parent"||||"tag"||||"$0"||||"count;count++}' profiling_kernel.ll > Coverage.Map  

        # Inline vuprofiler_setTag and vuprofiler_getTag calls to boost performance        
        ${OPT} -load=${VUPROFILER_PATH}/cb_inliner.so -inline_callbacks -o  kernel.bc profiling_kernel.bc   
     else
        if [ "${HAVE_ICP}" == "True" ]; then
            ${OPT} -load=${ICP_PATH}/icp_pass.so -icp_promote -o  kernel.bc bare_kernel.bc > icp_dump
        else
            cp bare_kernel.bc kernel.bc
        fi
     fi

     # --relocation-model=static
     ${LLC} -O2 -code-model=kernel -disable-tail-calls -o kernel.s kernel.bc
     ${AS} --64 -o kernel.o kernel.s

     wrapper=(`echo ${preserve_files}`); 
     echo ${preserve_files}
    `force_preserve_symbols wrapper[@]`

     echo 'Entered lto_modpost_link'
     objects="--whole-archive				        \
		 ${o_objects} kernel.o ${lto_archives}	        \
		--no-whole-archive				\
		--start-group					\
		${KBUILD_VMLINUX_LIBS//.a/.parc}				\
		--end-group"

     ${LD} ${KBUILD_LDFLAGS} -r -o ${1} ${objects}
     
}

# LTO Hooks
# At the moment call the full-lto backend
# In the future we want to support ThinLTO Builds through these hooks as well
lto_vmlinux_switch()
{
     echo 'Entered lto_vmlinux_switch'

     lto_incremental_vmlinux_link ${1} ${2}
}

lto_modpost_switch()
{
     echo 'Entered lto_modpost_switch'

     lto_incremental_modpost_link ${1}
}



# Link of vmlinux.o used for section mismatch analysis
# ${1} output file
modpost_link()
{
	local objects
        if [ "${HAVE_LTO}" == "True" ]; then
                        lto_modpost_switch ${1}
        else
	objects="--whole-archive				\
		${KBUILD_VMLINUX_OBJS}				\
		--no-whole-archive				\
		--start-group					\
		${KBUILD_VMLINUX_LIBS}				\
		--end-group"

	${LD} ${KBUILD_LDFLAGS} -r -o ${1} ${objects}
        fi
}

# Link of vmlinux
# ${1} - optional extra .o files
# ${2} - output file
vmlinux_link()
{
	local lds="${objtree}/${KBUILD_LDS}"
	local objects

	if [ "${SRCARCH}" != "um" ]; then
                if [ "${HAVE_LTO}" == "True" ]; then
                     lto_vmlinux_switch ${2} ${1}
                else
		objects="--whole-archive			\
			${KBUILD_VMLINUX_OBJS}			\
			--no-whole-archive			\
			--start-group				\
			${KBUILD_VMLINUX_LIBS}			\
			--end-group				\
			${1}"

		${LD} ${KBUILD_LDFLAGS} ${LDFLAGS_vmlinux} -o ${2}	\
			-T ${lds} ${objects}
                fi
	else
		objects="-Wl,--whole-archive			\
			${KBUILD_VMLINUX_OBJS}			\
			-Wl,--no-whole-archive			\
			-Wl,--start-group			\
			${KBUILD_VMLINUX_LIBS}			\
			-Wl,--end-group				\
			${1}"

		${CC} ${CFLAGS_vmlinux} -o ${2}			\
			-Wl,-T,${lds}				\
			${objects}				\
			-lutil -lrt -lpthread
		rm -f linux
	fi
}


# Create ${2} .o file with all symbols from the ${1} object file
kallsyms()
{
	info KSYM ${2}
	local kallsymopt;

	if [ -n "${CONFIG_KALLSYMS_ALL}" ]; then
		kallsymopt="${kallsymopt} --all-symbols"
	fi

	if [ -n "${CONFIG_KALLSYMS_ABSOLUTE_PERCPU}" ]; then
		kallsymopt="${kallsymopt} --absolute-percpu"
	fi

	if [ -n "${CONFIG_KALLSYMS_BASE_RELATIVE}" ]; then
		kallsymopt="${kallsymopt} --base-relative"
	fi

	local aflags="${KBUILD_AFLAGS} ${KBUILD_AFLAGS_KERNEL}               \
		      ${NOSTDINC_FLAGS} ${LINUXINCLUDE} ${KBUILD_CPPFLAGS}"

	local afile="`basename ${2} .o`.S"

	${NM} -n ${1} | scripts/kallsyms ${kallsymopt} > ${afile}
	${CC} ${aflags} -c -o ${2} ${afile}
}

# Create map file with all symbols from ${1}
# See mksymap for additional details
mksysmap()
{
	${CONFIG_SHELL} "${srctree}/scripts/mksysmap" ${1} ${2}
}

sortextable()
{
	${objtree}/scripts/sortextable ${1}
}

# Delete output files in case of error
cleanup()
{
	rm -f .tmp_System.map
	rm -f .tmp_kallsyms*
	rm -f .tmp_vmlinux*
	rm -f System.map
	rm -f vmlinux
	rm -f vmlinux.o
}

on_exit()
{
	if [ $? -ne 0 ]; then
		cleanup
	fi
}
trap on_exit EXIT

on_signals()
{
	exit 1
}
trap on_signals HUP INT QUIT TERM

#
#
# Use "make V=1" to debug this script
case "${KBUILD_VERBOSE}" in
*1*)
	set -x
	;;
esac

if [ "$1" = "clean" ]; then
	cleanup
	exit 0
fi

# We need access to CONFIG_ symbols
. include/config/auto.conf

# Update version
info GEN .version
if [ -r .version ]; then
	VERSION=$(expr 0$(cat .version) + 1)
	echo $VERSION > .version
else
	rm -f .version
	echo 1 > .version
fi;

# final build of init/
${MAKE} -f "${srctree}/scripts/Makefile.build" obj=init

#link vmlinux.o
info LD vmlinux.o
modpost_link vmlinux.o

# modpost vmlinux.o to check for section mismatches
${MAKE} -f "${srctree}/scripts/Makefile.modpost" vmlinux.o

kallsymso=""
kallsyms_vmlinux=""
if [ -n "${CONFIG_KALLSYMS}" ]; then

	# kallsyms support
	# Generate section listing all symbols and add it into vmlinux
	# It's a three step process:
	# 1)  Link .tmp_vmlinux1 so it has all symbols and sections,
	#     but __kallsyms is empty.
	#     Running kallsyms on that gives us .tmp_kallsyms1.o with
	#     the right size
	# 2)  Link .tmp_vmlinux2 so it now has a __kallsyms section of
	#     the right size, but due to the added section, some
	#     addresses have shifted.
	#     From here, we generate a correct .tmp_kallsyms2.o
	# 3)  That link may have expanded the kernel image enough that
	#     more linker branch stubs / trampolines had to be added, which
	#     introduces new names, which further expands kallsyms. Do another
	#     pass if that is the case. In theory it's possible this results
	#     in even more stubs, but unlikely.
	#     KALLSYMS_EXTRA_PASS=1 may also used to debug or work around
	#     other bugs.
	# 4)  The correct ${kallsymso} is linked into the final vmlinux.
	#
	# a)  Verify that the System.map from vmlinux matches the map from
	#     ${kallsymso}.

	kallsymso=.tmp_kallsyms2.o
	kallsyms_vmlinux=.tmp_vmlinux2

	# step 1
	vmlinux_link "" .tmp_vmlinux1
	kallsyms .tmp_vmlinux1 .tmp_kallsyms1.o

	# step 2
	vmlinux_link .tmp_kallsyms1.o .tmp_vmlinux2
	kallsyms .tmp_vmlinux2 .tmp_kallsyms2.o

	# step 3
	size1=$(${CONFIG_SHELL} "${srctree}/scripts/file-size.sh" .tmp_kallsyms1.o)
	size2=$(${CONFIG_SHELL} "${srctree}/scripts/file-size.sh" .tmp_kallsyms2.o)

	if [ $size1 -ne $size2 ] || [ -n "${KALLSYMS_EXTRA_PASS}" ]; then
		kallsymso=.tmp_kallsyms3.o
		kallsyms_vmlinux=.tmp_vmlinux3

		vmlinux_link .tmp_kallsyms2.o .tmp_vmlinux3

		kallsyms .tmp_vmlinux3 .tmp_kallsyms3.o
	fi
fi

info LD vmlinux
vmlinux_link "${kallsymso}" vmlinux

if [ -n "${CONFIG_BUILDTIME_EXTABLE_SORT}" ]; then
	info SORTEX vmlinux
	sortextable vmlinux
fi

info SYSMAP System.map
mksysmap vmlinux System.map

# step a (see comment above)
if [ -n "${CONFIG_KALLSYMS}" ]; then
	mksysmap ${kallsyms_vmlinux} .tmp_System.map

	if ! cmp -s System.map .tmp_System.map; then
		echo >&2 Inconsistent kallsyms data
		echo >&2 Try "make KALLSYMS_EXTRA_PASS=1" as a workaround
		exit 1
	fi
fi
