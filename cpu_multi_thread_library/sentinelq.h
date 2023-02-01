#ifndef SENTINELQ_H
#define SENTINELQ_H

#include <ucontext.h>
#include <memory>
#include <string>
#include "cv.h"
#include "mutex.h"

struct MyContext {
    ucontext_t *pcontext;
    char *pstack;
};

class SentinelQ {
public:
    std::string myname;

    SentinelQ *next;
    MyContext my;
    int64_t my_id;

    SentinelQ(std::string name);
    ~SentinelQ();
    
    void push(MyContext const& other, int64_t id);
    MyContext& top();
    int64_t getID();
    void pop();
    bool empty();
};


void set_and_swap(MyContext my, MyContext my_next, int64_t newID); // interrupt must be disable when swap
void thread_switch(void);
void thread_clean(void);


//// Global vairable

extern std::unique_ptr<SentinelQ> ready_queue;
extern std::unique_ptr<SentinelQ> finish_queue;
extern std::unique_ptr<SentinelQ> living_queue;


//extern running thread information

extern ucontext_t *ctxRunning;
extern char *ctxStack;
extern int64_t runningThreadID;

//extern thread id record
extern int64_t next_thread_id;

#endif
