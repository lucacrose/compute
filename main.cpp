#include <iostream>
#include <random>
#include <cstddef>
#include <vector>
#include "values.hpp"
#include "baskets.hpp"

constexpr std::size_t items_count = 100;
constexpr std::size_t time_steps  = 2048;
constexpr std::size_t baskets_count = 4;

int main() {
    std::random_device rd;
    std::mt19937 gen(rd());

    std::vector<backtester::ItemResult> item_values = backtester::generate_values(items_count, time_steps, gen);

    std::vector<backtester::Basket> baskets = backtester::generate_baskets(baskets_count, gen);

    std::cout << item_values.size() << std::endl;

    return 0;
}
