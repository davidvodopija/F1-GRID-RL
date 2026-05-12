#pragma once

#include "QLearningAgent.hpp"

namespace F1RL {
class GridWorld;

class AgentTrainer {
   public:
    AgentTrainer(GridWorld& world, QLearningAgent& agent, const SimulationConfig& config);

    void train(int episodes);

   private:
    GridWorld& world_;
    QLearningAgent& agent_;
    const SimulationConfig& config_;
};

}  // namespace F1RL
