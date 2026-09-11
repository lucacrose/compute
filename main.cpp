#include <iostream>
#include <random>
#include <cstddef>
#include <vector>
#include "values.cpp"
#include "baskets.hpp"

constexpr std::size_t items_count = 100;
constexpr std::size_t time_steps  = 2048;

int main() {
    std::random_device rd;
    std::mt19937 gen(rd());

    std::vector<ItemResult<time_steps>> item_values = generate_values<time_steps>(items_count); // TODO: change this API, it's akward...

    std::vector<backtester::Basket> baskets = backtester::generate_baskets(3, gen);

    std::cout << item_values.size() << std::endl;

    return 0;
}
