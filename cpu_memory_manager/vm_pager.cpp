#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cstring>

#include "vm_pager.h"
#include "memory_manager.h"

void vm_init(unsigned int memory_pages, unsigned int swap_blocks)
{
    nullp = memory_pages;

    char *p = (char*)vm_physmem;

    // set first page to 0s
    memset(p, 0, VM_PAGESIZE);

    page_book = new PageTraits [memory_pages];

    process_queue = new SentinelQ<Process*>();

    empty_ppage_ids = new SentinelQ<PageId>();

    empty_spage_ids = new SentinelQ<PageId>();

    for (PageId i = 0; i < memory_pages; ++i) {
        empty_ppage_ids->push(i);
    }
 
    for (PageId i = 0; i < swap_blocks; ++i) {
        empty_spage_ids->push(i);
    }

    total_virtual_pages = VM_ARENA_SIZE/VM_PAGESIZE;

    total_memory_pages = memory_pages;
    
    // pop up zero page
    empty_ppage_ids->pop();
    last_ppage = 0;
    page_book[0].next = 0;
    page_book[0].referenced = 1;
    page_book[0].dirty = 0;
}

int vm_create(pid_t parent_pid, pid_t child_pid) 
{
    Process *child = new Process(child_pid);

    // find parent process
    Process *parent = find_process(parent_pid);

    if (parent != nullptr) { // parent in queue
        child->vpage_counter = parent->vpage_counter;
        for (PageId i = 0; i < parent->vpage_counter; ++i) {
            child->page_table.ptes[i] = parent->page_table.ptes[i];
            if (parent->devices[i].filename != nullptr) {
                child->devices[i].filename = new char [strlen(parent->devices[i].filename) + 1];
                strcpy(child->devices[i].filename, parent->devices[i].filename);
                child->devices[i].block = parent->devices[i].block;
            }

            if (child->devices[i].filename == nullptr) {
                if (empty_spage_ids->empty()) {
                    return -1;
                } else {
                    child->devices[i].block = empty_spage_ids->top();
                    empty_spage_ids->pop();
                }
            }
        }
    } else { // parent not in queue
        child->vpage_counter = 0;
    }

    // add page book user
    for (PageId vpid = 0; vpid < child->vpage_counter; ++vpid) {
        if (child->page_table.ptes[vpid].ppage != nullp) {
            PageId ppage = child->page_table.ptes[vpid].ppage;
            page_book[ppage].puser->push(child->page_table.ptes + vpid);
        }
    }

    process_queue->push(child);

    return 0;
}

void vm_switch(pid_t pid) 
{
    parent_page_table_base_register = page_table_base_register;

    current_process = find_process(pid);
    page_table_base_register = &current_process->page_table;
}

int vm_fault(const void* addr, bool write_flag) 
{
    Process *current = current_process;

    PageId fault_id = ((char*)addr - (char*)VM_ARENA_BASEADDR)/VM_PAGESIZE;

    // fault outside of valid virtual pages
    if ((fault_id >= current->vpage_counter) ||
        (fault_id < 0)) {
        return -1;
    }

    // fault inside arena ----------->
    page_table_entry_t *entry = current->page_table.ptes + fault_id;
    BackupDevice* device = current->devices + fault_id;

    if (entry->ppage == nullp) { // not in memory
        if (map_physical_page(fault_id) == -1) return -1;

        // load memory
        char *buf = (char*)vm_physmem + entry->ppage*VM_PAGESIZE;
        if (file_read(device->filename, device->block, buf) == -1) return -1;

        if (write_flag == 1)
            set_state_after_write(page_book + entry->ppage);
        else
            set_state_after_read(page_book + entry->ppage);

        return 0;
    }

    // in memory

    if (write_flag == 1) {  // trap by write
        if (entry->ppage == 0) { // point at 0 page
            if (map_physical_page(fault_id) == -1) return -1;

            // set memory to zero
            char *buf = (char*)vm_physmem + entry->ppage*VM_PAGESIZE;
            memset(buf, 0, VM_PAGESIZE);
        }

        set_state_after_write(page_book + entry->ppage);
    } else { // trap by read
        set_state_after_read(page_book + entry->ppage);
    }

    return 0;
}

void vm_destroy()
{
    Process *current = current_process;

    for (PageId i = 0; i < current->vpage_counter; ++i) {
        // release swap blocks
        if (current->devices[i].filename == nullptr) {
            empty_spage_ids->push(current->devices[i].block);
        }

        // release memory pages
        PageId ppage = current->page_table.ptes[i].ppage;
        if (ppage != nullp) {
            page_book[ppage].puser->pop(current->page_table.ptes + i);
        }
    }

    // reset last page to 0
    //last_ppage = 0;
    for (unsigned int i = 0; i < total_memory_pages; ++i)
        loop_page_book(release_memory_page);

    process_queue->pop(current);

    delete current;

    page_table_base_register = parent_page_table_base_register;

    //print_page_book();
}

