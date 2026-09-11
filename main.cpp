#include <iostream>
#include "values.cpp"
#include "baskets.hpp"

constexpr size_t items_count = 100;
constexpr size_t time_steps  = 2048;

int main() {
    std::vector<ItemResult<time_steps>> item_values = generate_values<time_steps>(items_count); // TODO: change this API, it's akward...

    backtester::generate_baskets(3);

    std::cout << item_values.size() << std::endl;

    return 0;
}
