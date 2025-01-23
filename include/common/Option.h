#pragma once


template <typename T>
class Option {
public:
    Option(T v) : v_(v), some_(true) {}
    Option() : some_(false) {}
    bool is_some() { return some_; }
    T&operator *() { return v_; }
    T* operator ->() { return &v_; }

    operator bool() const { return some_; }
private:
    T v_;
    bool some_;
};

