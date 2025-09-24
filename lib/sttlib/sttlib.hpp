#pragma once
#include <vector>
#include <ostream>
#include <iostream>
class AtomicString {
public:
    AtomicString(const std::string & str);
    void string() const;
    AtomicString & operator=(const std::string & rhs);
    AtomicString & operator+=(const std::string & rhs);
    bool operator==(const std::string & rhs) const;
    bool operator!=(const std::string & rhs) const;
    bool operator<(const std::string & rhs) const;
    bool operator>(const std::string & rhs) const;
};

class AtomicFloat {
public:
    AtomicFloat(float v);
    void operatorfloat() const;
    AtomicFloat & operator=(float rhs);
    AtomicFloat & operator+=(float rhs);
    AtomicFloat & operator-=(float rhs);
    AtomicFloat & operator*=(float rhs);
    AtomicFloat & operator/=(float rhs);
    AtomicFloat & operator++();
    float operator++(int);
    AtomicFloat & operator--();
    float operator--(int);
    bool operator==(float rhs) const;
    bool operator!=(float rhs) const;
    bool operator<(float rhs) const;
    bool operator<=(float rhs) const;
    bool operator>(float rhs) const;
    bool operator>=(float rhs) const;
};
class AtomicInt {
public:
    AtomicInt() noexcept : v_(0) {}
    AtomicInt(int x) noexcept : v_(x) {}
    AtomicInt(const AtomicInt& other) noexcept : v_(other.v_.load()) {}

    AtomicInt& operator=(const AtomicInt& other) noexcept {
        if (this != &other) v_.store(other.v_.load());
        return *this;
    }
    AtomicInt& operator=(int x) noexcept {
        v_.store(x);
        return *this;
    }

    operator int() const noexcept { return v_.load(); }

    int operator++() noexcept { return v_.fetch_add(1) + 1; }
    int operator++(int) noexcept { return v_.fetch_add(1); }
    int operator--() noexcept { return v_.fetch_sub(1) - 1; }
    int operator--(int) noexcept { return v_.fetch_sub(1); }

    AtomicInt& operator+=(int x) noexcept { v_.fetch_add(x); return *this; }
    AtomicInt& operator-=(int x) noexcept { v_.fetch_sub(x); return *this; }
    AtomicInt& operator*=(int x) noexcept { cas_update([x](int cur){ return cur * x; }); return *this; }
    AtomicInt& operator/=(int x) noexcept { cas_update([x](int cur){ return cur / x; }); return *this; }
    AtomicInt& operator%=(int x) noexcept { cas_update([x](int cur){ return cur % x; }); return *this; }

    AtomicInt& operator&=(int x) noexcept { v_.fetch_and(x); return *this; }
    AtomicInt& operator|=(int x) noexcept { v_.fetch_or(x);  return *this; }
    AtomicInt& operator^=(int x) noexcept { v_.fetch_xor(x); return *this; }

    AtomicInt& operator<<=(int s) noexcept { cas_update([s](int cur){ return cur << s; }); return *this; }
    AtomicInt& operator>>=(int s) noexcept { cas_update([s](int cur){ return cur >> s; }); return *this; }

    int load(std::memory_order mo = std::memory_order_seq_cst) const noexcept { return v_.load(mo); }
    void store(int x, std::memory_order mo = std::memory_order_seq_cst) noexcept { v_.store(x, mo); }
    int exchange(int x, std::memory_order mo = std::memory_order_seq_cst) noexcept { return v_.exchange(x, mo); }
    bool compare_exchange(int& expected, int desired,
                          std::memory_order success = std::memory_order_seq_cst,
                          std::memory_order failure = std::memory_order_seq_cst) noexcept {
        return v_.compare_exchange_strong(expected, desired, success, failure);
    }

private:
    template <class F>
    void cas_update(F&& f) noexcept {
        int cur = v_.load();
        while (true) {
            int next = f(cur);
            if (v_.compare_exchange_weak(cur, next)) break;
        }
    }

    std::atomic<int> v_;
};
class BigInt {
public:
    BigInt(uint64_t value);
    BigInt operator+(const BigInt & other) const;
    void print() const;
};

void sleep(int ms);

class range {
public:
    class Iterator {
    public:
        Iterator(int current, int step) : current_(current), step_(step) {}

        int operator*() const { return current_; }
        Iterator& operator++() {
            current_ += step_;
            return *this;
        }
        bool operator!=(const Iterator& other) const {
            return step_ > 0 ? current_ < other.current_ : current_ > other.current_;
        }

    private:
        int current_;
        int step_;
    };

    range(int start, int end, int step = 1)
        : start_(start), end_(end), step_(step) {}

    Iterator begin() const { return Iterator(start_, step_); }
    Iterator end() const { return Iterator(end_, step_); }

private:
    int start_, end_, step_;
};

inline void print() {
    std::cout << std::endl;
}

// Variadic template: print eerste argument en roept recursief aan
template<typename T, typename... Args>
void print(const T& first, const Args&... rest) {
    std::cout << first << " ";
    print(rest...);
}