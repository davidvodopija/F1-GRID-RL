#include <SFML/Graphics.hpp>
#include <iostream>
#include <optional>
#include <string>

#include "ai/AgentTrainer.hpp"
#include "ai/QLearningAgent.hpp"
#include "common/Types.hpp"
#include "engine/GridWorld.hpp"
#include "engine/SimulationRunner.hpp"
#include "io/ConfigLoader.hpp"
#include "io/WorldLoader.hpp"

using namespace F1RL;

static AppConfig createConfig(int argc, char** argv) {
    AppConfig config;

    /*
        Usage examples:

        ./app ai assets/track1.json assets/config1.config
        ./app manual assets/track2.json assets/config2.config
    */

    if (argc != 4) {
        throw std::invalid_argument("Invalid number of parameters");
    }

    std::string modeArg = argv[1];
    if (modeArg == "manual") {
        config.mode = RunMode::Manual;
    }
    else if (modeArg == "ai") {
        config.mode = RunMode::AI;
    }
    else {
        throw std::invalid_argument("Unsupported run mode");
    }

    config.trackPath = argv[2];
    config.simulationConfigPath = argv[3];

    return config;
}

int main(int argc, char** argv) {
    AppConfig appConfig = createConfig(argc, argv);

    MapData map = WorldLoader::loadFromJson(appConfig.trackPath);
    SimulationConfig simulationConfig = ConfigLoader::loadSimulationConfig(appConfig.simulationConfigPath);

    GridWorld world(map, simulationConfig);
    QLearningAgent agent(simulationConfig.ALPHA, simulationConfig.GAMMA, simulationConfig.EPSILON);

    if (appConfig.mode == RunMode::AI) {
        AgentTrainer trainer(world, agent, simulationConfig);
        trainer.train(simulationConfig.TRAINING_EPISODES);
    }

    SimulationRunner simulationRunner(world, agent, appConfig);
    simulationRunner.runSimulation();

    return 0;
}