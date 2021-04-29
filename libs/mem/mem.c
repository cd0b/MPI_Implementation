#include "mem.h"
#include "sem.h"

#define DATA_SIZE 2048
#define PAGE_COUNT 10000
#define ENTRY_PER_PT 10
#define PT_COUNT MAX_PROC_COUNT

#define INVALID 0
#define VALID 1

/* used in page directory */
#define NOT_IN_USE -1

/* used in page offsets */
#define ALLOCATED -1

typedef struct __attribute__((packed)) {

    int order;
    int tag;
    unsigned char data[DATA_SIZE];

} Page;

typedef struct __attribute__((packed)) {

    int page_table_offset;
    int page_table_entry_offset;
    int page_offset;
    unsigned int valid;
    unsigned short sender_rank;
    unsigned short receiver_rank;

} Page_table_entry;

typedef struct __attribute__((packed)) {
    unsigned int entry_count;
    Page_table_entry entries[ENTRY_PER_PT];
    int next_page_table_offset;
    int prev_page_table_offset;
} Page_table;

typedef struct __attribute__((packed)) {

    int page_table_offset;
    unsigned int receiver_rank;

} Page_dir_entry;

typedef struct __attribute__((packed)) {

    unsigned short zero_region;
    unsigned short proc_count;
    Page_dir_entry page_dir_entries[PT_COUNT];
    Page_table page_tables[PT_COUNT];
    Page pages[PAGE_COUNT];
    int page_offsets[PAGE_COUNT];

} Mem;


static void load_directory(Mem* const, unsigned short);
static void load_page_tables(Mem* const, unsigned short);
static void load_page_offsets(Mem* const);

static Page_table_entry* alloc_page(Mem* const, unsigned short, unsigned short);
static int alloc_page_ipt(Mem* const);
static Page_table_entry* alloc_page_pt(Mem* const, int, unsigned short, unsigned short);
static int find_page_table(Mem* const mem, unsigned short receiver_rank);
static int alloc_pt(Mem* const, unsigned short);
static int alloc_pt_pd(Mem* const, unsigned short);

static void free_page(Mem* const, Page_table_entry*);
static void free_page_ipt(Mem* const, int);
static void free_page_pt(Mem* const, Page_table_entry*);
static void free_pt(Mem* const, int);
static void free_pt_pd(Mem* const, int);


static int write_page(Mem* const, unsigned short, unsigned short, const void*, unsigned int, int, int);
static Page_table_entry* find_pt_entry(Mem* const, unsigned short, unsigned short, int, int);
static int read_page(Mem* const, unsigned short, unsigned short, void*, unsigned int, int, int);


void init_mem(void* map, unsigned short proc_count) {

    Mem* const mem = (Mem* const)map;

    mem->zero_region = 0;
    mem->proc_count = proc_count;
    load_directory(mem, proc_count);
    load_page_tables(mem, proc_count);
    load_page_offsets(mem);

}

unsigned short get_proc_count(const void* map) {

    Mem* const mem = (Mem* const) map;
    return mem->proc_count;

}

unsigned short alloc_proc_rank(void* map) {

    Mem* const mem = (Mem* const) map;

    sem_t* zero_region = osem("zero_region");
 
    dsem(zero_region);
    unsigned short zero_region_val = mem->zero_region;
    ++mem->zero_region;
    usem(zero_region);

    return zero_region_val;

}

void free_proc(void* map, int rank) {
    Mem* const mem = (Mem* const)map;

    int pt_offset = find_page_table(mem, rank);

    char sem_name[DEFAULT_SEMAPHORE_NAME_LENGTH];
    gsn_pt(sem_name, rank);
    sem_t* pt_sem = osem(sem_name);

    dsem(pt_sem);
    while(pt_offset != NOT_IN_USE) {
        mem->page_tables[pt_offset].entry_count = 0;
        mem->page_tables[pt_offset].prev_page_table_offset = NOT_IN_USE;
        for(int i = 0 ; i < ENTRY_PER_PT; ++i) {
            mem->page_tables[pt_offset].entries[i].valid = INVALID;
            free_page_ipt(mem, mem->page_tables[pt_offset].entries[i].page_offset);
        }
        int temp_offset = mem->page_tables[pt_offset].next_page_table_offset;
        mem->page_tables[pt_offset].next_page_table_offset = NOT_IN_USE;
        free_pt_pd(mem, pt_offset);
        pt_offset = temp_offset;
    }
    usem(pt_sem);
}


