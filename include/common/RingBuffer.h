#pragma once

#include <vector>
#include <mutex>
#include <condition_variable>

template <typename T>
class RingBuffer {
public:
    RingBuffer(size_t size): put_ptr(0), take_ptr(0) {
        size_t power = 1;
        while (power < size) {
            power *= 2;
        }
        this->size = power;
        this->mask = this->size - 1;
        buffer.resize(this->size);
    }

    void put_unsafe(T val) {
        if (((put_ptr - take_ptr) & mask) == mask) return;
        buffer[put_ptr] = val;
        put_ptr = (put_ptr + 1) % mask;
    }

    void put(T val) {
        std::lock_guard<std::mutex> guard(mutex);
        put_unsafe(val);
        cv.notify_one();
    }

    void put_all(const std::vector<T>& values) {
        std::lock_guard<std::mutex> guard(mutex);
        for (auto& val : values) {
            put_unsafe(val);
        }
        cv.notify_one();
    }

    void clear() {
        std::lock_guard<std::mutex> guard(mutex);
        take_ptr = put_ptr;
    }

    bool try_take(T* val) {
        std::lock_guard<std::mutex> guard(mutex);
        if (take_ptr == put_ptr) return false;
        *val = buffer[take_ptr];
        take_ptr = (take_ptr + 1) % mask;
        return true;
    }

    void take_all(std::vector<T>& values) {
        std::lock_guard<std::mutex> guard(mutex);
        while (take_ptr != put_ptr) {
            values.push_back(buffer[take_ptr]);
            take_ptr = (take_ptr + 1) % mask;
        }
    }

    T take() {
        std::unique_lock<std::mutex> guard(mutex);
        if (take_ptr == put_ptr) {
            cv.wait(guard, [this] { return take_ptr != put_ptr; });
        }
        T val = buffer[take_ptr];
        take_ptr = (take_ptr + 1) % mask;
        guard.unlock();
        cv.notify_one();
        return val;
    }

private:
    size_t size;
    int8_t mask;
    std::mutex mutex;
    std::condition_variable cv;
    std::vector<T> buffer;
    int8_t put_ptr;
    int8_t take_ptr;
};

