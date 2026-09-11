#pragma once

template <size_t TimeSteps>
struct ItemResult {
    std::array<int, TimeSteps> prices;
    size_t category;
};

template <size_t TimeSteps>
std::vector<ItemResult<TimeSteps>> generate_values(size_t items_count);
