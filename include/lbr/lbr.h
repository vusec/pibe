#ifndef __LBR_H__
#define __LBR_H__

/* To enable the LBR capabilities of recent Intel processors, one has to 
 * write specific values to two model specific registers (MSR):
 * - MSR_IA32_DEBUGCTL: To enable the LBR feature in the first place.
 * - MSR_LBR_SELECT:    To tell the CPU what kind of branches it should track.
 */

#ifndef MSR_IA32_DEBUGCTLMSR
#define MSR_IA32_DEBUGCTLMSR        0x000001d9
#endif
#ifndef MSR_LBR_SELECT
#define MSR_LBR_SELECT              0x000001c8
#endif
#ifndef MSR_LBR_TOS
#define MSR_LBR_TOS                 0x000001c9
#endif
#ifndef MSR_LBR_NHM_FROM
#define MSR_LBR_NHM_FROM            0x00000680
#endif
#ifndef MSR_LBR_NHM_TO
#define MSR_LBR_NHM_TO              0x000006c0
#endif

/* The lowest bit in the MSR_IA32_DEBUG register enables the LBR feature */
#define IA32_DEBUGCTL 0x1

/* The LBR_SELECT register has the following options:
 *
 * Bit Field     Bit Offset Access Description
 * CPL_EQ_0      0          R/W    When set, do not capture branches occurring in ring 0
 * CPL_NEQ_0     1          R/W    When set, do not capture branches occurring in ring >0
 * JCC           2          R/W    When set, do not capture conditional branches
 * NEAR_REL_CALL 3          R/W    When set, do not capture near relative calls
 * NEAR_IND_CALL 4          R/W    When set, do not capture near indirect calls
 * NEAR_RET      5          R/W    When set, do not capture near returns
 * NEAR_IND_JMP  6          R/W    When set, do not capture near indirect jumps
 * NEAR_REL_JMP  7          R/W    When set, do not capture near relative jumps
 * FAR_BRANCH    8          R/W    When set, do not capture far branches
 * EN_CALLSTACK  9                 Enable LBR stack to use LIFO filtering to capture
 *                                 call stack profile
 * Reserved      63:10             Must be zero
 *
 * This is currently set to:
 * 0x1C6 = 0111000110   --> capture branches occuring in ring = 0;
 *                          capture near relative calls
 *                          capture near indirect calls
 *                          capture near returns
 */
#define LBR_SELECT 0x3C6

/* This is the number of LBR states that we allocate room for in our module.
 * Whenever a process that is being LBRed is scheduled out, we store its LBR
 * state into a cache. This number thus represents the maximum number of
 * simultanously active threads/processes we support for our target.
 */
#define LBR_CACHE_SIZE 2024


/* The LBR.from registers contain a bit more information than just the source
 * address of the branch:
 *
 * Bit Field Bit Offset Access Description   
 * Data      47:0       R/O    The linear address of the branch instruction itself, 
 *                             this is the “branch from“ address.
 * SIGN_EXT  60:48      R/O    Signed extension of bit 47 of this register.
 * TSX_ABORT 61         R/O    When set, indicates a TSX Abort entry 
 *                             LBR_FROM: EIP at the time of the TSX Abort
 *                             LBR_TO  : EIP of the start of HLE region, or EIP of 
 *                                       the RTM Abort Handler
 * IN_TSX    62         R/O    When set, indicates the entry occurred in a TSX region
 * MISPRED   63         R/O    When set, indicates either the target of the branch was 
 *                             mispredicted and/or the direction (taken/non-taken) was 
 *                             mispredicted; otherwise, the target branch was predicted.
 *
 * The layout of _TO registers is simpler:
 *
 * Bit Field Bit Offset Access Description
 * Data      47:0       R/O    The linear address of the target of the branch instruction i
 *                             itself, this is the “branch to“ address.
 * SIGN_EXT  63:4       R/O    Signed extension of bit 47 of this register.
 */
#define LBR_FROM_FLAG_MISPRED (1ULL << 63)
#define LBR_FROM_FLAG_IN_TSX  (1ULL << 62)
#define LBR_FROM_FLAG_ABORT   (1ULL << 61)
/* I would say that we can ignore above and merge Data with SIGN_EXT */
#define LBR_SKIP 3
#define LBR_FROM(from) (uint64_t)((((int64_t)from) << LBR_SKIP) >> LBR_SKIP)

    
#endif /* __LBR_H__ */
