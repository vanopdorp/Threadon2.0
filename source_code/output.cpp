
#include "lib/sttlib/sttlib.hpp"
using namespace std;
int fibonacci_recursive(int n) {if ((n <= 1)) {
    return n;
}
else {
    return (fibonacci_recursive((n - 1)) + fibonacci_recursive((n - 2)));
}
}
void thread_starter() {
}
int main() {thread_starter();
int n = 40;
print(fibonacci_recursive(n));
}
