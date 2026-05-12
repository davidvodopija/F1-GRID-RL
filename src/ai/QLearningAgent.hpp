#pragma once

#include <array>
#include <random>
#include <unordered_map>

#include "../common/Types.hpp"

namespace F1RL {

class QLearningAgent {
   public:
    QLearningAgent(double alpha = 0.1, double gamma = 0.99, double epsilon = 1.0);

    Action act(const Observation& obs, bool exploit_only = false);

    void learn(const Observation& state, Action action, double reward, const Observation& next_state);

    void decayEpsilon(double min_epsilon, double decay_rate);

   private:
    double alpha_;
    double gamma_;
    double epsilon_;

    std::unordered_map<size_t, std::array<double, 5>> q_table_;

    std::mt19937 rng_;
    std::uniform_real_distribution<double> dist_01_;
    std::uniform_int_distribution<int> dist_action_;

    double getMaxQ(const Observation& state);
};
}  // namespace F1RL