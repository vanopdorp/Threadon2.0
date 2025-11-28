<img src="./images/Threadon.png">

# ⚡️ Threadon Programming Language 🚀

Threadon is an programming language designed to combine the **simplicity of Python 🐍** with the **raw performance of C++ 💨**.  
It targets modern multi-threaded applications and leverages C++20 features under the hood.  
The language is translated into optimized C++ code, giving developers both **productivity** and **execution speed**.  

---

### ✨ Features
- 📝 Python-like syntax for readability.  
- ⚡ Direct transpilation to C++20, ensuring high performance.  
- 🔀 Built-in concurrency model with simplified thread management.  
- 🛠️ You can use **all** C++ datatypes
- 🔗 Seamless interoperability with standard C++ libraries.  
- 🎭 Class support, function generation, and default arguments.  
- 🔄 Native coroutine support via `Task` return type.  

---

### 📊 Performance Benchmark

#### fib.th
```
def int fibonacci_recursive(int n):
    if n <= 1:
        return n
    else:
        return fibonacci_recursive(n-1) + fibonacci_recursive(n-2)
def main():
    int n = 40  
    print(fibonacci_recursive(n))

```

Performance measured using the recursive Fibonacci implementation:

| 🔗 Language / Framework | ⚡ Execution Time |
|--------------------------|------------------|
| 🐍 Python (NumPy)        | 🕒 44.56 sec      |
| 💨 C++                   | ⏱️ 0.85 sec       |
| 🚀 Threadon              | ⏱️ 0.86 sec       |

🎉 *Threadon reaches near-native C++ speeds while keeping the code simple and clean!*

---

### 🏗️ Compilation Process
1. 🧩 Source code is parsed into an AST.  
2. 🔧 AST is transformed into equivalent C++20 code.  
3. ⚙️ Generated C++ is compiled with `g++ -std=c++20 -Ofast`.  
4. 🔀 Threaded functions are automatically launched in detached `std::thread` objects.  

---

### ❓ Why Threadon?
- 🐍 Python is beginner-friendly but slow for compute-heavy tasks.  
- 💨 C++ is extremely fast but complex to write and maintain.  
- 🚀 The snake in the logo is based on the python logo yellow is readable en blue is performance threadon is a combination of it
---

