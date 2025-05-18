#include "utils.hpp"

#include <cassert>

int main() {
    assert(utils::parse<double>("5.2") == 5.2);
}
