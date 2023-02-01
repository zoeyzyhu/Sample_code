#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "cpu.h"
#include "cv.h"

// Test 5: Test thread yield and lock
// 2 threads

enum{
    async = 1,
    not_async = 0
};

enum{
    sync = 1,
    not_sync = 0
};

mutex test_lock;

void t_func(void *arg) {
    char *i = (char*)arg;
    std::cout << "Function " << *i << " aquiring lock..." << std::endl;
    test_lock.lock();
    thread::yield();
    std::cout << "Function " << *i << " critical section running..." << std::endl;
    thread::yield();
    test_lock.unlock();
    std::cout << "Function " << *i << " finished" << std::endl;
}

void cpu_boot_func(void *arg) {
    int *i = (int*)arg;
    std::cout << "I'm CPU boot function with arg: " << *i << std::endl;
    thread tA(&t_func, (void*) "A");
    thread tB(&t_func, (void*) "B");
}

int main(){
    unsigned int num_cpus = 1;
    int arg = 482;
    int random_seed = 0;
    cpu::boot(num_cpus, cpu_boot_func, (void *)&arg, not_async, not_sync, random_seed);
}
