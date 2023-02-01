#include <memory>
#include <vector>
#include <iostream>
#include <cstring>

#include "memory_manager.h"

// global variables ===============================================

unsigned int            total_virtual_pages;

unsigned int            total_memory_pages;

unsigned int            nullp;

Process                 *current_process;

PageTraits              *page_book; // phisical page book

PageId                  last_ppage; // page before clock head; need in delete

page_table_t*           parent_page_table_base_register;

// queues ==========================================================

SentinelQ<Process*>     *process_queue; // to switch context

SentinelQ<PageId>       *empty_ppage_ids;

SentinelQ<PageId>       *empty_spage_ids;


// devices --------------------------------
BackupDevice::BackupDevice():
    filename(nullptr), block(0)
{}

BackupDevice::~BackupDevice()
{
    if (filename != nullptr)
        delete [] filename;
}

bool BackupDevice::operator==(BackupDevice const& other)
{
    if (block != other.block) return false;
    if ((filename == nullptr) && (other.filename == nullptr)) return true;
    if ((filename != nullptr) && (other.filename == nullptr)) return false;
    if ((filename == nullptr) && (other.filename != nullptr)) return false;
    if (strcmp(filename, other.filename) == 0) return true;
    return false;
}


// process --------------------------------
Process::Process(pid_t my_pid):
    pid(my_pid), vpage_counter(0)
{
    for (PageId i = 0; i < total_virtual_pages; ++i) {
        page_table.ptes[i].ppage = nullp;
        page_table.ptes[i].read_enable = 0;
        page_table.ptes[i].write_enable = 0;
    }
}

// physical page --------------------------
PageTraits::PageTraits():
    next(nullp), referenced(0), dirty(0)
{
    puser = new SentinelQ<page_table_entry_t*>();
}

PageTraits::~PageTraits()
{
    delete puser;
}

void PageTraits::reset() {
    next = nullp; // nullp: non-resident; otherwise: resident
    referenced = 0;
    dirty = 0;
    if (device.filename != nullptr) {
        delete [] device.filename;
        device.filename = nullptr;
    }
    device.block = 0;
    puser->clear();
}

// helper functions ================================================

BackupDevice const *device_to_find;

int find_fileblock_in_memory(PageId &curr, PageId const& prev) {
    if (page_book[curr].device == *device_to_find) {
        return 1;
    }

    return 0;
}

int find_victim(PageId &curr, PageId const& prev) {
    last_ppage = prev;
    // do not evict 0 page and set bits
    if (curr == 0) return 0;
    if (page_book[curr].referenced == 0) {
        return 1;
    } else {
        page_book[curr].referenced = 0;

        // set r/w bit of vpage pusers (of this ppage)
        SentinelQ<page_table_entry_t*> *p = page_book[curr].puser->getNext();
        while (p != nullptr) {
            page_table_entry_t *entry = p->getData();
            entry->read_enable = 0;
            entry->write_enable = 0;
            p = p->getNext();
        }
    }
    return 0;
}

// free memory in vm_destroy()
int release_memory_page(PageId &curr, PageId const& prev) {
    // never release 0 page
    if (curr == 0) return 0;
    if (page_book[curr].puser->empty()) {
        if (last_ppage == curr)
            last_ppage = prev;
        empty_ppage_ids->push(curr);
        page_book[prev].next = page_book[curr].next;
        page_book[curr].reset();
        curr = prev;
    }

    return 0;
}

PageId loop_page_book(page_book_processor_t func) {
    // std::cout << "loop_page_book" << std::endl;
    PageId prev = last_ppage;
    // empty page book
    if (prev == nullp) return nullp;

    PageId curr = page_book[last_ppage].next; //clock head

    // std::cout << curr;
    do {
        int status = func(curr, prev);
        if (status) break;
        prev = curr;
        curr = page_book[curr].next;
        // std::cout << " -> " << curr;
    } while (curr != page_book[last_ppage].next);
    // std::cout << std::endl;

    return curr;
}

// used in fork, switch
Process* find_process(pid_t pid) {
    SentinelQ<Process*> *q = process_queue->getNext();
    while (q != nullptr) {
        if (q->getData()->pid == pid) {
            // std::cout << "find child process" << std::endl;
            return q->getData();
        }
        q = q->getNext();
    }
    return nullptr;
}

PageId find_vpage(const char *str) {
    return (str - (char*)VM_ARENA_BASEADDR)/VM_PAGESIZE;
}

Offset find_offset(const char *str, PageId vpage) {
    return (str - (char*)VM_ARENA_BASEADDR) - vpage*VM_PAGESIZE;
}

void *get_physical_address(PageId vpage, Offset offset) {
    return (char*)vm_physmem + offset +
        current_process->page_table.ptes[vpage].ppage*VM_PAGESIZE;
}

std::string fetch_string(char const* src) {
    char dst[VM_PAGESIZE+1];

    if (vm_fault(src, 0) == -1) {
        return "";  // filename illegal
    }
    PageId vpage = find_vpage(src);
    Offset offset = find_offset(src, vpage);
    Offset nchar_max = VM_PAGESIZE - offset;

    strncpy(dst, (char*)get_physical_address(vpage, offset), nchar_max);
    dst[nchar_max] = '\0';

    if (strlen(dst) < nchar_max) { // fully in one page
        return dst;
    } else { // has something in the second page
        return std::string(dst) + fetch_string(
            (char*)VM_ARENA_BASEADDR + (vpage+1)*VM_PAGESIZE);
    }
}

void print_page_book() {
    std::cout << "Page Book:" << std::endl;
    for (PageId i = 0; i < total_memory_pages; ++i) {
         std::cout << page_book[i].next << " ";
    }
    std::cout << std::endl;
    std::cout << "Head at: " << last_ppage << std::endl;
    std::cout << "Page users:" << std::endl;
    for (PageId i = 0; i < total_memory_pages; ++i) {
        std::cout << i << ": " << std::endl;
        SentinelQ<page_table_entry_t*> *p = page_book[i].puser->getNext();
        while (p != nullptr) {
            std::cout << p->getData() << " | " << p->getData()->ppage << std::endl;
            p = p->getNext();
        }
    }
}

// state transition -----------------------------
void set_state_after_read(PageTraits *page) {
    page->referenced = 1;

    SentinelQ<page_table_entry_t*> *p = page->puser->getNext();
    while (p != nullptr) {
        page_table_entry_t *entry = p->getData();
        entry->read_enable = 1;
        if(page->dirty == 1)
            entry->write_enable = 1;
        p = p->getNext();
    }
}

void set_state_after_write(PageTraits *page) {
    page->referenced = 1;
    page->dirty = 1;

    SentinelQ<page_table_entry_t*> *p = page->puser->getNext();
    while (p != nullptr) {
        page_table_entry_t *entry = p->getData();
        entry->read_enable = 1;
        entry->write_enable = 1;
        p = p->getNext();
    }
}
