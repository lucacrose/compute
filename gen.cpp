#include <iostream>
#include <vector>
#include <random>
#include <iomanip>
#include <print>

std::vector<int> item_value(std::mt19937& gen, size_t count) {
    std::uniform_real_distribution<double> pdist(2.0, 7.0);
    std::uniform_real_distribution<double> vdist(-0.05, 0.05);

    std::vector<int> value;

    double p = exp10(pdist(gen));
    double v = vdist(gen) * p;

    for (size_t i = 0; i < count; i++) {
        value.push_back(p);

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

    std::vector<std::vector<int>> item_values;

    for (size_t i = 0; i < 10; i++) {
        item_values.push_back(item_value(gen, 32));
    }

    std::print("{}\n", item_values);

    return 0;
}
