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

std::vector<Basket> generate_baskets(std::size_t count, std::vector<ItemResult> items, std::uint16_t max_attempts, std::mt19937& gen) {
    std::size_t time_steps = items[0].prices.size();

    if (items.size() == 0 || time_steps == 0 || max_attempts == 0) {
        throw std::runtime_error("Cannot generate baskets with items_count = 0 or time_steps = 0 or max_attempts = 0.");
    }

    std::vector<Basket> baskets;
    baskets.reserve(count);

    std::uniform_int_distribution<std::uint8_t> random_size_dist(1, maxBasketItems);
    std::uniform_int_distribution<std::size_t> random_item_dist(0, items.size() - 1);
    std::uniform_int_distribution<std::size_t> random_time_step_dist(0, time_steps - 1);
    std::normal_distribution<float> value_ratio_dist(0.05f, 0.005f);

    std::vector<std::size_t> item_last_attempt_used(items.size());

    for (std::size_t i = 0; i < count; ++i) {
        Basket basket{};

        std::array<std::size_t, maxBasketItems> items_given{};
        std::array<std::size_t, maxBasketItems> items_received{};

        std::uint8_t items_given_count;
        std::uint8_t items_received_count;

        std::size_t basket_time_step_index = random_time_step_dist(gen);
        float log_target_value_ratio = std::abs(value_ratio_dist(gen));

        std::uint16_t attempts = 1;

        while (attempts < max_attempts + 1) {
            std::uint64_t given_value = 0;
            std::uint64_t received_value = 0;

            std::size_t attempt_index = attempts * 2;

            items_given_count = random_size_dist(gen);
            items_received_count = random_size_dist(gen);

            for (std::size_t j = 0; j < items_given_count; ++j) {
                std::size_t item_index;

                while (true) {
                    item_index = random_item_dist(gen);

                    if (item_last_attempt_used[item_index] / 2 != attempt_index) {
                        break;
                    }
                }

                items_given[j] = item_index;
                item_last_attempt_used[item_index] = attempts * 2;
                given_value += items[item_index].prices[basket_time_step_index];
            }

            for (std::size_t j = 0; j < items_received_count; ++j) {
                std::size_t item_index;

                while (1) {
                    item_index = random_item_dist(gen);
    
                    if (item_last_attempt_used[item_index] / 2 != attempt_index) {
                        break;
                    }
                }

                items_received[j] = item_index;
                item_last_attempt_used[item_index] = attempts * 2 + 1;
                received_value += items[item_index].prices[basket_time_step_index];
            }

            double given_received_ratio = static_cast<double>(given_value) / static_cast<double>(received_value);

            if (std::abs(std::log(given_received_ratio)) < log_target_value_ratio) {
                std::cout << attempts << ", " << given_value << ", " << received_value << "\n";
                break;
            }

            ++attempts;
        }

        if (attempts == max_attempts + 1) {
            --i;
            continue;
        }

        basket.items_given = items_given;
        basket.items_received = items_received;

        basket.items_given_count = items_given_count;
        basket.items_received_count = items_received_count;

        std::cout << "Basket " << i << ":\n->Given: ";

        for (std::size_t j = 0; j < items_given_count; ++j) {
            std::cout << items_given[j] << " (" << items[items_given[j]].prices[basket_time_step_index] << "), ";
        }

        std::cout << "\n->Received: ";

        for (std::size_t j = 0; j < items_received_count; ++j) {
            std::cout << items_received[j] << " (" << items[items_received[j]].prices[basket_time_step_index] << "), " << std::endl;
        }

        baskets.emplace_back(basket);

        std::fill(item_last_attempt_used.begin(), item_last_attempt_used.end(), 0);
    }

    return baskets;
}

}
