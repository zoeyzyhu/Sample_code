#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "cpu.h"
#include "cv.h"

// Test 4: Test thread yield and join
// 0 threads

enum{
    async = 1,
    not_async = 0
};

enum{
    sync = 1,
    not_sync = 0
};

void t_func_A(void *arg) {
    std::cout << "A yielding to CPU boot func" << std::endl;
    thread::yield();

    std::cout << "A yielding to CPU boot func again" << std::endl;
    thread::yield();
    
    std::cout << "A finish" << std::endl;
}

void cpu_boot_func(void *arg) {
    int *i = (int*)arg;
    std::cout << "I'm CPU boot function with arg: " << *i << std::endl;
    thread t(&t_func_A, nullptr);

    std::cout << "CPU boot func wait on func_A" << std::endl;
    t.join();

    std::cout << "CPU boot func wait on func_A again" << std::endl;
    t.join();

    std::cout << "CPU boot func finish" << std::endl;
}

int main(){
    unsigned int num_cpus = 1;
    int arg = 1;
    int random_seed = 0;
    cpu::boot(num_cpus, cpu_boot_func, (void *)&arg, not_async, not_sync, random_seed);
}
