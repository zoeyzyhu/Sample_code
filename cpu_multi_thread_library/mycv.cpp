#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>

#include "cv.h"
#include "thread.h"
#include "cpu.h"
//#include "sentinelq.h"
#include "mutex_impl.h"
#include "cv_impl.h"

cv::impl::impl() {
    wait_queue = std::make_unique<SentinelQ>("wait_queue_cv");
}

cv::impl::~impl() {
    
}

void cv::impl::wait(mutex& lock) {

    //// may interrupt disable here

    lock.impl_ptr->unlock();

    if (ready_queue->next != nullptr) {

        // save myself to ready queue
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
        cpu::interrupt_enable_suspend();
    }

    lock.impl_ptr->lock();

}

void cv::impl::signal() {
    if (wait_queue->next != nullptr) {
        MyContext readyOne = wait_queue->top();
        int64_t readyOne_id = wait_queue->getID();
        wait_queue->pop();

        ready_queue->push(readyOne, readyOne_id);
    }
}

void cv::impl::broadcast() {
    while (wait_queue->next != nullptr) {
        MyContext readyOne = wait_queue->top();
        int64_t readyOne_id = wait_queue->getID();
        wait_queue->pop();

        ready_queue->push(readyOne, readyOne_id);
    }
}

cv::cv() {
    impl_ptr = new impl;
}

cv::~cv() {
    //this->impl_ptr->~impl();
    delete impl_ptr;
}

void cv::wait(mutex& lock) {
    cpu::interrupt_disable();

    impl_ptr->wait(lock);

    cpu::interrupt_enable();
}

void cv::signal() {
    cpu::interrupt_disable();

    impl_ptr->signal();

    cpu::interrupt_enable();
}

void cv::broadcast() {
    cpu::interrupt_disable();

    impl_ptr->broadcast();

    cpu::interrupt_enable();
}
