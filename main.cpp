#include <iostream>
#include <random>
#include <cstddef>
#include <vector>
#include <string>
#include "values.hpp"
#include "baskets.hpp"

int main(int argc, char* argv[]) {
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " <items_count> <time_steps> <baskets_count> <max_basket_generation_attempts>\n";
        return 1;
    }

    std::size_t items_count = std::stoull(argv[1]);
    std::size_t time_steps = std::stoull(argv[2]);
    std::size_t baskets_count = std::stoull(argv[3]);
    std::size_t max_basket_generation_attempts = std::stoull(argv[4]);

    std::random_device rd;
    std::mt19937 gen(rd());

    std::vector<backtester::ItemResult> item_values = backtester::generate_values(items_count, time_steps, gen);

    std::vector<backtester::Basket> baskets = backtester::generate_baskets(baskets_count, item_values, max_basket_generation_attempts, gen);

    return 0;
}
