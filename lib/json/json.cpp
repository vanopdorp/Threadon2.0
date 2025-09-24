#pragma once
#include <cassert>
#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstdint>
#include <cstring>   // strlen
#include <exception>
#include <iomanip>
#include <iostream>
#include <limits>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>
#include <iostream>
namespace json{

struct parse_error : std::runtime_error {
    size_t line;
    size_t col;
    std::string path;    // JSON Pointer-achtig pad (bijv. /items/3/name)
    std::string context; // klein venster rond de foutlocatie
    parse_error(const std::string& msg, size_t line_, size_t col_, std::string path_, std::string ctx = {})
        : std::runtime_error(msg), line(line_), col(col_), path(std::move(path_)), context(std::move(ctx)) {}
};

class json {
public:
    class object;
    using array = std::vector<json>;

    using value = std::variant<std::monostate, bool, int64_t, double, std::string, array, object>;

    class object {
    public:
        using kv = std::pair<std::string, json>;
        object() = default;

        bool contains(const std::string& key) const {
            return index_.find(key) != index_.end();
        }
        json* find(const std::string& key) {
            auto it = index_.find(key);
            return it == index_.end() ? nullptr : &items_[it->second].second;
        }
        const json* find(const std::string& key) const {
            auto it = index_.find(key);
            return it == index_.end() ? nullptr : &items_[it->second].second;
        }
        json& at(const std::string& key) {
            auto p = find(key);
            if (!p) throw std::out_of_range("key not found: " + key);
            return *p;
        }
        const json& at(const std::string& key) const {
            auto p = find(key);
            if (!p) throw std::out_of_range("key not found: " + key);
            return *p;
        }
        std::pair<json&, bool> set(std::string key, json value) {
            auto it = index_.find(key);
            if (it == index_.end()) {
                size_t idx = items_.size();
                index_.emplace(key, idx);
                items_.emplace_back(std::move(key), std::move(value));
                return { items_.back().second, true };
            } else {
                items_[it->second].second = std::move(value);
                return { items_[it->second].second, false };
            }
        }
        std::pair<json&, bool> emplace(std::string key, json value) {
            if (contains(key)) return { at(key), false };
            return set(std::move(key), std::move(value));
        }
        bool erase(const std::string& key) {
            auto it = index_.find(key);
            if (it == index_.end()) return false;
            size_t idx = it->second;
            items_.erase(items_.begin() + static_cast<std::ptrdiff_t>(idx));
            index_.erase(it);
            for (size_t i = idx; i < items_.size(); ++i) {
                index_[items_[i].first] = i;
            }
            return true;
        }

        // iteration in insertion order
        auto begin() { return items_.begin(); }
        auto end() { return items_.end(); }
        auto begin() const { return items_.begin(); }
        auto end() const { return items_.end(); }
        auto cbegin() const { return items_.cbegin(); }
        auto cend() const { return items_.cend(); }

        size_t size() const { return items_.size(); }
        bool empty() const { return items_.empty(); }

        kv& at(size_t i) { return items_.at(i); }
        const kv& at(size_t i) const { return items_.at(i); }

    private:
        std::vector<kv> items_;
        std::unordered_map<std::string, size_t> index_;
    };

private:
    value v_;

public:
    // ctors
    json() noexcept : v_(std::monostate{}) {}
    json(std::nullptr_t) noexcept : v_(std::monostate{}) {}
    json(bool b) : v_(b) {}
    json(int64_t i) : v_(i) {}
    json(int i) : v_(static_cast<int64_t>(i)) {}
    json(unsigned int i) : v_(static_cast<int64_t>(i)) {}
    json(long long i) : v_(static_cast<int64_t>(i)) {}
    json(double d) : v_(d) {}
    json(const char* s) : v_(std::string(s)) {}
    json(std::string s) : v_(std::move(s)) {}
    json(array a) : v_(std::move(a)) {}
    json(object o) : v_(std::move(o)) {}

    // factories
    static json make_array() { return json(array{}); }
    static json make_object() { return json(object{}); }

