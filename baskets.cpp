#include <vector>
#include <cstddef>
#include <random>
#include <cmath>
#include <iostream>
#include <cstdint>
#include <stdexcept>
#include "baskets.hpp"
#include "values.hpp"

namespace backtester {

std::vector<Basket> generate_baskets(std::size_t count, std::vector<ItemResult> items, std::mt19937& gen) {
    std::vector<Basket> baskets;
    baskets.reserve(count);

    std::uniform_int_distribution<std::uint8_t> random_size_dist(1, maxBasketItems);
    std::uniform_int_distribution<std::size_t> random_item_dist(0, items.size() - 1);
    std::normal_distribution<float> value_ratio_dist(0.05f, 0.005f);

    for (std::size_t i = 0; i < count; ++i) {
        Basket basket{};

        std::array<std::size_t, maxBasketItems> items_given{};
        std::array<std::size_t, maxBasketItems> items_received{};

        std::uint8_t items_given_count;
        std::uint8_t items_received_count;

        std::uint16_t attempts = 0;

        while (attempts < UINT16_MAX) {
            std::uint64_t given_value = 0;
            std::uint64_t received_value = 0;

            items_given_count = random_size_dist(gen);
            items_received_count = random_size_dist(gen);

            for (std::size_t j = 0; j < items_given_count; ++j) {
                std::size_t item_index = random_item_dist(gen);
                items_given[j] = item_index + 1;
                given_value += items[item_index].prices[0];
            }

            for (std::size_t j = 0; j < items_received_count; ++j) {
                std::size_t item_index = random_item_dist(gen);
                items_received[j] = item_index + 1;
                received_value += items[item_index].prices[0];
            }

            double given_received_ratio = static_cast<double>(given_value) / static_cast<double>(received_value);

            if (std::abs(std::log(given_received_ratio)) < std::abs(value_ratio_dist(gen))) {
                //std::cout << attempts << ", " << given_value << ", " << received_value << "\n";
                break;
            }

            items_given.fill(0);
            items_received.fill(0);

            ++attempts;
        }

        basket.items_given = items_given;
        basket.items_received = items_received;

        basket.items_given_count = items_given_count;
        basket.items_received_count = items_received_count;

        if (attempts == UINT16_MAX) {
            throw std::runtime_error("Unable to generate basket");
        }

        baskets.emplace_back(basket);
    }

    return baskets;
}

}