int memwrite(void* map, unsigned short receiver_rank, 
    unsigned short sender_rank, const void* buffer, unsigned int length, int tag) {
    
    Mem* const mem = (Mem* const) map;

    int order = 0;

    while(length > 0) {
        unsigned int will_send = 0;
        if(length > DATA_SIZE)
            will_send = DATA_SIZE;
        else
            will_send = length;
    
        if(write_page(mem, receiver_rank, sender_rank, buffer, length, tag, order) == 0) {
            buffer += will_send;
            length -= will_send;
            ++order;
        }
    }

    return 0;

}


int memread(void* map, unsigned short receiver_rank, 
    unsigned short sender_rank, void* buffer, unsigned int length, int tag) {

    Mem* const mem = (Mem* const) map;

    int order = 0;

    while(length > 0) {
        unsigned int will_recv = 0;
        if(length > DATA_SIZE)
            will_recv = DATA_SIZE;
        else
            will_recv = length;

        if(read_page(mem, receiver_rank, sender_rank, buffer, length, tag, order) == 0) {
            buffer += will_recv;
            length -= will_recv;
            ++order;
        }
    }

    return 0;

}









static void free_page(Mem* const mem, Page_table_entry* pte) {

    free_page_ipt(mem, pte->page_offset);
    free_page_pt(mem, pte);

}

static void free_page_ipt(Mem* const mem, int page_offset) {
    
    sem_t* ipt_region = osem("ipt_region");

    dsem(ipt_region);
    for(int i = 0; i < PAGE_COUNT; ++i) {
        if(mem->page_offsets[i] == ALLOCATED) {
            mem->page_offsets[i] = page_offset;
            break;
        }
    }
    usem(ipt_region);

}

static void free_page_pt(Mem* const mem, Page_table_entry* pte) {

    char sem_name[DEFAULT_SEMAPHORE_NAME_LENGTH];
    gsn_pt(sem_name, pte->receiver_rank);
    sem_t* pt_sem = osem(sem_name);

    int pt_offset;

    dsem(pt_sem);
    pt_offset = pte->page_table_offset;
    pte->valid = INVALID;
    --(mem->page_tables[pt_offset].entry_count);

    free_pt(mem, pt_offset);
    
    usem(pt_sem);

}

static void free_pt(Mem* const mem, int pt_offset) {
    if(mem->page_tables[pt_offset].entry_count == 0 &&
        mem->page_tables[pt_offset].prev_page_table_offset != NOT_IN_USE) {
            // delete this page table and connect others
            int prev_offset = mem->page_tables[pt_offset].prev_page_table_offset;
            mem->page_tables[prev_offset].next_page_table_offset = mem->page_tables[pt_offset].next_page_table_offset;

            mem->page_tables[pt_offset].next_page_table_offset = NOT_IN_USE;
            mem->page_tables[pt_offset].prev_page_table_offset = NOT_IN_USE;

            free_pt_pd(mem, pt_offset);
    }
}


static void free_pt_pd(Mem* const mem, int pt_offset) {

    // pt_offset equals pd_offset
    sem_t* pd_region = osem("pd_region");

    dsem(pd_region);
    mem->page_dir_entries[pt_offset].receiver_rank = NOT_IN_USE;
    usem(pd_region);

}











/*
    allocates a free page
*/
static Page_table_entry* alloc_page(Mem* const mem, unsigned short receiver_rank, unsigned short sender_rank) {

    int page_offset = alloc_page_ipt(mem);
    if(page_offset == -1) { perror("Can not allocate page!"); return NULL; }
    return alloc_page_pt(mem, page_offset, receiver_rank, sender_rank);

}

