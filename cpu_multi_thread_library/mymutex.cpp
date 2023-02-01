#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "cpu.h"
#include "mutex_impl.h"

mutex::impl::impl() {

    wait_queue = std::make_unique<SentinelQ>("wait_queue_mutex");
    lock_owner = -1;
    lock_status = true;

}

mutex::impl::~impl() {

}

void mutex::impl::lock() {

    //std::cout << "Entering lock" << std::endl;
    if (!lock_status) {

        MyContext my;
        my.pcontext = ctxRunning;
        my.pstack = ctxStack;

        wait_queue->push(my, runningThreadID);

        // pop out the next job to run
        MyContext my_next = ready_queue->top();
        int64_t my_next_id = ready_queue->getID();
        ready_queue->pop();

        // switch to next job
        set_and_swap(my, my_next, my_next_id);

    } else {
        lock_status = false;
        lock_owner = runningThreadID;
    }

}

void mutex::impl::unlock() {

    //cpu::interrupt_disable();
    
    //std::cout << "Unlocking" << std::endl;
    if(lock_owner != runningThreadID) {
        cpu::interrupt_enable();
        throw std::runtime_error("Try to release a lock it do not held");
    }

    if (wait_queue->next != nullptr) {

        MyContext my_next_withLock = wait_queue->top();
        int64_t my_next_withLock_id = wait_queue->getID();
        wait_queue->pop();

        lock_owner = my_next_withLock_id;

        ready_queue->push(my_next_withLock, my_next_withLock_id);
    } else {
        lock_owner = -1;
        lock_status = true;
    }

    //cpu::interrupt_enable();

}

mutex::mutex() {
    this->impl_ptr = new impl;
}

mutex::~mutex() {
}

void mutex::lock() {
    cpu::interrupt_disable();

    this->impl_ptr->lock();

    cpu::interrupt_enable();
}

void mutex::unlock() {
    cpu::interrupt_disable();

    this->impl_ptr->unlock();

    cpu::interrupt_enable();
}


