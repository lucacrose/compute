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
    std::uniform_real_distribution<double> velocity_dist(-0.05, 0.05);
    std::uniform_real_distribution<double> anchor_dist(0.5, 2.0);

    std::array<int, time_steps> series;

    double value = exp10(value_dist(gen));
    double velocity = velocity_dist(gen) * value;
    double anchor = value * anchor_dist(gen);

    for (size_t i = 0; i < time_steps; ++i) {
        value += velocity;
        
        double reversion = (anchor - value) / 500;

        value += reversion * reversion * (1 + -2 * std::signbit(reversion));

        series[i] = std::round(value);

        velocity = velocity_dist(gen) * value;
    }

    std::cout << "Anchor Value: " << anchor << std::endl;

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
