#include <cstdio>
#include <cstdlib>
#include "sentinelq.h"

SentinelQ::SentinelQ(std::string name) :
    myname(name)
{
    //printf("I'm %s-Q constructor\n", myname.c_str());
    my.pcontext = nullptr;
    my.pstack = nullptr;
    next = nullptr;
    my_id = 0;
}

SentinelQ::~SentinelQ() {
    //printf("I'm %s-Q destructor\n", myname.c_str());
    while (next != nullptr) {
        pop();
    }
}

void SentinelQ::push(MyContext const& other, int64_t id) {
    SentinelQ* q = this;
    while (q->next != nullptr) {
        q = q->next;
    }
    q->next = new SentinelQ("new thread");
    q->next->my.pcontext = other.pcontext;
    q->next->my.pstack = other.pstack;
    q->next->next = nullptr;
    q->next->my_id = id;

}

MyContext& SentinelQ::top() {
    if (next != nullptr) {
        return next->my;
    } else {
        printf("empty Q!");
        exit(1);
    }
}

void SentinelQ::pop() {
    if (next != nullptr){
        SentinelQ* q = next;

        next = q->next;
        q->next = nullptr;
        delete q;
    } else {
        printf("empty Q!");
        exit(1);
    }
}


int64_t SentinelQ::getID() {
    return next->my_id;
}

bool SentinelQ::empty() {
    return (next == nullptr);

}

void set_and_swap(MyContext my, MyContext my_next, int64_t newID) {

    // switch to next job
    ctxRunning = my_next.pcontext;
    ctxStack = my_next.pstack;
    runningThreadID = newID;
    swapcontext(my.pcontext, my_next.pcontext);

}


//// Global vairable

std::unique_ptr<SentinelQ> ready_queue;
std::unique_ptr<SentinelQ> finish_queue;
std::unique_ptr<SentinelQ> living_queue;

ucontext_t *ctxRunning;
char *ctxStack;
int64_t runningThreadID;

//extern thread id record
int64_t next_thread_id;