    // type checks
    bool is_null()   const { return std::holds_alternative<std::monostate>(v_); }
    bool is_bool()   const { return std::holds_alternative<bool>(v_); }
    bool is_int()    const { return std::holds_alternative<int64_t>(v_); }
    bool is_double() const { return std::holds_alternative<double>(v_); }
    bool is_number() const { return is_int() || is_double(); }
    bool is_string() const { return std::holds_alternative<std::string>(v_); }
    bool is_array()  const { return std::holds_alternative<array>(v_); }
    bool is_object() const { return std::holds_alternative<object>(v_); }

    // accessors
    bool               as_bool()   const { return std::get<bool>(v_); }
    int64_t            as_int()    const { return std::get<int64_t>(v_); }
    double             as_double() const { return std::get<double>(v_); }
    const std::string& as_string() const { return std::get<std::string>(v_); }
    const array&       as_array()  const { return std::get<array>(v_); }
    const object&      as_object() const { return std::get<object>(v_); }

    std::string& as_string() { return std::get<std::string>(v_); }
    array&       as_array()  { return std::get<array>(v_); }
    object&      as_object() { return std::get<object>(v_); }

    // size
    size_t size() const {
        if (is_array()) return as_array().size();
        if (is_object()) return as_object().size();
        if (is_string()) return as_string().size();
        return 0;
    }
    bool empty() const { return size() == 0; }

    // object lookups
    bool contains(const std::string& key) const {
        return is_object() && as_object().contains(key);
    }
    json* find(const std::string& key) {
        return is_object() ? as_object().find(key) : nullptr;
    }
    const json* find(const std::string& key) const {
        return is_object() ? as_object().find(key) : nullptr;
    }
    json& at(const std::string& key) { return as_object().at(key); }
    const json& at(const std::string& key) const { return as_object().at(key); }

    // array indexing
    json& at(size_t i) { return as_array().at(i); }
    const json& at(size_t i) const { return as_array().at(i); }

    // operator[]: alleen bestaande keys (geen autovivify)
    json& operator[](const std::string& key) {
        auto p = find(key);
        if (!p) throw std::out_of_range("key not found: " + key);
        return *p;
    }
    const json& operator[](const std::string& key) const {
        auto p = find(key);
        if (!p) throw std::out_of_range("key not found: " + key);
        return *p;
    }
    json& operator[](size_t i) { return as_array().at(i); }
    const json& operator[](size_t i) const { return as_array().at(i); }

    // modifiers
    std::pair<json&, bool> set(const std::string& key, json value) { return as_object().set(key, std::move(value)); }
    std::pair<json&, bool> emplace(const std::string& key, json value) { return as_object().emplace(key, std::move(value)); }
    bool erase(const std::string& key) { return as_object().erase(key); }

    // equality
    friend bool operator==(const json& a, const json& b) { return a.v_ == b.v_; }
    friend bool operator!=(const json& a, const json& b) { return !(a == b); }

    // get<T>
    template <class T>
    T get() const {
        if constexpr (std::is_same_v<T, bool>) return as_bool();
        else if constexpr (std::is_same_v<T, int64_t>) {
            if (is_int()) return as_int();
            if (is_double()) {
                double d = as_double();
                if (!std::isfinite(d)) throw std::domain_error("non-finite cannot convert to int64");
                auto v = static_cast<int64_t>(d);
                if (static_cast<double>(v) != d) throw std::domain_error("lossy integer conversion");
                return v;
            }
            throw std::bad_variant_access();
        } else if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
            auto v = get<int64_t>();
            if (v < std::numeric_limits<T>::min() || v > std::numeric_limits<T>::max())
                throw std::out_of_range("integer out of range");
            return static_cast<T>(v);
        } else if constexpr (std::is_same_v<T, double>) {
            if (is_double()) return as_double();
            if (is_int()) return static_cast<double>(as_int());
            throw std::bad_variant_access();
        } else if constexpr (std::is_same_v<T, std::string>) return as_string();
        else if constexpr (std::is_same_v<T, array>) return as_array();
        else if constexpr (std::is_same_v<T, object>) return as_object();
        else {
            static_assert(!sizeof(T*), "Unsupported get<T> type");
        }
    }

