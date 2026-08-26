#include <iostream>
#include <vector>
#include <random>
#include <iomanip>

int main() {
    // store items Y
    // init values Y
    // create values over time
    // generate baskets with (normal?) varience and currency

    std::vector<std::vector<int>> item_values;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(2.0, 7.0);

    std::cout << std::setprecision(15);

    for (int i = 0; i < 10; i++) {
        std::cout << exp10(dist(gen)) << std::endl;
    }

    return 0;
}
