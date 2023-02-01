#include <iostream>
#include <cstdio>
#include <memory>

#include "cpu.h"
#include "sentinelq.h"

// global memory
ucontext_t first_job;
char first_stack[STACK_SIZE];

// body_wrapper defined in mythread.cpp
void body_wrapper(void *user_body, void *user_arg, void *pthread);

void handle_timer(void) {
    cpu::interrupt_disable();
    std::cout << "-------Interrupt!" << std::endl;
    thread_switch();
    cpu::interrupt_enable();
}

void cpu::init(thread_startfunc_t body, void *arg) {
    
    if (arg != nullptr) {

        try {
            printf("CPU okay \n");

            // initialize queues
            ready_queue = std::make_unique<SentinelQ>("ready");
            finish_queue = std::make_unique<SentinelQ>("finish");
            living_queue = std::make_unique<SentinelQ>("living");
            // queue deallocation is managed my unique_ptr

            // initialize time handler
            interrupt_vector_table[0] = handle_timer; 

            // make context for first job (on stack)
            first_job.uc_stack.ss_sp = first_stack;
            first_job.uc_stack.ss_size = STACK_SIZE;
            first_job.uc_stack.ss_flags = 0;
            first_job.uc_link = nullptr;

            makecontext(&first_job, (void(*)())body_wrapper, 3, body, arg, nullptr);
            // save context and stack pointer
            ctxRunning = &first_job;
            ctxStack = nullptr;
            
            runningThreadID = next_thread_id;
            next_thread_id++;
           
            // run first job
            std::cout << "set first job" << std::endl;
            //cpu::interrupt_enable();
            setcontext(&first_job);
            
        }
        catch (std::bad_alloc& ba) {
            std::cerr << "bad_alloc caught: " << ba.what() << '\n';
        }

    } else {
        printf("CPU not okay\n");
    }

    // cpu::init does not return
    std::cout << "I shall never be here" << std::endl;
}
