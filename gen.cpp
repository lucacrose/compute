#include <iostream>
#include <vector>
#include <random>
#include <iomanip>
#include <print>

constexpr size_t items_count = 10;
constexpr size_t time_steps = 32;

std::array<int, time_steps> item_value(std::mt19937& gen) {
    std::uniform_real_distribution<double> pdist(2.0, 7.0);
    std::uniform_real_distribution<double> vdist(-0.05, 0.05);

    std::array<int, time_steps> value;

    double p = exp10(pdist(gen));
    double v = vdist(gen) * p;

    for (size_t i = 0; i < time_steps; i++) {
        value[i] = std::round(p);

        p += v;
        v = vdist(gen) * p;
    }

    return value;
}

int main() {
    // store items Y
    // init values Y
    // create values over time Y
    // generate baskets with (normal?) varience and currency

    std::random_device rd;
    std::mt19937 gen(rd());

    std::cout << std::setprecision(15);

    std::vector<std::array<int, time_steps>> item_values;

    for (size_t i = 0; i < items_count; i++) {
        item_values.push_back(item_value(gen));
    }

    std::print("{}\n", item_values);

    return 0;
}
