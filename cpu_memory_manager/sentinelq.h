#ifndef SENTINELQ_H
#define SENTINELQ_H

#include <stdexcept>

template<typename T>
class SentinelQ {
public:
  SentinelQ():
    next_(nullptr), count_(0)
  {}

  virtual ~SentinelQ() {
    clear();
  }

  // add copy construtor and assignment operato
  
  int getCount() const {
    int count = 0;
    SentinelQ* q = next_;
    while (q != nullptr) {
        count++;
        q = q->next_;
    }
    return count;
  }

  bool empty() {
    //return count_ == 0;
    return next_ == nullptr;
  }
  
  void push(T const& other) {
    SentinelQ* q = this;
    while (q->next_ != nullptr)
      q = q->next_;
    q->next_ = new SentinelQ;
    q->next_->data_ = other;
    q->next_->next_ = nullptr;
    ++count_;
  }

  T& top() {
    if (next_ != nullptr) {
      return next_->data_;
    }
    throw std::runtime_error("SentinelQ is empty");
    return data_;
  }

  void pop() {
    if (next_ != nullptr) {
      SentinelQ* q = next_;
      next_ = q->next_;
      q->next_ = nullptr;
      delete q;
      --count_;
    }
  }

  void pop(T const& other) {
    SentinelQ *q = this;
    while (q->next_ != nullptr) {
        if (q->next_->data_ == other) {
            break;
        }
        q = q->next_;
    }

    if (q->next_ != nullptr)
        q->pop();
  }

  SentinelQ* getNext() {
    return next_;
  }

  T& getData() {
    return data_;
  }

  void clear() {
    while (next_ != nullptr)
      pop();
  }

protected:
  T         data_;
  SentinelQ *next_;
  int       count_;
};

#endif
