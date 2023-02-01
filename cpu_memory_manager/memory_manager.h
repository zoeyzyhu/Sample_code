#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <string>

#include "vm_arena.h"
#include "vm_pager.h"
#include "sentinelq.h"

// typedefs ========================================================

using PageId = unsigned int;

using Offset = unsigned int;

using page_book_processor_t = int (*)(PageId &, PageId const&);

// forward declaration
struct Process;

struct PageTraits;


// global variables ===============================================

extern unsigned int            total_virtual_pages;

extern unsigned int            total_memory_pages;

extern unsigned int            nullp;

extern Process                 *current_process;

extern PageTraits              *page_book; // phisical page book

extern PageId                  last_ppage; // page before clock head; need in delete

extern page_table_t*           parent_page_table_base_register;

// queues ==========================================================

extern SentinelQ<Process*>     *process_queue; // to switch context

extern SentinelQ<PageId>       *empty_ppage_ids;

extern SentinelQ<PageId>       *empty_spage_ids;

// structs ==========================================================

// ---------------------------------
// File-backed or swap-backed device
// ---------------------------------
struct BackupDevice {
    BackupDevice();
    ~BackupDevice();

    bool operator==(BackupDevice const& other);

    char                *filename;
    unsigned int        block;
};

// ----------------
// Manage a process
// ----------------
struct Process {
    Process(pid_t my_pid);

    pid_t               pid;
    page_table_t        page_table;
    PageId              vpage_counter; // total used; next available
    BackupDevice        devices[VM_ARENA_SIZE/VM_PAGESIZE]; // for read
};

// --------------------------------
// Manage a page in physical memory
// --------------------------------
struct PageTraits {
    PageTraits();
    ~PageTraits();

    void reset();

    PageId              next; // next page in use
    int                 referenced;
    int                 dirty;
    BackupDevice        device; // for evict

    SentinelQ<page_table_entry_t*>
                        *puser; // all vpages point to this ppage
};


// helper funcitons
extern BackupDevice const *device_to_find;


int find_fileblock_in_memory(PageId &curr, PageId const& prev);

int find_victim(PageId &curr, PageId const& prev);

int release_memory_page(PageId &curr, PageId const& prev);

PageId loop_page_book(page_book_processor_t func);

Process* find_process(pid_t pid);

void print_page_book();

// treat file name
PageId find_vpage(const char *str);

Offset find_offset(const char *str, PageId vpage);

void *get_physical_address(PageId vpage, Offset offset);

std::string fetch_string(char const* src);


// memory functions
int evict_physical_memory();

void* map_virtual_memory(PageId vpid, char const *filename, unsigned int block);

int map_physical_page(PageId vpid);


// state transition
void set_state_after_read(PageTraits *page);

void set_state_after_write(PageTraits *page);

void set_state_map_swap(PageTraits *page);

void set_state_map_file(PageTraits *page);

#endif
