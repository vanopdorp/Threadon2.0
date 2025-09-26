#include <iostream>
#include <unordered_map>
#include <string>

// ANSI kleurcodes
std::unordered_map<std::string, std::string> kleurCodes = {
    {"black", "30"}, {"red", "31"}, {"green", "32"}, {"yellow", "33"},
    {"blue", "34"}, {"magenta", "35"}, {"cyan", "36"}, {"white", "37"},
    {"bright_black", "90"}, {"bright_red", "91"}, {"bright_green", "92"},
    {"bright_yellow", "93"}, {"bright_blue", "94"}, {"bright_magenta", "95"},
    {"bright_cyan", "96"}, {"bright_white", "97"}
};

// ANSI stijlcodes
std::unordered_map<std::string, std::string> stijlCodes = {
    {"nothing", "0"}, {"bold", "1"}, {"dim", "2"}, {"italic", "3"},
    {"underline", "4"}, {"blink", "5"}, {"reverse", "7"}
};
std::unordered_map<std::string, std::string> achtergrondCodes = {
    {"black", "40"}, {"red", "41"}, {"green", "42"}, {"yellow", "43"},
    {"blue", "44"}, {"magenta", "45"}, {"cyan", "46"}, {"white", "47"},
    {"bright_black", "100"}, {"bright_red", "101"}, {"bright_green", "102"},
    {"bright_yellow", "103"}, {"bright_blue", "104"}, {"bright_magenta", "105"},
    {"bright_cyan", "106"}, {"bright_white", "107"}
};

// Activeer kleur + stijl
void color(const std::string& kleur, const std::string& stijl) {
    std::string kleurCode = kleurCodes.count(kleur) ? kleurCodes[kleur] : "37"; // default: white
    std::string stijlCode = stijlCodes.count(stijl) ? stijlCodes[stijl] : "0";  // default: nothing
    std::cout << "\033[" << stijlCode << ";" << kleurCode << "m";
}

// Reset terminalkleur
void reset() {
    std::cout << "\033[0m";
}
