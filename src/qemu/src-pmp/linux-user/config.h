/* PMP Modification - Begin */
#include "config.inc"

#define __NR_setpid 326
#define __NR_setflag 327
#define __NR_printmsg 328
#define __NR_validaddr 329
#define __NR_killfamily 330

extern target_ulong program_code_offset, program_code_start, program_code_end;

extern unsigned int index_path_scheme;
extern GSList *path_scheme;

extern char fname_path_scheme[BUFF_LEN], fname_path_trace[BUFF_LEN];
extern char fname_insn_tmp[BUFF_LEN], fname_dep_tmp[BUFF_LEN];

struct pair {
  target_ulong first;
  target_ulong second;
};

extern void do_initialize(void);
extern void read_path_scheme(void);

static inline int g_slist_compare_pair_first(const void *pointera, const void *pointerb) {
    struct pair *pa = (struct pair *)(pointera);
    struct pair *pb = (struct pair *)(pointerb);
    
    if (!pa || !pb) return -1;
    else return pa->first - pb->first;
}

static inline int is_program_code(target_ulong iaddr) {
    return (iaddr >= program_code_start && iaddr <= program_code_end);
}

static inline int valid_addr(target_ulong insn_addr) {
    return (insn_addr > ALLOCATE_SIZE);
}
/* PMP Modification - End */
