#include "AgentTrainer.hpp"

#include <iostream>

#include "../engine/GridWorld.hpp"

namespace F1RL {

AgentTrainer::AgentTrainer(GridWorld& world, QLearningAgent& agent, const SimulationConfig& config)
    : world_(world), agent_(agent), config_(config) {}

void AgentTrainer::train(int episodes) {
    std::cout << "Starting headless training (" << episodes << " episodes) -------------------" << std::endl;

    for (int episode = 0; episode < episodes; ++episode) {
        world_.reset();

        bool done = false;

        while (!done) {
            Observation obs = world_.getObservation();

            Action action = agent_.act(obs, false);

            StepResult result = world_.step(action);

            Observation nextObs = world_.getObservation();

            agent_.learn(obs, action, result.reward, nextObs);

            done = result.done;
        }

        agent_.decayEpsilon(config_.MIN_EPSILON, config_.EPSILON_DECAY_RATE);

        if (config_.TRAINING_LOG_INTERVAL > 0 && (episode + 1) % config_.TRAINING_LOG_INTERVAL == 0) {
            std::cout << "Episode " << (episode + 1) << "/" << episodes << " complete." << std::endl;
        }
    }

    std::cout << "--- Training Complete! ---" << std::endl;
}

}  // namespace F1RL
