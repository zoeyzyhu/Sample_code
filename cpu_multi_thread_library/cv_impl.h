#ifndef CV_IMPL_H
#define CV_IMPL_H

#include "sentinelq.h"

class cv::impl {
public:
	impl();
    ~impl();

    void wait(mutex& lock);             // wait on this condition variable
    void signal();                      // wake up one thread on this condition variable
    void broadcast();                   // wake up all threads on this condition variable

private:
    std::unique_ptr<SentinelQ> wait_queue;
};

#endif
