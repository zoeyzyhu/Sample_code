#ifndef CIRCULARQ_H
#define CIRCULARQ_H

#include "sentinelq.h"

template<typename T>
class CircularQ : public SentinelQ<T> {
public:
    CircularQ() {
        next_ = this;
    }

    ~CircularQ() {
        while (next_ != this)
            pop();
    }

    void push(T const& other) {
        CircularQ* q = this->next_;
        do {
            q = q->next_;
        } while(q != this->next_);

        q->next_ = new CircularQ;
        q->next_->data_ = other;
        q->next_->next_ = this->next_;
        ++count_;
    }

    void pop() {
        if (next_ != this) {
            CircularQ* q = next_;
            next_ = q->next_;
            q->next_ = q;
            delete q;
            --count_;
        }
    }

    void pop(T const& other) {
        CircularQ* q = this;
        do {
            if (q->next_->data_ == other) break;
            q = q->next_;
        } while (q->next_ != this->next_);
        q->pop();
    }

    void setNext(CircularQ *next) {
        next_ = next;
    }
};

#endif
