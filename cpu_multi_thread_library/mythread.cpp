#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "thread.h"
#include "sentinelq.h"
#include "cv_impl.h"

bool current_thread_alive() {
    SentinelQ *p = living_queue->next;
    while (p != nullptr) {
        if (p->my_id == runningThreadID) {
            return true;
        }
        p = p->next;
    }
    return false;
}

class thread::impl {
public:
    mutex my_mutex;
    cv my_queue;
    bool incomplete;
};

void body_wrapper(void *user_body, void *user_arg, void *pthread) {

    cpu::interrupt_enable();

    // 1. run user code
    thread_startfunc_t body = (thread_startfunc_t)user_body;
    std::cout << "run user code now" << std::endl;
    body(user_arg);

    // 2. put to finish queue and move on
    cpu::interrupt_disable();

    std::cout << "finished; move to next thread" << std::endl;

    // push my context, stack pointer to finish queue
    MyContext my;
    my.pcontext = ctxRunning;
    my.pstack = ctxStack;

    finish_queue->push(my, runningThreadID);

    thread * pd = (thread *) pthread;
    //std::cout << "I'm " << runningThreadID << std::endl;
    if (current_thread_alive()) {
        pd->impl_ptr->incomplete = false;
        pd->impl_ptr->my_queue.impl_ptr->signal();
    }

    // switch to next threads in the ready queue
    if (ready_queue->next != nullptr) {

        MyContext my_next = ready_queue->top();
        runningThreadID = ready_queue->getID();
        ready_queue->pop();

        ctxRunning = my_next.pcontext;
        ctxStack = my_next.pstack;
        setcontext(my_next.pcontext);
    }

    cpu::interrupt_enable();
}


thread::thread(thread_startfunc_t body, void* arg) {

    //std::cout << "thread begin" << std::endl;

    // initiate the complete status
    impl_ptr = new impl;
    impl_ptr->incomplete = true;

    // generate new context and stack pointer
    ucontext_t *job = new ucontext_t;
    char *stack = new char [STACK_SIZE];

    // initiate context
    job->uc_stack.ss_sp = stack;
    job->uc_stack.ss_size = STACK_SIZE;
    job->uc_stack.ss_flags = 0;
    job->uc_link = nullptr;

    makecontext(job, (void(*)())body_wrapper, 3, body, arg, this);

    // push to ready queue
    MyContext my;
    my.pcontext = job;
    my.pstack = stack;
    int64_t my_id = next_thread_id;
    next_thread_id++;
    ready_queue->push(my, my_id);
    living_queue->push(my, my_id);

    // yield to CPU
    thread::yield();
}


thread::~thread() {
    SentinelQ *p = living_queue->next;
    while (p != nullptr) {
        if (runningThreadID == p->my_id) {
            p->my_id = living_queue->next->my_id;
            living_queue->pop();
            break;
        }
        p = p->next;
    }
        
    //std::cout << runningThreadID << " out" << std::endl;
    //std::cout << "thread end" << std::endl;
    delete impl_ptr;
}


void thread::yield() {

    cpu::interrupt_disable();

    std::cout << "yield to cpu" << std::endl;

    // 1. switch to next ready thread
    if (ready_queue->next != nullptr) {
        thread_switch();
    } 

    // 2. clean finish queue on the way
    thread_clean();

    cpu::interrupt_enable();
}

void thread::join() {
    //std::cout << "Thread join" << std::endl;
    impl_ptr->my_mutex.lock();

    while (impl_ptr->incomplete) {
        std::cout << "I'm waiting" << std::endl;
        impl_ptr->my_queue.wait(impl_ptr->my_mutex);
    }

    impl_ptr->my_mutex.unlock();
}

void thread_clean() {
    while (finish_queue->next != nullptr) {

        std::cout << "## Thread cleaning! ##" << std::endl;
        
        // get the context of finished thread to clean
        MyContext to_clean = finish_queue->top();
        finish_queue->pop();
        
        // delete
        if (to_clean.pstack != nullptr) {

            std::cout << "- clean something"<< std::endl;

            delete [] to_clean.pstack;
            delete to_clean.pcontext;
        } else {

            std::cout << "- nothing to clean" << std::endl;
        }
    }
}

void thread_switch() {
    // save current thread on ready queue
    MyContext my;
    my.pcontext = ctxRunning;
    my.pstack = ctxStack;
    ready_queue->push(my, runningThreadID);

    // take next thread off ready queue
    MyContext my_next = ready_queue->top();
    runningThreadID = ready_queue->getID();

    ready_queue->pop();

    // update next thread to run
    ctxRunning = my_next.pcontext;
    ctxStack = my_next.pstack;

    // switch to new thread
    swapcontext(my.pcontext, my_next.pcontext);

}
