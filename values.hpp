#pragma once

#include <vector>
#include <cstddef>
#include <random>
#include <cstdint>

namespace backtester {

struct ItemResult {
    std::vector<std::uint32_t> prices;
    std::size_t category;
};

[[nodiscard]] std::vector<ItemResult> generate_values(std::size_t items_count, std::size_t time_steps, std::mt19937& gen);

}
