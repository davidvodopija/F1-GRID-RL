#pragma once
#include <SFML/Graphics.hpp>

#include "../ai/QLearningAgent.hpp"
#include "../common/Types.hpp"
#include "GridWorld.hpp"
#include <optional>

namespace F1RL {
class SimulationRunner {
   public:
    SimulationRunner(GridWorld& world, QLearningAgent& agent, const AppConfig& appConfig);
    void runSimulation();

   private:
    std::optional<Action> getManualAction(const sf::Event::KeyPressed& keyPressed);
    const void drawWorld();
    static sf::Color getCellColor(const Cell& cell);
    void printStepInfo(const StepResult& result);
    static void printObservation(const Observation& obs);

    GridWorld& world_;
    QLearningAgent& agent_;
    AppConfig appConfig_;
    sf::RenderWindow window_;
};

}  // namespace F1RL