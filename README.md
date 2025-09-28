<img src="./images/Threadon.png">

# ⚡️ Threadon Programming Language 🚀

Threadon is an experimental programming language designed to combine the **simplicity of Python 🐍** with the **raw performance of C++ 💨**.  
It targets modern multi-threaded applications and leverages C++20 features under the hood.  
The language is translated into optimized C++ code, giving developers both **productivity** and **execution speed**.  

---

### ✨ Features
- 📝 Python-like syntax for rapid prototyping and readability.  
- ⚡ Direct transpilation to C++20, ensuring high performance.  
- 🔀 Built-in concurrency model with simplified thread management.  
- 🛠️ Strong type mapping (`int`, `float`, `string`, `bool`, `BigInt`, `Task`, `Atomic` types).  
- 🔗 Seamless interoperability with standard C++ libraries.  
- 🎭 Class support, function generation, and default arguments.  
- 🔄 Native coroutine support via `Task` return type.  

## This simple example computes the 40th Fibonacci number using recursion.

---

### 📊 Performance Benchmark

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
- 🚀 Threadon is the **best of both worlds**: easy syntax, native performance, and built-in concurrency.  

---

### 🛣️ Roadmap
- ⏳ Extend support for async/await style coroutines.  
- 🛡️ Add memory safety analysis.  
- 📈 Optimize collection types further.  
- 📦 Add a package management system for third-party modules.  
- 🐞 Provide enhanced debugging and error messages.  

---
