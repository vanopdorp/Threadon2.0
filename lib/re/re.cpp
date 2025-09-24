#include <iostream>
#include <regex>
#include <string>
#include <vector>
#include <optional>

namespace re {

    struct Match {
        std::smatch sm;
        bool success{false};

        std::string group(size_t i = 0) const {
            if (!success || i >= sm.size()) return "";
            return sm[i].str();
        }

        size_t groups() const { return sm.size() ? sm.size() - 1 : 0; }
    };

    class Regex {
    public:
        explicit Regex(const std::string& pattern, std::regex_constants::syntax_option_type flags = std::regex::ECMAScript)
            : pat(pattern), reg(pattern, flags) {}

        std::optional<Match> match(const std::string& text) const {
            Match m;
            m.success = std::regex_match(text, m.sm, reg);
            if (!m.success) return std::nullopt;
            return m;
        }

        std::optional<Match> search(const std::string& text) const {
            Match m;
            m.success = std::regex_search(text, m.sm, reg);
            if (!m.success) return std::nullopt;
            return m;
        }

        std::vector<std::string> findall(const std::string& text) const {
            std::vector<std::string> results;
            auto begin = std::sregex_iterator(text.begin(), text.end(), reg);
            auto end = std::sregex_iterator();
            for (auto it = begin; it != end; ++it) {
                results.push_back((*it).str());
            }
            return results;
        }

        std::vector<Match> finditer(const std::string& text) const {
            std::vector<Match> results;
            auto begin = std::sregex_iterator(text.begin(), text.end(), reg);
            auto end = std::sregex_iterator();
            for (auto it = begin; it != end; ++it) {
                Match m;
                m.sm = *it;
                m.success = true;
                results.push_back(m);
            }
            return results;
        }

        std::string sub(const std::string& repl, const std::string& text) const {
            return std::regex_replace(text, reg, repl);
        }

        std::vector<std::string> split(const std::string& text) const {
            std::vector<std::string> parts;
            std::sregex_token_iterator it(text.begin(), text.end(), reg, -1);
            std::sregex_token_iterator end;
            for (; it != end; ++it) {
                parts.push_back(*it);
            }
            return parts;
        }

    private:
        std::string pat;
        std::regex reg;
    };

    // Module-level functions
    inline Regex compile(const std::string& pattern, std::regex_constants::syntax_option_type flags = std::regex::ECMAScript) {
        return Regex(pattern, flags);
    }

    inline std::optional<Match> match(const std::string& pattern, const std::string& text) {
        return compile(pattern).match(text);
    }

    inline std::optional<Match> search(const std::string& pattern, const std::string& text) {
        return compile(pattern).search(text);
    }

    inline std::vector<std::string> findall(const std::string& pattern, const std::string& text) {
        return compile(pattern).findall(text);
    }

    inline std::vector<Match> finditer(const std::string& pattern, const std::string& text) {
        return compile(pattern).finditer(text);
    }

    inline std::string sub(const std::string& pattern, const std::string& repl, const std::string& text) {
        return compile(pattern).sub(repl, text);
    }

    inline std::vector<std::string> split(const std::string& pattern, const std::string& text) {
        return compile(pattern).split(text);
    }

    inline std::string escape(const std::string& s) {
static const std::regex metachars{R"([{}.^$|()[\]

*+?\\]

)"};

        return std::regex_replace(s, metachars, R"(\$&)");
    }

} // namespace re

// ----------------------
// DEMO
// ----------------------
int main() {
    using namespace re;

    auto m = match(R"(\d+)", "123abc");
    if (m) {
        std::cout << "Match: " << m->group() << "\n";
    }

    auto s = search(R"([a-z]+)", "123abc456");
    if (s) {
        std::cout << "Search: " << s->group() << "\n";
    }

    auto all = findall(R"(\d+)", "abc 123 def 456");
    for (auto& a : all) std::cout << "Findall: " << a << "\n";

    auto replaced = sub(R"(\d+)", "NUM", "abc 123 def 456");
    std::cout << "Sub: " << replaced << "\n";

    auto parts = split(R"(\s+)", "split this   text");
    for (auto& p : parts) std::cout << "[" << p << "]";
    std::cout << "\n";

    std::cout << "Escape: " << escape("c++") << "\n";

    return 0;
}