/*
    finds and allocates a page from empty pages' table
*/
static int alloc_page_ipt(Mem* const mem) {

    sem_t* ipt_region = osem("ipt_region");

    int offset = ALLOCATED;
    dsem(ipt_region);
    for(int i = 0; i < PAGE_COUNT; ++i) {
        if(mem->page_offsets[i] != ALLOCATED) {
            offset = mem->page_offsets[i];
            mem->page_offsets[i] = ALLOCATED;
            break;
        }
    }
    usem(ipt_region);

    return offset;

}

/*
    add a entry to page table
    this entry includes new allocated page
*/
static Page_table_entry* alloc_page_pt(Mem* const mem, int page_offset, unsigned short receiver_rank, unsigned short sender_rank) {

    int pt_offset = alloc_pt(mem, receiver_rank);
    if(pt_offset == -1) { free_page_ipt(mem, page_offset); return NULL; } // free page in here

    char sem_name[DEFAULT_SEMAPHORE_NAME_LENGTH];
    gsn_pt(sem_name, receiver_rank);
    sem_t* pt_sem = osem(sem_name);

    int pte_offset = 0;
    dsem(pt_sem);
    for(int i = 0; i < ENTRY_PER_PT; ++i) {
        if(mem->page_tables[pt_offset].entries[i].valid == INVALID) {
            mem->page_tables[pt_offset].entries[i].page_offset = page_offset;
            mem->page_tables[pt_offset].entries[i].receiver_rank = receiver_rank;
            mem->page_tables[pt_offset].entries[i].sender_rank = sender_rank;
            mem->page_tables[pt_offset].entries[i].valid = VALID;
            ++(mem->page_tables[pt_offset].entry_count);
            pte_offset = i;
            break;
        }
    }
    usem(pt_sem);

    return &(mem->page_tables[pt_offset].entries[pte_offset]);

}

/*
    finds a page table from directory.
    if page table NOT_IN_USE returns -1
*/
static int find_page_table(Mem* const mem, unsigned short receiver_rank) {

    Page_dir_entry pde = mem->page_dir_entries[receiver_rank];
    if(pde.receiver_rank == NOT_IN_USE) { perror("Page table not in use!"); return -1; }
    return pde.page_table_offset;

}

/*
    get page table which set as in use
    if there is unsufficient empty slot in page table's of process
    extends process' page table with this new page table.
*/
static int alloc_pt(Mem* const mem, unsigned short receiver_rank) {

    char sem_name[DEFAULT_SEMAPHORE_NAME_LENGTH];
    gsn_pt(sem_name, receiver_rank);

    sem_t* pt_sem = osem(sem_name);

    int pt_offset = find_page_table(mem, receiver_rank);
    if(pt_offset == -1) return -1;

    dsem(pt_sem);
    for(; mem->page_tables[pt_offset].next_page_table_offset != NOT_IN_USE &&
            mem->page_tables[pt_offset].entry_count == ENTRY_PER_PT; 
        pt_offset = mem->page_tables[pt_offset].next_page_table_offset);

    if(mem->page_tables[pt_offset].entry_count == ENTRY_PER_PT) {
        int new_pt_offset = alloc_pt_pd(mem, receiver_rank);
        if(new_pt_offset != -1) {
            mem->page_tables[pt_offset].next_page_table_offset = new_pt_offset;
            mem->page_tables[new_pt_offset].entry_count = 0;
            mem->page_tables[new_pt_offset].next_page_table_offset = NOT_IN_USE;
            mem->page_tables[new_pt_offset].prev_page_table_offset = pt_offset;
            pt_offset = new_pt_offset;
        } else {
            pt_offset = -1;
        }
    }
    usem(pt_sem);
    return pt_offset;

}

/*
    get a empty page table and set it in use
    returns this page table's offset
*/
static int alloc_pt_pd(Mem* const mem, unsigned short receiver_rank) {
    
    sem_t* pd_region = osem("pd_region");

    int pt_offset = NOT_IN_USE;
    dsem(pd_region);
    for(int i = PT_COUNT - 1; i >= 0; --i) {
        if(mem->page_dir_entries[i].receiver_rank == NOT_IN_USE) {
            pt_offset = mem->page_dir_entries[i].page_table_offset;
            mem->page_dir_entries[i].receiver_rank = receiver_rank;
            break;
        }
    }
    usem(pd_region);

    return pt_offset;

}












