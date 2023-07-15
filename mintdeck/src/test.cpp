#include "test.hpp"

namespace test
{

void printstring(std::string string) {
    std::cout << string << std::endl << std::endl;
}

void stringvectorprint(std::vector<std::string> stringlist) {
        for (const auto& string : stringlist) {
        std::cout << string << std::endl;
    }
}

}