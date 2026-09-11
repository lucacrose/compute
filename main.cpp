#include <iostream>
#include <random>
#include <cstddef>
#include <vector>
#include <string>
#include "values.hpp"
#include "baskets.hpp"

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <items_count> <time_steps> <baskets_count>\n";
        return 1;
    }

    std::size_t items_count = std::stoull(argv[1]);
    std::size_t time_steps = std::stoull(argv[2]);
    std::size_t baskets_count = std::stoull(argv[3]);

    std::random_device rd;
    std::mt19937 gen(rd());

    std::vector<backtester::ItemResult> item_values = backtester::generate_values(items_count, time_steps, gen);

    std::vector<backtester::Basket> baskets = backtester::generate_baskets(baskets_count, gen);

    return 0;
}
