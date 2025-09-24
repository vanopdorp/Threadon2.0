#include <iostream>
#include <string>
#include <mutex>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <sstream>
#include <iomanip>
#include <cstdio>
#include <vector>
#include <ostream>
class AtomicString {
    std::string value;
    mutable std::mutex mtx;

public:
    AtomicString(const std::string& str = "") : value(str) {}

    // Impliciete conversie naar std::string
    operator std::string() const {
        std::lock_guard<std::mutex> lock(mtx);
        return value;
    }

    // Toewijzing
    AtomicString& operator=(const std::string& rhs) {
        std::lock_guard<std::mutex> lock(mtx);
        value = rhs;
        return *this;
    }

    // += operator
    AtomicString& operator+=(const std::string& rhs) {
        std::lock_guard<std::mutex> lock(mtx);
        value += rhs;
        return *this;
    }

    // Vergelijkingen
    bool operator==(const std::string& rhs) const {
        std::lock_guard<std::mutex> lock(mtx);
        return value == rhs;
    }

    bool operator!=(const std::string& rhs) const {
        std::lock_guard<std::mutex> lock(mtx);
        return value != rhs;
    }

    bool operator<(const std::string& rhs) const {
        std::lock_guard<std::mutex> lock(mtx);
        return value < rhs;
    }

    bool operator>(const std::string& rhs) const {
        std::lock_guard<std::mutex> lock(mtx);
        return value > rhs;
    }

    // Friend operators voor string links
    friend bool operator==(const std::string& lhs, const AtomicString& rhs) { return rhs == lhs; }
    friend bool operator!=(const std::string& lhs, const AtomicString& rhs) { return rhs != lhs; }
    friend bool operator<(const std::string& lhs, const AtomicString& rhs)  { return lhs < std::string(rhs); }
    friend bool operator>(const std::string& lhs, const AtomicString& rhs)  { return lhs > std::string(rhs); }

};

class AtomicFloat {
    std::atomic<float> value;

public:
    AtomicFloat(float v = 0.0f) : value(v) {}

    // Impliciete conversie naar float
    operator float() const {
        return value.load(std::memory_order_relaxed);
    }

    // Toewijzing
    AtomicFloat& operator=(float rhs) {
        value.store(rhs, std::memory_order_relaxed);
        return *this;
    }

    // Arithmetic operators
    AtomicFloat& operator+=(float rhs) {
        float current = value.load(std::memory_order_relaxed);
        value.store(current + rhs, std::memory_order_relaxed);
        return *this;
    }

    AtomicFloat& operator-=(float rhs) {
        float current = value.load(std::memory_order_relaxed);
        value.store(current - rhs, std::memory_order_relaxed);
        return *this;
    }

    AtomicFloat& operator*=(float rhs) {
        float current = value.load(std::memory_order_relaxed);
        value.store(current * rhs, std::memory_order_relaxed);
        return *this;
    }

    AtomicFloat& operator/=(float rhs) {
        float current = value.load(std::memory_order_relaxed);
        value.store(current / rhs, std::memory_order_relaxed);
        return *this;
    }

    // Increment/decrement
    AtomicFloat& operator++() {
        *this += 1.0f;
        return *this;
    }

    float operator++(int) {
        float old = value.load(std::memory_order_relaxed);
        *this += 1.0f;
        return old;
    }

    AtomicFloat& operator--() {
        *this -= 1.0f;
        return *this;
    }

    float operator--(int) {
        float old = value.load(std::memory_order_relaxed);
        *this -= 1.0f;
        return old;
    }

    // Comparison operators
    bool operator==(float rhs) const { return value.load(std::memory_order_relaxed) == rhs; }
    bool operator!=(float rhs) const { return value.load(std::memory_order_relaxed) != rhs; }
    bool operator<(float rhs)  const { return value.load(std::memory_order_relaxed) < rhs; }
    bool operator<=(float rhs) const { return value.load(std::memory_order_relaxed) <= rhs; }
    bool operator>(float rhs)  const { return value.load(std::memory_order_relaxed) > rhs; }
    bool operator>=(float rhs) const { return value.load(std::memory_order_relaxed) >= rhs; }

    // Friend comparison with float on left
    friend bool operator==(float lhs, const AtomicFloat& rhs) { return rhs == lhs; }
    friend bool operator!=(float lhs, const AtomicFloat& rhs) { return rhs != lhs; }
    friend bool operator<(float lhs, const AtomicFloat& rhs)  { return lhs < float(rhs); }
    friend bool operator<=(float lhs, const AtomicFloat& rhs) { return lhs <= float(rhs); }
    friend bool operator>(float lhs, const AtomicFloat& rhs)  { return lhs > float(rhs); }
    friend bool operator>=(float lhs, const AtomicFloat& rhs) { return lhs >= float(rhs); }
};

void sleep(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}



class BigInt {
    static const uint32_t BASE = 1000000000; // 10^9
    std::vector<uint32_t> digits;

public:
    BigInt(uint64_t value = 0) {
        while (value > 0) {
            digits.push_back(value % BASE);
            value /= BASE;
        }
    }

    // Optellen
    BigInt operator+(const BigInt& other) const {
        BigInt result;
        result.digits.clear();

        const size_t n = std::max(digits.size(), other.digits.size());
        uint64_t carry = 0;

        for (size_t i = 0; i < n || carry; ++i) {
            uint64_t sum = carry;
            if (i < digits.size()) sum += digits[i];
            if (i < other.digits.size()) sum += other.digits[i];
            result.digits.push_back(sum % BASE);
            carry = sum / BASE;
        }

        return result;
    }

    // Printen
    void print() const {
        if (digits.empty()) {
            std::cout << "0";
            return;
        }
        std::cout << digits.back();
        for (int i = digits.size() - 2; i >= 0; --i)
            std::cout << std::setw(9) << std::setfill('0') << digits[i];
    }
friend std::ostream& operator<<(std::ostream& os, const BigInt& b) {
    if (b.digits.empty()) return os << "0";
    os << b.digits.back();
    for (int i = b.digits.size() - 2; i >= 0; --i)
        os << std::setw(9) << std::setfill('0') << b.digits[i];
    return os;
}

};
// ---------------- Template print ----------------
// Basis: stopt de print
inline void print() {
    std::cout << std::endl;
}

// Variadic template: print eerste argument en roept recursief aan
template<typename T, typename... Args>
void print(const T& first, const Args&... rest) {
    std::cout << first << " ";
    print(rest...);
}