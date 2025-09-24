#pragma once
#include <chrono>
#include <thread>
#include <ctime>
#include <string>
#include <sstream>
#include <iomanip>

namespace time_utils {

    // Pauzeer het programma voor x seconden
    void sleep(int seconds) {
        std::this_thread::sleep_for(std::chrono::seconds(seconds));
    }

    // Pauzeer in milliseconden
    void sleep_ms(int milliseconds) {
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }

    // Pauzeer in microseconden
    void sleep_us(int microseconds) {
        std::this_thread::sleep_for(std::chrono::microseconds(microseconds));
    }

    // Geef huidige tijd als string
    std::string now() {
        std::time_t t = std::time(nullptr);
        return std::ctime(&t);
    }

    // Geef UNIX timestamp
    long long timestamp() {
        return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    }

    // Formatteer huidige tijd
    std::string format(const std::string& pattern = "%Y-%m-%d %H:%M:%S") {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm tm = *std::localtime(&t);

        std::ostringstream oss;
        oss << std::put_time(&tm, pattern.c_str());
        return oss.str();
    }

    // Geef lokale tijd als std::tm
    std::tm get_local_time() {
        std::time_t t = std::time(nullptr);
        return *std::localtime(&t);
    }

    // Geef UTC tijd als std::tm
    std::tm get_utc_time() {
        std::time_t t = std::time(nullptr);
        return *std::gmtime(&t);
    }

    // Parse een tijdstring naar std::tm
    std::tm parse(const std::string& time_str, const std::string& format) {
        std::tm tm = {};
        std::istringstream ss(time_str);
        ss >> std::get_time(&tm, format.c_str());
        return tm;
    }

    // Stopwatch klasse
    class Stopwatch {
        std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
    public:
        void start() {
            start_time = std::chrono::high_resolution_clock::now();
        }

        double elapsed() {
            auto end_time = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> diff = end_time - start_time;
            return diff.count(); // seconden
        }
    };

    // Vergelijk twee tijdstempels
    bool is_before(std::time_t a, std::time_t b) {
        return std::difftime(a, b) < 0;
    }

    bool is_after(std::time_t a, std::time_t b) {
        return std::difftime(a, b) > 0;
    }

    bool is_equal(std::time_t a, std::time_t b) {
        return std::difftime(a, b) == 0;
    }

}