    // parsing
    static json parse(const std::string& s) {
        struct state {
            const char* p;
            const char* b;
            size_t line = 1, col = 1;
            std::vector<std::string> path; // JSON Pointer segments

            char peek() const { return *p; }
            bool eof() const { return *p == '\0'; }
            void advance() {
                if (*p == '\n') { ++line; col = 1; } else { ++col; }
                ++p;
            }
            void skip_ws() {
                while (std::isspace(static_cast<unsigned char>(*p))) advance();
            }
            std::string pointer() const {
                std::string out;
                for (const auto& seg : path) {
                    out.push_back('/');
                    for (char c : seg) {
                        if (c == '~') out += "~0";
                        else if (c == '/') out += "~1";
                        else out.push_back(c);
                    }
                }
                return out;
            }
            [[noreturn]] void fail(const std::string& msg) const {
                const char* start = p;
                while (start > b && (p - start) < 20) --start;
                const char* end = p;
                const char* bend = b + std::strlen(b);
                if (end < bend) {
                    const char* maxend = p + 20;
                    end = (maxend < bend ? maxend : bend);
                }
                std::string ctx(start, end - start);
                throw parse_error(msg, line, col, pointer(), ctx);
            }

            uint32_t parse_hex4() {
                auto hex = [&](char c)->int {
                    if (c >= '0' && c <= '9') return c - '0';
                    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
                    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
                    return -1;
                };
                uint32_t code = 0;
                for (int i = 0; i < 4; ++i) {
                    int h = hex(*p);
                    if (h < 0) fail("invalid hex digit in unicode escape");
                    code = (code << 4) | static_cast<uint32_t>(h);
                    advance();
                }
                return code;
            }

            static void append_utf8(uint32_t cp, std::string& out) {
                if (cp <= 0x7F) out.push_back(static_cast<char>(cp));
                else if (cp <= 0x7FF) {
                    out.push_back(static_cast<char>(0xC0 | ((cp >> 6) & 0x1F)));
                    out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
                } else if (cp <= 0xFFFF) {
                    out.push_back(static_cast<char>(0xE0 | ((cp >> 12) & 0x0F)));
                    out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
                    out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
                } else if (cp <= 0x10FFFF) {
                    out.push_back(static_cast<char>(0xF0 | ((cp >> 18) & 0x07)));
                    out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
                    out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
                    out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
                } else {
                    throw std::runtime_error("invalid Unicode code point");
                }
            }

            std::string parse_string() {
                if (*p != '"') fail("expected opening quote");
                advance();
                std::string out;
                while (!eof()) {
                    char c = peek();
                    advance();
                    if (c == '"') break;
                    if (c == '\\') {
                        char e = peek();
                        advance();
                        switch (e) {
                            case '"': out.push_back('"'); break;
                            case '\\': out.push_back('\\'); break;
                            case '/': out.push_back('/'); break;
                            case 'b': out.push_back('\b'); break;
                            case 'f': out.push_back('\f'); break;
                            case 'n': out.push_back('\n'); break;
                            case 'r': out.push_back('\r'); break;
                            case 't': out.push_back('\t'); break;
                            case 'u': {
                                uint32_t u1 = parse_hex4();
                                if (u1 >= 0xD800 && u1 <= 0xDBFF) {
                                    if (*p != '\\') fail("expected low surrogate after high surrogate");
                                    advance();
                                    if (*p != 'u') fail("expected 'u' in low surrogate");
                                    advance();
                                    uint32_t u2 = parse_hex4();
                                    if (u2 < 0xDC00 || u2 > 0xDFFF) fail("invalid low surrogate");
                                    uint32_t cp = 0x10000 + (((u1 - 0xD800) << 10) | (u2 - 0xDC00));
                                    append_utf8(cp, out);
                                } else if (u1 >= 0xDC00 && u1 <= 0xDFFF) {
                                    fail("unexpected low surrogate");
                                } else {
                                    append_utf8(u1, out);
                                }
                                break;
                            }
                            default:
                                fail("invalid escape sequence");
                        }
                    } else {
                        if (static_cast<unsigned char>(c) < 0x20) fail("unescaped control character in string");
                        out.push_back(c);
                    }
                }
                return out;
            }

            json parse_number() {
                const char* start = p;
                if (*p == '-') advance();
                if (*p == '0') {
                    advance();
                } else {
                    if (!std::isdigit(static_cast<unsigned char>(*p))) fail("invalid number");
                    while (std::isdigit(static_cast<unsigned char>(*p))) advance();
                }
                bool is_float = false;
                if (*p == '.') {
                    is_float = true;
                    advance();
                    if (!std::isdigit(static_cast<unsigned char>(*p))) fail("invalid fraction");
                    while (std::isdigit(static_cast<unsigned char>(*p))) advance();
                }
                if (*p == 'e' || *p == 'E') {
                    is_float = true;
                    advance();
                    if (*p == '+' || *p == '-') advance();
                    if (!std::isdigit(static_cast<unsigned char>(*p))) fail("invalid exponent");
                    while (std::isdigit(static_cast<unsigned char>(*p))) advance();
                }
                std::string_view sv(start, static_cast<size_t>(p - start));
                std::string tmp(sv);
                char* endptr = nullptr;
                errno = 0;
                double d = std::strtod(tmp.c_str(), &endptr);
                if (!std::isfinite(d)) fail("non-finite number");

                if (!is_float) {
                    bool negative = (sv.front() == '-');
                    size_t i = negative ? 1u : 0u;
                    int64_t val = 0;
                    while (i < sv.size()) {
                        char c = sv[i++];
                        if (c < '0' || c > '9') { val = 0; negative = false; break; }
                        int digit = c - '0';
                        if (negative) {
                            if (val < (std::numeric_limits<int64_t>::min() + digit) / 10) {
                                return json(d);
                            }
                            val = val * 10 - digit;
                        } else {
                            if (val > (std::numeric_limits<int64_t>::max() - digit) / 10) {
                                return json(d);
                            }
                            val = val * 10 + digit;
                        }
                    }
                    return json(val);
                }
                return json(d);
            }

            json parse_array() {
                advance(); // '['
                skip_ws();
                array arr;
                if (peek() == ']') { advance(); return json(std::move(arr)); }
                size_t idx = 0;
                while (true) {
                    path.push_back(std::to_string(idx));
                    arr.push_back(parse_value());
                    path.pop_back();
                    ++idx;
                    skip_ws();
                    if (peek() == ']') { advance(); break; }
                    if (peek() != ',') fail("expected comma in array");
                    advance();
                    skip_ws();
                }
                return json(std::move(arr));
            }

            json parse_object() {
                advance(); // '{'
                skip_ws();
                object obj;
                if (peek() == '}') { advance(); return json(std::move(obj)); }
                while (true) {
                    if (peek() != '"') fail("object key must be a string");
                    std::string key = parse_string();
                    skip_ws();
                    if (peek() != ':') fail("expected colon");
                    advance();
                    skip_ws();
                    path.push_back(key);
                    json val = parse_value();
                    path.pop_back();
                    obj.set(std::move(key), std::move(val));
                    skip_ws();
                    if (peek() == '}') { advance(); break; }
                    if (peek() != ',') fail("expected comma in object");
                    advance();
                    skip_ws();
                }
                return json(std::move(obj));
            }

            json parse_value() {
                skip_ws();
                char c = peek();
                switch (c) {
                    case 'n': {
                        const char* exp = "null";
                        for (int i = 0; exp[i]; ++i) { if (peek() != exp[i]) fail("invalid literal null"); advance(); }
                        return json(nullptr);
                    }
                    case 't': {
                        const char* exp = "true";
                        for (int i = 0; exp[i]; ++i) { if (peek() != exp[i]) fail("invalid literal true"); advance(); }
                        return json(true);
                    }
                    case 'f': {
                        const char* exp = "false";
                        for (int i = 0; exp[i]; ++i) { if (peek() != exp[i]) fail("invalid literal false"); advance(); }
                        return json(false);
                    }
                    case '"': return json(parse_string());
                    case '[': return parse_array();
                    case '{': return parse_object();
                    default:
                        if (c == '-' || c == '.' || std::isdigit(static_cast<unsigned char>(c)))
                            return parse_number();
                        fail("unexpected character");
                }
            }
        };

        state st{ s.c_str(), s.c_str() };
        json j = st.parse_value();
        st.skip_ws();
        if (!st.eof()) st.fail("trailing characters after JSON value");
        return j;
    }

