#include <iostream>
#include <vector>

int main() {
    double value;
    while (std::cin.read(reinterpret_cast<char*>(&value), sizeof(value))) {
        std::cout << "→ " << value << "\n";
    }
    return 0;
}
