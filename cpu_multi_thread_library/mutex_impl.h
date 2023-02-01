#ifndef MUTEX_IMPL_H
#define MUTEX_IMPL_H

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "mutex"
#include "cpu.h"
#include "sentinelq.h"

class mutex::impl {
public:
	impl();
    ~impl();

    void lock();
    void unlock();

private:
    std::unique_ptr<SentinelQ> wait_queue;
    bool lock_status; // True is free, False is unfree
    int64_t lock_owner; // -1 is own by nobody, non-negative otherwise

};

#endif