    static json parse(std::istream& is) {
        std::ostringstream oss;
        oss << is.rdbuf();
        return parse(oss.str());
    }

    // dumping
struct dump_options {
    int indent;
    bool serialize_non_finite_as_null;
    dump_options(int indent_ = -1, bool non_finite_as_null = true)
        : indent(indent_), serialize_non_finite_as_null(non_finite_as_null) {}
};


    std::string dump(dump_options opt = dump_options{}) const {
        std::ostringstream os;
        dump(os, opt);
        return os.str();
    }

    void dump(std::ostream& os, dump_options opt = dump_options{}) const {
        struct dumper {
            std::ostream& os;
            dump_options opt;
            void write_indent(int level) {
                if (opt.indent >= 0) for (int i = 0; i < level * opt.indent; ++i) os.put(' ');
            }
            void write_string(const std::string& s) {
                os.put('"');
                for (unsigned char c : s) {
                    switch (c) {
                        case '"':  os << "\\\""; break;
                        case '\\': os << "\\\\"; break;
                        case '\b': os << "\\b";  break;
                        case '\f': os << "\\f";  break;
                        case '\n': os << "\\n";  break;
                        case '\r': os << "\\r";  break;
                        case '\t': os << "\\t";  break;
                        default:
                            if (c < 0x20) {
                                os << "\\u"
                                   << std::hex << std::uppercase << std::setw(4) << std::setfill('0')
                                   << static_cast<int>(c)
                                   << std::dec << std::nouppercase;
                            } else {
                                os.put(static_cast<char>(c));
                            }
                    }
                }
                os.put('"');
            }
            void write_double(double d) {
                if (!std::isfinite(d)) {
                    if (opt.serialize_non_finite_as_null) { os << "null"; }
                    else throw std::domain_error("non-finite double cannot be serialized in JSON");
                    return;
                }
                std::ostringstream tmp;
                tmp.setf(std::ios::fmtflags(0), std::ios::floatfield);
                tmp << std::setprecision(17) << d;
                os << tmp.str();
            }
            void write(const json& j, int level) {
                if (j.is_null()) { os << "null"; return; }
                if (j.is_bool()) { os << (j.as_bool() ? "true" : "false"); return; }
                if (j.is_int())  { os << j.as_int(); return; }
                if (j.is_double()) { write_double(j.as_double()); return; }
                if (j.is_string()) { write_string(j.as_string()); return; }
                if (j.is_array()) {
                    const auto& a = j.as_array();
                    os << "[";
                    if (!a.empty()) {
                        if (opt.indent >= 0) os << "\n";
                        for (size_t i = 0; i < a.size(); ++i) {
                            if (opt.indent >= 0) write_indent(level + 1);
                            write(a[i], level + 1);
                            if (i + 1 < a.size()) os << ",";
                            if (opt.indent >= 0) os << "\n";
                        }
                        if (opt.indent >= 0) write_indent(level);
                    }
                    os << "]";
                    return;
                }
                if (j.is_object()) {
                    const auto& o = j.as_object();
                    os << "{";
                    if (!o.empty()) {
                        if (opt.indent >= 0) os << "\n";
                        for (size_t i = 0; i < o.size(); ++i) {
                            const auto& kv = o.at(i);
                            if (opt.indent >= 0) write_indent(level + 1);
                            write_string(kv.first);
                            os << (opt.indent >= 0 ? ": " : ":");
                            write(kv.second, level + 1);
                            if (i + 1 < o.size()) os << ",";
                            if (opt.indent >= 0) os << "\n";
                        }
                        if (opt.indent >= 0) write_indent(level);
                    }
                    os << "}";
                    return;
                }
            }
        } d{ os, opt };
        d.write(*this, 0);
    }
};

} // namespace json


int main() {
    using json::json;

    // Object bouwen met insertion-order behoud
    json o = json::make_object();
    o.as_object().set("name", "Ada");
    o.as_object().set("active", true);
    o.as_object().set("score", 99.5);
    o.as_object().set("big", int64_t{9223372036854775807}); // int64 exact

    // Array
    json a = json::make_array();
    a.as_array().push_back(1);
    a.as_array().push_back(2);
    a.as_array().push_back("three");

    o.as_object().set("list", a);

    // Toegang (geen autovivify): operator[] vereist bestaande sleutels
    std::cout << o["name"].as_string() << "\n";
    std::cout << o["list"][2].as_string() << "\n";

    // Pretty print
    json::json::dump_options opts;
    opts.indent = 2;
    std::cout << o.dump(opts) << "\n";

    // Parse
    auto p = json::parse(R"({"x":[1,2,3],"y":"\u20AC"})");
    std::cout << p["y"].as_string() << "\n"; // €
}
