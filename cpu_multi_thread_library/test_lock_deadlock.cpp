#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "cpu.h"
#include "mutex.h"

enum{
    async = 1,
    not_async = 0
};

enum{
    sync = 1,
    not_sync = 0
};

mutex test_lock;

int j = 11;

void t_myfunc2(void *arg) {

    test_lock.lock();

    int *j = (int*)arg;
    //std::cout << *j << std::endl;
    std::cout << "I'm function 2 with arg: " << *j << std::endl;
    //std::cout << "I'm function 2 with arg: " << std::endl;
}

void t_myfunc3(void *arg) {
    
    std::cout << "function 3 start" << std::endl;
    test_lock.lock();

    int *j = (int*)arg;
    //std::cout << *j << std::endl;
    std::cout << "I'm function 3 with arg: " << *j << std::endl;
    //std::cout << "I'm function 2 with arg: " << std::endl;
}


void t_myfunc(void *arg) {
    int *i = (int*)arg;
    std::cout << "I'm function 1 with arg: " << *i << std::endl;

    thread t2(&t_myfunc2, &j);

    thread t3(&t_myfunc3, &j);

}

int main(){
    unsigned int num_cpus = 1;
    int arg = 10;
    int random_seed = 0;
    cpu::boot(num_cpus, t_myfunc, (void *)&arg, not_async, not_sync, random_seed);
}

