#include <iostream>
#include <vector>
#include <random>
#include <iomanip>
#include <print>
#include <fstream>

constexpr size_t items_count = 1;
constexpr size_t time_steps = 2048;

std::array<int, time_steps> item_value(std::mt19937& gen) {
    std::uniform_real_distribution<double> value_dist(2.0, 7.0);
    std::normal_distribution<double> velocity_dist(0, 0.0005);
    std::normal_distribution<double> value_shock(0, 0.01);
    std::normal_distribution<double> anchor_drift(0, 0.005);

    std::array<int, time_steps> series;

    double log_value = std::log(exp10(value_dist(gen)));
    double log_anchor = log_value;
    double velocity = 0;

    for (size_t i = 0; i < time_steps; ++i) {
        log_anchor += anchor_drift(gen);

        velocity = velocity * 0.8 + velocity_dist(gen) + (log_anchor - log_value) / 100;

        log_value += velocity + value_shock(gen);

        series[i] = std::round(std::exp(log_value));
    }

    return series;
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

    for (size_t i = 0; i < items_count; ++i) {
        item_values.push_back(item_value(gen));
    }

    //std::print("{}\n", item_values);

    std::ofstream output_file("out.csv");

    output_file << "step,value\n";

    for (size_t i = 0; i < time_steps; ++i) {
        output_file << i << "," << item_values[0][i] << "\n";
    }

    output_file.close();

    return 0;
}
