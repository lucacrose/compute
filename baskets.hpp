#pragma once

#include <vector>
#include <cstddef>
#include <cstdint>

namespace backtester {

struct Basket {
    std::vector<std::size_t> items_given;
    std::vector<std::size_t> items_received;
    std::uint64_t net_currency_received;
};

[[nodiscard]] std::vector<Basket> generate_baskets(std::size_t count); 

}
