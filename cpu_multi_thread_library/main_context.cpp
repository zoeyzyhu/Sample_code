#include <ucontext.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#define STACK_SIZE 9999

ucontext_t cxtA, cxtB, cxtMain;

void funcA(int a) {
    std::cout << "Begin funcA with arg " << a << std::endl;

    swapcontext(&cxtA, &cxtB);

    std::cout << "Exit funcA" << std::endl;
}

void funcB(int b) {
    std::cout << "Begin funcB with arg " << b << std::endl;

    swapcontext(&cxtB, &cxtA);

    std::cout << "Exit funcB" << std::endl;
}

int main() {
    std::cout << "Begin main" << std::endl;

    char stackA[STACK_SIZE];
    char stackB[STACK_SIZE];

    int a = 2;
    int b = 3;

    getcontext(&cxtA);
    cxtA.uc_stack.ss_sp = stackA;
    cxtA.uc_stack.ss_size = STACK_SIZE;
    cxtA.uc_link = &cxtMain;
    makecontext(&cxtA, (void(*)())funcA, 1, a);

    getcontext(&cxtB);
    cxtB.uc_stack.ss_sp = stackB;
    cxtB.uc_stack.ss_size = STACK_SIZE;
    cxtB.uc_link = &cxtA;
    makecontext(&cxtB, (void(*)())funcB, 1, b);

    swapcontext(&cxtMain, &cxtB);

    std::cout << "Exit main" << std::endl;
}
