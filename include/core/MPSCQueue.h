#pragma once

#include <common/Option.h>
#include <common/Debug.h>
#include <mutex>
#include <vector>

template <typename Value>
class MPSCQueue {
public:
    explicit MPSCQueue(size_t size) : internal_(size) {
        for (size_t i = 0; i < internal_.size(); i++) {
            internal_[i] = nullptr;
        }
    }

    Option<Value> get() {
        if (tail_ != head_) {
            Value elem = internal_[tail_];
            internal_[tail_] = nullptr;
            tail_ = (tail_ + 1) % internal_.size();
            return Option<Value>(elem);
        } else {
            return Option<Value>();
        }

    }
    void put(Value val) {
        internal_[head_] = val;
        head_ = (head_ + 1) % internal_.size();
    }

private:
    size_t head_ = 0;
    size_t tail_ = 0;
    std::vector<Value> internal_;
};

