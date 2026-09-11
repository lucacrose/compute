#include <iostream>
#include "values.cpp"

constexpr size_t items_count = 100;
constexpr size_t time_steps  = 2048;

int main() {
    std::vector<ItemResult<time_steps>> items = generate_values<time_steps>(items_count); // TODO: change this API, it's akward...

    std::cout << items.size() << std::endl;

    return 0;
}
