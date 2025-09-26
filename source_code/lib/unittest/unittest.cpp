#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <stdexcept>
#include <chrono>
#include <iomanip>
#include <cmath>
#include <sstream>

namespace unittest {

struct TestCaseBase {
    using TestFunc  = std::function<void()>;
    using TestEntry = std::pair<std::string, TestFunc>;

    std::vector<TestEntry> test_methods;

    virtual void setUp() {}
    virtual void tearDown() {}
    virtual ~TestCaseBase() = default;

    void register_method(const std::string& name, TestFunc func) {
        test_methods.emplace_back(name, std::move(func));
    }

    template <typename... Entries>
    void register_methods(Entries&&... entries) {
        (test_methods.emplace_back(std::forward<Entries>(entries)), ...);
    }

    void run() {
        int passed = 0, failed = 0;
        for (const auto& [name, func] : test_methods) {
            auto start = std::chrono::high_resolution_clock::now();
            bool ok = true;
            std::string msg;

            try {
                setUp();
                func();
                tearDown();
            } catch (const std::exception& e) {
                ok = false;
                msg = e.what();
            } catch (...) {
                ok = false;
                msg = "unknown error";
            }

            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elapsed = end - start;

            if (ok) {
                std::cout << "✅ \033[32mPASS\033[0m: " << name
                          << " (" << std::fixed << std::setprecision(2)
                          << elapsed.count() << " ms)\n";
                ++passed;
            } else {
                std::cout << "❌ \033[31mFAIL\033[0m: " << name << " — " << msg << "\n";
                ++failed;
            }
        }

        std::cout << "\n\033[1mSummary:\033[0m "
                  << passed + failed << " tests run — "
                  << "\033[32m" << passed << " passed\033[0m, "
                  << "\033[31m" << failed << " failed\033[0m\n\n";
    }
};

// ---------------- Assertions met meer context ----------------
#define ASSERT_TRUE(expr) \
    if (!(expr)) { \
        std::ostringstream oss; \
        oss << "Assertion failed: " #expr; \
        throw std::runtime_error(oss.str()); \
    }

#define ASSERT_FALSE(expr) \
    if ((expr)) { \
        std::ostringstream oss; \
        oss << "Assertion failed: not " #expr; \
        throw std::runtime_error(oss.str()); \
    }

#define ASSERT_EQ(a, b) \
    { auto _va = (a); auto _vb = (b); \
      if (!(_va == _vb)) { \
        std::ostringstream oss; \
        oss << "Assertion failed: " #a " == " #b \
            << " (actual: " << _va << ", expected: " << _vb << ")"; \
        throw std::runtime_error(oss.str()); \
      } }

#define ASSERT_NE(a, b) \
    { auto _va = (a); auto _vb = (b); \
      if (!(_va != _vb)) { \
        std::ostringstream oss; \
        oss << "Assertion failed: " #a " != " #b \
            << " (both: " << _va << ")"; \
        throw std::runtime_error(oss.str()); \
      } }

#define ASSERT_FLOAT_EQ(a, b, epsilon) \
    { auto _va = (a); auto _vb = (b); \
      if (std::fabs(_va - _vb) > (epsilon)) { \
        std::ostringstream oss; \
        oss << "Assertion failed: " #a " ~= " #b \
            << " (actual: " << _va << ", expected: " << _vb \
            << ", epsilon: " << epsilon << ")"; \
        throw std::runtime_error(oss.str()); \
      } }

#define ASSERT_THROW(expr, exception_type) \
    { bool thrown = false; \
      try { expr; } \
      catch (const exception_type&) { thrown = true; } \
      catch (...) {} \
      if (!thrown) { \
        throw std::runtime_error("Expected exception: " #exception_type); \
      } }

// ---------------- Mocking & Stubbing ----------------
template<typename Ret, typename... Args>
class MockFunction {
public:
    using FuncType = std::function<Ret(Args...)>;
    FuncType func;
    int call_count = 0;

    void set_stub(FuncType f) { func = f; }
    Ret operator()(Args... args) {
        ++call_count;
        if (func) return func(std::forward<Args>(args)...);
        if constexpr (!std::is_void_v<Ret>) return Ret{};
    }
};

// ---------------- Parameterized tests ----------------
template<typename... Args>
struct ParamTest {
    using TestBody = std::function<void(Args...)>;
    std::vector<std::tuple<Args...>> params;
    TestBody body;

    void add_case(Args... args) {
        params.emplace_back(std::make_tuple(args...));
    }

    void run_all() {
        for (auto& tup : params) {
            std::apply(body, tup);
        }
    }
};

// Macro’s voor automatische registratie
#define TEST_METHOD(method) \
    unittest::TestCaseBase::TestEntry{ \
        std::string(#method), \
        unittest::TestCaseBase::TestFunc([this]{ this->method(); }) \
    }

#define AUTO_REGISTER_TESTS(...) \
    this->register_methods(__VA_ARGS__)

} // namespace unittest

#include <stdexcept>
#include <cmath>
#include <string>

class MathTests : public unittest::TestCaseBase {
public:
    int base = 0;

    MathTests() {
        AUTO_REGISTER_TESTS(
            TEST_METHOD(test_addition),
            TEST_METHOD(test_mocking),
            TEST_METHOD(test_param)
        );
    }

    void setUp() override { base = 42; }
    void tearDown() override { base = 0; }

    void test_addition() {
        ASSERT_EQ(base + 8, 50);
        ASSERT_FLOAT_EQ(0.1 + 0.2, 0.3, 0.0001);
    }

    void test_mocking() {
        unittest::MockFunction<int,int,int> mockAdd;
        mockAdd.set_stub([](int a, int b){ return a + b; });
        ASSERT_EQ(mockAdd(2,3), 5);
        ASSERT_EQ(mockAdd.call_count, 1);
    }

    void test_param() {
        unittest::ParamTest<int,int> pt;
        pt.body = [](int a, int b) {
            ASSERT_TRUE(a + b > 0);
        };
        pt.add_case(1,2);
        pt.add_case(5,10);
        pt.run_all();
    }
};

int main() {
    MathTests tests;
    tests.run();
}
