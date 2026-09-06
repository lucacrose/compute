// market_simulation.cpp
//
// Simulates item prices as a three-level factor model, the same structure
// used in real equity-market models (APT / Fama-French style):
//
//      item return = fundamental growth
//                  + beta_market  * market_return          <- systemic factor
//                  + beta_category * category_return       <- sector factor
//                  + idiosyncratic residual                <- item-specific
//
// Each level (market, category, item) gets its own:
//   - Ornstein-Uhlenbeck (OU) process for a slowly mean-reverting drift/mispricing
//   - GARCH(1,1) process for time-varying, clustered volatility (Bollerslev, 1986)
//   - Student-t distributed innovations for realistic fat tails (Bollerslev, 1987)
//
// Categories are assigned to items uniformly at random (equal probability).
//
// Complexity: generating the market path is O(T); generating the category
// paths is O(category_count * T); generating all items is O(items_count * T).
// Since category_count is a fixed small constant, total work is
// O(items_count * time_steps), and the market/category paths are computed
// once and reused (not recomputed per item).

#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

constexpr size_t items_count = 100;   // tune freely; should be >> category_count
constexpr size_t time_steps  = 2048;

namespace params {
    // ---------------- Market: the single systemic / macro factor ----------------
    constexpr double market_drift_theta = 0.01;     // OU speed: how fast the "business cycle" drift reverts
    constexpr double market_drift_mu    = 0.00035;  // long-run average daily return (~9%/yr)
    constexpr double market_drift_sigma = 0.00005;  // how much the drift itself wanders (regime shocks)

    constexpr double market_unconditional_vol = 0.010; // target long-run daily market volatility (~16%/yr)
    constexpr double market_garch_alpha = 0.08;        // ARCH term: reaction to the last shock
    constexpr double market_garch_beta  = 0.90;        // GARCH term: volatility persistence
    constexpr double market_garch_omega =
        market_unconditional_vol * market_unconditional_vol * (1.0 - market_garch_alpha - market_garch_beta);

    // ---------------- Category: sector-level macro dynamics ----------------
    constexpr size_t category_count = 6;

    constexpr double category_beta_min = 0.7;  // sector sensitivity to the market (CAPM-style beta)
    constexpr double category_beta_max = 1.3;

    constexpr double category_drift_theta = 0.02;    // sector rotation speed
    constexpr double category_drift_sigma = 0.0001;  // how much sector excess-return trends drift

    constexpr double category_unconditional_vol = 0.013; // sector-specific vol on top of its market beta
    constexpr double category_garch_alpha = 0.05;
    constexpr double category_garch_beta  = 0.90;
    constexpr double category_garch_omega =
        category_unconditional_vol * category_unconditional_vol * (1.0 - category_garch_alpha - category_garch_beta);

    // ---------------- Item: idiosyncratic, single-name dynamics ----------------
    constexpr double item_beta_market_mean   = 1.0;   // avg single-name beta to the market
    constexpr double item_beta_market_sd     = 0.30;
    constexpr double item_beta_category_mean = 1.0;   // avg loading on its own sector
    constexpr double item_beta_category_sd   = 0.25;

    constexpr double item_reversion_theta_min = 0.004; // speed of correction of mispricing (value/arbitrage)
    constexpr double item_reversion_theta_max = 0.020;

    constexpr double item_growth_mean = 0.0002;  // per-item long-run "fundamental" (earnings) growth
    constexpr double item_growth_sd   = 0.0003;  // dispersion of growth rates across items
    constexpr double item_fundamental_shock_sigma = 0.001; // surprise around that growth trend, per step

    constexpr double item_unconditional_vol = 0.013; // single-name idiosyncratic vol (still > index vol)
    constexpr double item_garch_alpha = 0.05;
    constexpr double item_garch_beta  = 0.85;
    constexpr double item_garch_omega =
        item_unconditional_vol * item_unconditional_vol * (1.0 - item_garch_alpha - item_garch_beta);

    constexpr double student_t_dof = 5.0; // fat tails, matching empirical excess kurtosis of real returns
}

