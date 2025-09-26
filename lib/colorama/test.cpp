#include "colorama.hpp"
int main() {
    color("red", "bold");
    std::cout << "Dit is rode, vette tekst!" << std::endl;
    reset();
    color("red","nothing");
    std::cout << "Dit is gewone rode tekst!" << std::endl;
    reset();
    color("cyan", "underline");
    std::cout << "Onderstreepte cyaan tekst." << std::endl;
    reset();
    return 0;
}