/*
    loads page directory
*/
static void load_directory(Mem* const mem, const unsigned short proc_count) {

    for(int proc = 0; proc < PT_COUNT; ++proc) {
        mem->page_dir_entries[proc].page_table_offset = proc;

        if(proc < proc_count)
            mem->page_dir_entries[proc].receiver_rank = proc;
        else
            mem->page_dir_entries[proc].receiver_rank = NOT_IN_USE;
    }

}

/*
    loads page tables
*/
static void load_page_tables(Mem* const mem, const unsigned short proc_count) {

    for(int table = 0; table < PT_COUNT; ++table) {
        mem->page_tables[table].entry_count = 0;
        mem->page_tables[table].next_page_table_offset = NOT_IN_USE;
        mem->page_tables[table].prev_page_table_offset = NOT_IN_USE;
        for(int entry = 0; entry < ENTRY_PER_PT; ++entry) {
            mem->page_tables[table].entries[entry].page_table_entry_offset = entry;
            mem->page_tables[table].entries[entry].page_table_offset = table;
            mem->page_tables[table].entries[entry].valid = INVALID;
        }
    }

}

/*
    loads page's offsets
*/
static void load_page_offsets(Mem* const mem) {

    for(int offset = 0; offset < PAGE_COUNT; ++offset) {
        mem->page_offsets[offset] = offset;
    }

}







static int write_page(Mem* const mem, 
        unsigned short receiver_rank, 
        unsigned short sender_rank, 
        const void* buf, 
        unsigned int length,
        int tag,
        int order) 
{
    Page_table_entry* pte_ptr = alloc_page(mem, receiver_rank, sender_rank);
    if(pte_ptr == NULL) { perror("Can not write!\n"); return -1; }

    mem->pages[pte_ptr->page_offset].tag = tag;
    mem->pages[pte_ptr->page_offset].order = order;
    
    memcpy(mem->pages[pte_ptr->page_offset].data, buf, length);
    return 0;
}


static Page_table_entry* find_pt_entry(Mem* const mem, unsigned short receiver_rank, 
        unsigned short sender_rank, int tag, int order) 
{

    int pt_offset = find_page_table(mem, receiver_rank);
    
    char sem_name[DEFAULT_SEMAPHORE_NAME_LENGTH];
    gsn_pt(sem_name, receiver_rank);
    sem_t* pt_sem = osem(sem_name);

    Page_table_entry* pte = NULL;

    dsem(pt_sem);
    while(pt_offset != NOT_IN_USE) {
        for(int i = 0; i < ENTRY_PER_PT; ++i) {
            if(mem->page_tables[pt_offset].entries[i].sender_rank == sender_rank &&
            mem->page_tables[pt_offset].entries[i].receiver_rank == receiver_rank &&
            mem->page_tables[pt_offset].entries[i].valid == VALID &&
            mem->pages[mem->page_tables[pt_offset].entries[i].page_offset].tag == tag &&
            mem->pages[mem->page_tables[pt_offset].entries[i].page_offset].order == order) {
                pte = &(mem->page_tables[pt_offset].entries[i]);
                pt_offset = NOT_IN_USE;
                break;
            }
        }
        if(pt_offset != NOT_IN_USE)
            pt_offset = mem->page_tables[pt_offset].next_page_table_offset;
    }
    usem(pt_sem);

    return pte;

}


static int read_page(Mem* const mem,
        unsigned short receiver_rank,
        unsigned short sender_rank,
        void* buf,
        unsigned int length,
        int tag,
        int order) 
{
    Page_table_entry* pte = find_pt_entry(mem, receiver_rank, sender_rank, tag, order);
    if(pte == NULL) { perror("Can not find valid page!\n"); return -1; }

    // copy data to buffer
    memcpy(buf, mem->pages[pte->page_offset].data, length);

    // free page
    free_page(mem, pte);
    return 0;

}