constexpr std::array<const char*, params::category_count> category_names = {
    "Consumer Staples", "Cyclical", "Technology", "Energy", "Financials", "Healthcare"
};

// GARCH(1,1): sigma_t^2 = omega + alpha * epsilon_{t-1}^2 + beta * sigma_{t-1}^2
inline double garch_update_variance(double omega, double alpha, double beta,
                                     double prev_variance, double prev_shock) {
    return omega + alpha * prev_shock * prev_shock + beta * prev_variance;
}

// Standardized (unit-variance) Student-t innovation, as in GARCH-t models.
// Var[t_dist(dof)] = dof / (dof - 2), so we rescale to unit variance.
inline double standardized_t_shock(std::mt19937& gen) {
    static std::student_t_distribution<double> t_dist(params::student_t_dof);
    static const double correction = std::sqrt(params::student_t_dof / (params::student_t_dof - 2.0));
    return t_dist(gen) / correction;
}

// ---------------------------------------------------------------------------
// Market-wide log-return path: one OU-driven drift + one GARCH(1,1) vol process
// shared by every category and every item. O(time_steps).
// ---------------------------------------------------------------------------
std::array<double, time_steps> generate_market_path(std::mt19937& gen) {
    std::array<double, time_steps> returns{};

    std::normal_distribution<double> drift_noise(0.0, params::market_drift_sigma);

    double drift = params::market_drift_mu;
    double variance = params::market_garch_omega /
                       (1.0 - params::market_garch_alpha - params::market_garch_beta);
    double prev_shock = 0.0;

    for (size_t t = 0; t < time_steps; ++t) {
        // Ornstein-Uhlenbeck: dx = theta*(mu - x) + sigma*dW  (slow macro/business-cycle drift)
        drift += params::market_drift_theta * (params::market_drift_mu - drift) + drift_noise(gen);

        variance = garch_update_variance(params::market_garch_omega, params::market_garch_alpha,
                                          params::market_garch_beta, variance, prev_shock);
        double sigma = std::sqrt(variance);
        double shock = sigma * standardized_t_shock(gen);

        returns[t] = drift + shock;
        prev_shock = shock; // GARCH memory only sees the pure innovation, not the drift
    }

    return returns;
}

// ---------------------------------------------------------------------------
// One log-return path per category, each with its own OU drift ("sector
// rotation") and GARCH(1,1) vol, loaded onto the market via a CAPM-style beta.
// O(category_count * time_steps).
// ---------------------------------------------------------------------------
std::vector<std::array<double, time_steps>> generate_category_paths(
    std::mt19937& gen,
    const std::array<double, time_steps>& market_returns,
    std::vector<double>& betas_out)
{
    std::uniform_real_distribution<double> beta_dist(params::category_beta_min, params::category_beta_max);
    std::normal_distribution<double> drift_noise(0.0, params::category_drift_sigma);

    std::vector<std::array<double, time_steps>> category_returns(params::category_count);
    betas_out.resize(params::category_count);

    for (size_t c = 0; c < params::category_count; ++c) {
        double beta = beta_dist(gen);
        betas_out[c] = beta;

        double drift = 0.0;
        double variance = params::category_garch_omega /
                           (1.0 - params::category_garch_alpha - params::category_garch_beta);
        double prev_shock = 0.0;

        for (size_t t = 0; t < time_steps; ++t) {
            drift += params::category_drift_theta * (0.0 - drift) + drift_noise(gen);

            variance = garch_update_variance(params::category_garch_omega, params::category_garch_alpha,
                                              params::category_garch_beta, variance, prev_shock);
            double sigma = std::sqrt(variance);
            double shock = sigma * standardized_t_shock(gen);

            // CAPM-style: sector return = beta * market return + idiosyncratic sector drift/shock
            category_returns[c][t] = beta * market_returns[t] + drift + shock;
            prev_shock = shock;
        }
    }

    return category_returns;
}

struct ItemResult {
    std::array<int, time_steps> prices;
    size_t category;
};

