#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>
#include "values.hpp"

namespace backtester {

constexpr std::size_t maxBasketItems = 4;

struct Basket {
    std::array<std::size_t, maxBasketItems> items_given;
    std::array<std::size_t, maxBasketItems> items_received;

    std::int64_t net_currency_received = 0;

    std::uint8_t items_given_count = 0;
    std::uint8_t items_received_count = 0;
};

[[nodiscard]]
std::vector<Basket> generate_baskets(std::size_t count, std::vector<ItemResult> items, std::uint16_t max_attempts, std::mt19937& gen); 

}
