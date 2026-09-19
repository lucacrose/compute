#include <vector>
#include <cstddef>
#include <random>
#include <cmath>
#include <iostream>
#include <cstdint>
#include <stdexcept>
#include <algorithm>
#include <array>
#include "baskets.hpp"
#include "values.hpp"

namespace backtester {

std::vector<Basket> generate_baskets(std::size_t count, const std::vector<ItemResult>& items, std::uint16_t max_attempts, std::mt19937& gen) {
    if (items.empty()) {
        throw std::runtime_error("Cannot generate baskets: no items provided.");
    }

    if (items[0].prices.empty()) {
        throw std::runtime_error("Cannot generate baskets: item has no price data.");
    }

    if (max_attempts == 0) {
        throw std::runtime_error("Cannot generate baskets: max_attempts is zero.");
    }

    std::vector<Basket> baskets;
    baskets.reserve(count);

    std::uniform_int_distribution<int> random_size_dist(1, maxBasketItems);
    std::uniform_int_distribution<std::size_t> random_item_dist(0, items.size() - 1);
    std::uniform_int_distribution<std::size_t> random_time_step_dist(0, items[0].prices.size() - 1);
    std::normal_distribution<float> log_value_ratio_dist(0.05f, 0.005f);
    std::bernoulli_distribution includes_currency_dist(0.5);
    std::lognormal_distribution<double> currency_ratio_dist(std::log(0.05), 0.35);

    std::size_t baskets_created = 0;

    while (baskets_created < count) {
        Basket basket{};

        std::array<std::size_t, maxBasketItems> items_given{};
        std::array<std::size_t, maxBasketItems> items_received{};

        std::uint8_t items_given_count;
        std::uint8_t items_received_count;

        std::size_t basket_time_step_index = random_time_step_dist(gen);
        bool basket_includes_currency = includes_currency_dist(gen);
        double currency_ratio = currency_ratio_dist(gen);
        float log_target_value_ratio = std::abs(log_value_ratio_dist(gen));

        std::uint16_t attempts = 0;
        
        std::int64_t net_currency_received = 0;

        while (attempts < max_attempts) {
            std::uint64_t given_value = 0;
            std::uint64_t received_value = 0;

            items_given_count = static_cast<std::uint8_t>(random_size_dist(gen));
            items_received_count = static_cast<std::uint8_t>(random_size_dist(gen));

            for (std::size_t i = 0; i < items_given_count; ++i) {
                std::size_t item_index = random_item_dist(gen);
                
                items_given[i] = item_index;
                given_value += items[item_index].prices[basket_time_step_index];
            }

            const auto given_end = items_given.begin() + items_given_count;

            for (std::size_t i = 0; i < items_received_count; ++i) {
                std::size_t item_index;

                do {
                    item_index = random_item_dist(gen);
                } while (std::find(items_given.begin(), given_end, item_index) != given_end);

                items_received[i] = item_index;
                received_value += items[item_index].prices[basket_time_step_index];
            }

            if (basket_includes_currency) {
                bool we_pay = given_value < received_value;
                std::uint64_t currency = static_cast<std::uint64_t>(std::llround(std::min(given_value, received_value) * currency_ratio));

                if (we_pay) {
                    given_value += currency;
                    net_currency_received = -static_cast<std::int64_t>(currency);
                } else {
                    received_value += currency;
                    net_currency_received = static_cast<std::int64_t>(currency);
                }
            }

            double given_received_ratio = static_cast<double>(given_value) / received_value;

            if (std::abs(std::log(given_received_ratio)) < log_target_value_ratio) {
                break;
            }

            ++attempts;
        }

        if (attempts == max_attempts) {
            continue;
        }

        basket.items_given = items_given;
        basket.items_received = items_received;

        basket.items_given_count = items_given_count;
        basket.items_received_count = items_received_count;

        basket.net_currency_received = net_currency_received;

        std::cout << "Basket " << baskets_created << ":\n->Given: ";

        for (std::size_t i = 0; i < items_given_count; ++i) {
            std::cout << items_given[i] << " (" << items[items_given[i]].prices[basket_time_step_index] << "), ";
        }

        std::cout << "\n->Received: ";

        for (std::size_t i = 0; i < items_received_count; ++i) {
            std::cout << items_received[i] << " (" << items[items_received[i]].prices[basket_time_step_index] << "), ";
        }

        std::cout << "\n->Currency: " << basket.net_currency_received;

        std::cout << std::endl;

        baskets.emplace_back(basket);

        ++baskets_created;
    }

    return baskets;
}

}