// ---------------------------------------------------------------------------
// One item's price path: multi-factor return (market + category + idiosyncratic)
// plus mean reversion of price toward a slowly-drifting "fundamental value".
// Uses only the precomputed market/category paths, so this is O(time_steps)
// per item -> O(items_count * time_steps) overall.
// ---------------------------------------------------------------------------
ItemResult generate_item_series(
    std::mt19937& gen,
    size_t category,
    const std::array<double, time_steps>& market_returns,
    const std::array<double, time_steps>& category_returns)
{
    std::uniform_real_distribution<double> initial_price_pow(2.0, 7.0);
    std::normal_distribution<double> beta_market_dist(params::item_beta_market_mean, params::item_beta_market_sd);
    std::normal_distribution<double> beta_category_dist(params::item_beta_category_mean, params::item_beta_category_sd);
    std::uniform_real_distribution<double> reversion_theta_dist(params::item_reversion_theta_min,
                                                                  params::item_reversion_theta_max);
    std::normal_distribution<double> growth_dist(params::item_growth_mean, params::item_growth_sd);
    std::normal_distribution<double> fundamental_shock(0.0, params::item_fundamental_shock_sigma);

    double beta_mkt = std::max(0.0, beta_market_dist(gen));
    double beta_cat = std::max(0.0, beta_category_dist(gen));
    double theta = reversion_theta_dist(gen);
    double company_growth = growth_dist(gen); // this item's own long-run "earnings" growth trend

    double log_price = initial_price_pow(gen) * std::log(10.0); // log-uniform starting price in [100, 10,000,000]
    double log_fundamental = log_price;

    double variance = params::item_garch_omega /
                       (1.0 - params::item_garch_alpha - params::item_garch_beta);
    double prev_shock = 0.0;

    ItemResult result;
    result.category = category;

    for (size_t t = 0; t < time_steps; ++t) {
        double deviation = log_price - log_fundamental; // current mispricing vs. fair value

        double fundamental_growth = company_growth + fundamental_shock(gen);
        log_fundamental += fundamental_growth;

        variance = garch_update_variance(params::item_garch_omega, params::item_garch_alpha,
                                          params::item_garch_beta, variance, prev_shock);
        double sigma = std::sqrt(variance);
        double vol_shock = sigma * standardized_t_shock(gen);

        // Idiosyncratic residual = OU-style correction of mispricing + pure idiosyncratic noise
        double idiosyncratic = -theta * deviation + vol_shock;

        // Multi-factor (CAPM/APT-style) total return
        double item_return = fundamental_growth
                            + beta_mkt * market_returns[t]
                            + beta_cat * category_returns[t]
                            + idiosyncratic;

        log_price += item_return;
        prev_shock = vol_shock; // GARCH memory sees only the pure innovation

        result.prices[t] = std::max(1, static_cast<int>(std::llround(std::exp(log_price))));
    }

    return result;
}

int main() {
    std::random_device rd;
    std::mt19937 gen(rd());

    std::cout << std::setprecision(15);

    // 1. Full market dynamic, shared by everything below.            O(T)
    auto market_returns = generate_market_path(gen);

    // 2. Category ("sector") dynamics, shared by items in that category. O(category_count * T)
    std::vector<double> category_betas;
    auto category_returns = generate_category_paths(gen, market_returns, category_betas);

    // 3. Assign each item a category with equal probability, then simulate it. O(items_count * T)
    std::uniform_int_distribution<size_t> category_assignment(0, params::category_count - 1);

    std::vector<ItemResult> items;
    items.reserve(items_count);
    for (size_t i = 0; i < items_count; ++i) {
        size_t category = category_assignment(gen);
        items.push_back(generate_item_series(gen, category, market_returns, category_returns[category]));
    }

    std::ofstream output_file("out.csv");
    output_file << "item,category_id,category_name,step,value\n";
    for (size_t i = 0; i < items_count; ++i) {
        size_t c = items[i].category;
        for (size_t t = 0; t < time_steps; ++t) {
            output_file << i << "," << c << "," << category_names[c] << "," << t << "," << items[i].prices[t] << "\n";
        }
    }
    output_file.close();

    std::cout << "Wrote " << items_count << " items x " << time_steps
              << " steps across " << params::category_count << " categories to out.csv\n";

    return 0;
}