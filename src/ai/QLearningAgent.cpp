#include "QLearningAgent.hpp"

#include <algorithm>
#include <array>

namespace F1RL {

QLearningAgent::QLearningAgent(double alpha, double gamma, double epsilon)
    : alpha_(alpha)
    , gamma_(gamma)
    , epsilon_(epsilon)
    , rng_(std::random_device{}())
    , dist_01_(0.0, 1.0)
    , dist_action_(0, 4) {}

Action QLearningAgent::act(const Observation& obs, bool exploit_only) {
    size_t state_hash = obs.toHash();

    if (q_table_.find(state_hash) == q_table_.end()) {
        q_table_[state_hash] = {0.0, 0.0, 0.0, 0.0, 0.0};
    }

    if (!exploit_only && dist_01_(rng_) < epsilon_) {
        return static_cast<Action>(dist_action_(rng_));
    }

    const std::array<double, 5>& q_values = q_table_[state_hash];
    size_t best_action = 0;
    double max_qval = q_values[0];

    for (size_t i = 1; i <= 4; ++i) {
        if (q_values[i] > max_qval) {
            max_qval = q_values[i];
            best_action = i;
        }
    }
    return static_cast<Action>(best_action);
}

double QLearningAgent::getMaxQ(const Observation& state) {
    size_t hash = state.toHash();
    if (q_table_.find(hash) == q_table_.end()) return 0.0;

    const std::array<double, 5>& q_values = q_table_[hash];
    return *std::max_element(q_values.begin(), q_values.end());
}

void QLearningAgent::learn(const Observation& state, Action action, double reward, const Observation& next_state) {
    size_t hash = state.toHash();
    size_t action_index = static_cast<int>(action);

    if (q_table_.find(hash) == q_table_.end()) {
        q_table_[hash] = {0.0, 0.0, 0.0, 0.0, 0.0};
    }

    double current_q = q_table_[hash][action_index];
    double max_next_q = getMaxQ(next_state);

    double target = reward + gamma_ * max_next_q;
    q_table_[hash][action_index] = current_q + alpha_ * (target - current_q);
}

void QLearningAgent::decayEpsilon(double min_epsilon, double decay_rate) {
    epsilon_ = std::max(min_epsilon, epsilon_ * decay_rate);
}

}  // namespace F1RL