void* vm_map(const char *filename, unsigned int block)
{
    std::string str;

    if (filename != nullptr) {
        // illegal filename?
        str = fetch_string(filename);

        // if the first character is outside arena
        if (filename < (char*)VM_ARENA_BASEADDR)
            return nullptr;

        // if the last character (including \0) is outside arena
        if (filename + str.size() + 1 >= (char*)VM_ARENA_BASEADDR + VM_ARENA_SIZE)
            return nullptr;
    }

    Process *current = current_process;

    if (current->vpage_counter == total_virtual_pages)
        return nullptr;


    if (empty_spage_ids->empty() && (filename == nullptr)) {
        return nullptr;
    }

    void *p = map_virtual_memory(current->vpage_counter, filename, block);

    if (p != nullptr)
        ++current->vpage_counter;

    //print_page_book();

    return p;
}

int evict_physical_memory()
{
    PageId ppage = loop_page_book(find_victim);

    if (page_book[ppage].dirty) {
        char *buf = (char*)vm_physmem + ppage*VM_PAGESIZE;
        if (file_write(page_book[ppage].device.filename,
                       page_book[ppage].device.block, buf) == -1)
            return -1;
    }

    SentinelQ<page_table_entry_t*> *p = page_book[ppage].puser->getNext();

    while (p != nullptr) {
        page_table_entry_t *entry = p->getData();
        entry->ppage = nullp;
        entry->read_enable = 0;
        entry->write_enable = 0;
        p = p->getNext();
    }

    empty_ppage_ids->push(ppage);

    page_book[last_ppage].next = page_book[ppage].next;

    page_book[ppage].reset();

    return 0;
}

void* map_virtual_memory(PageId vpid, char const *filename, unsigned int block)
{
    Process *current = current_process;
    page_table_entry_t *entry = current->page_table.ptes + vpid;
    BackupDevice* device = current->devices + vpid;

    if (filename == nullptr) {
        
        if (empty_spage_ids->empty())
            return nullptr;

        entry->ppage = 0;

        page_book[0].puser->push(entry);

        device->filename = nullptr;
        device->block = empty_spage_ids->top();
        empty_spage_ids->pop();

        entry->read_enable = 1;
        entry->write_enable = 0;

    } else { // file backed
        std::string str;
        str = fetch_string(filename);

        device->filename = new char [str.size() + 1];
        strcpy(device->filename, str.c_str());
        device->block = block;

        // try to find file block in memory
        device_to_find = device;
        PageId ppage = loop_page_book(find_fileblock_in_memory);

        // find the memory page
        if (page_book[ppage].device == *device) {
            page_table_entry_t *parent = page_book[ppage].puser->getNext()->getData();

            // set virtual page table entry
            entry->ppage = ppage;
            entry->read_enable = parent->read_enable;
            entry->write_enable = parent->write_enable;

            page_book[entry->ppage].puser->push(entry);

        } else  { // did not find memory page
            entry->ppage = nullp;
            entry->read_enable = 0;
            entry->write_enable = 0;
        }
    }

    // return the virtual memory address
    return (char*)VM_ARENA_BASEADDR + vpid*VM_PAGESIZE;
}

int map_physical_page(PageId vpid) {
    // 1. return if not enough memory 
    if (empty_ppage_ids->empty()) {
        if (evict_physical_memory() == -1) return -1;
    }

    Process *current = current_process;

    // 2. get an available physmem page
    PageId ppage = empty_ppage_ids->top();
    empty_ppage_ids->pop();

    // 3. add to end of clock queue
    PageId first_ppage = page_book[last_ppage].next;
    page_book[last_ppage].next = ppage;
    page_book[ppage].next = first_ppage;
    last_ppage = ppage;

    // 4. set filename and block
    if (current->devices[vpid].filename != nullptr) {

        int len = strlen(current->devices[vpid].filename);
        page_book[ppage].device.filename = new char [len + 1];
        strcpy(page_book[ppage].device.filename, current->devices[vpid].filename);

    } else {
        page_book[ppage].device.filename = nullptr;
    }
    page_book[ppage].device.block = current->devices[vpid].block;

    // 5. detach current ppage
    page_table_entry_t *entry = current->page_table.ptes + vpid;

    if (entry->ppage != nullp) {
        page_book[entry->ppage].puser->pop(entry);
    }

    // 6. attach to new ppage
    entry->ppage = ppage;
    page_book[ppage].puser->push(entry);

    // 7. add users of the same loading file and block
    if (page_book[ppage].device.filename != nullptr) {

        SentinelQ<Process*> *p = process_queue->getNext();
        while (p != nullptr) {
            Process *my = p->getData();
            for (PageId i = 0; i < my->vpage_counter; ++i) {
                if (my->page_table.ptes[i].ppage != nullp)
                    continue;
                if (my->devices[i] == page_book[ppage].device) {
                    my->page_table.ptes[i].ppage = ppage;
                    page_book[ppage].puser->push(my->page_table.ptes + i);
                }
            }
            p = p->getNext();
        }
    }

    return 0;
}
