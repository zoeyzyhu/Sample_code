#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "sentinelq.h"

using namespace std;

#define STACK_SIZE 1999

ucontext_t ctxA, ctxB, ctxMain;

void funcA(int arg) {
    std::cout << "Begin funcA with arg " << arg << std::endl;

    swapcontext(&ctxA, &ctxB);

    std::cout << "Exit funcA" << std::endl;
}

void funcB(double arg) {
    std::cout << "Begin funcB with arg " << arg << std::endl;

    swapcontext(&ctxB, &ctxA);

    std::cout << "Exit funcB" << std::endl;
}

int main() {
    std::cout << "Begin main" << std::endl;

    char stackA[STACK_SIZE];
    char stackB[STACK_SIZE];

    int a = 2;
    int b = 3;

    getcontext(&ctxA);
    ctxA.uc_stack.ss_sp = stackA;
    ctxA.uc_stack.ss_size = STACK_SIZE;
    ctxA.uc_link = &ctxMain;
    makecontext(&ctxA, (void(*)())funcA, 1, a);

    getcontext(&ctxB);
    ctxB.uc_stack.ss_sp = stackB;
    ctxB.uc_stack.ss_size = STACK_SIZE;
    ctxB.uc_link = &ctxMain;
    makecontext(&ctxB, (void(*)())funcB, 1, b);

    SentinelQ* queue = new SentinelQ;

    queue->push(&ctxA);
    queue->push(&ctxB);

    while (queue->next != nullptr) {
        ucontext_t* p = queue->pop();
        swapcontext(&ctxMain, p);
    }

    delete queue;
    std::cout << "Exit main" << std::endl;
}
