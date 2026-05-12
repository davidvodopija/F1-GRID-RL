#include "ConfigLoader.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "../common/Utils.hpp"

namespace F1RL {

SimulationConfig ConfigLoader::loadSimulationConfig(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open config file: " + filepath);
    }

    SimulationConfig config;
    std::string line;

    while (std::getline(file, line)) {
        line = Utils::trim(line);
        if (line.empty() || line[0] == '#') continue;

        size_t eqPos = line.find('=');
        if (eqPos != std::string::npos) {
            std::string key = Utils::trim(line.substr(0, eqPos));
            std::string value = Utils::trim(line.substr(eqPos + 1));

            size_t commentPos = value.find('#');
            if (commentPos != std::string::npos) {
                value = Utils::trim(value.substr(0, commentPos));
            }

            try {
                if (key == "PIT_STOP_DURATION")
                    config.PIT_STOP_DURATION = std::stoi(value);
                else if (key == "MAX_SPEED")
                    config.MAX_SPEED = std::stoi(value);
                else if (key == "PITLANE_LIMITER")
                    config.PITLANE_LIMITER = std::stoi(value);
                else if (key == "OBSERVATION_LOOKAHEAD")
                    config.OBSERVATION_LOOKAHEAD = std::stoi(value);
                else if (key == "PIT_LATERAL_THRESHOLD")
                    config.PIT_LATERAL_THRESHOLD = std::stoi(value);
                else if (key == "TIME_PENALTY")
                    config.TIME_PENALTY = std::stof(value);
                else if (key == "SPEEDING_PENALTY")
                    config.SPEEDING_PENALTY = std::stof(value);
                else if (key == "CRASH_PENALTY")
                    config.CRASH_PENALTY = std::stof(value);
                else if (key == "TIRE_WEAR_TARMAC")
                    config.TIRE_WEAR_TARMAC = std::stof(value);
                else if (key == "TIRE_HEALTH_WORN_THRESHOLD")
                    config.TIRE_HEALTH_WORN_THRESHOLD = std::stof(value);
                else if (key == "TIRE_HEALTH_CRITICAL_THRESHOLD")
                    config.TIRE_HEALTH_CRITICAL_THRESHOLD = std::stof(value);
                else if (key == "LAP_REWARD")
                    config.LAP_REWARD = std::stof(value);
                else if (key == "TRACK_LIMIT_PENALTY")
                    config.TRACK_LIMIT_PENALTY = std::stof(value);
                else if (key == "DRIVING_BACKWARD_PENALTY")
                    config.DRIVING_BACKWARD_PENALTY = std::stof(value);
                else if (key == "DRIVING_CORRECT_WAY_REWARD")
                    config.DRIVING_CORRECT_WAY_REWARD = std::stof(value);
                else if (key == "ALPHA")
                    config.ALPHA = std::stof(value);
                else if (key == "GAMMA")
                    config.GAMMA = std::stof(value);
                else if (key == "EPSILON")
                    config.EPSILON = std::stof(value);
                else if (key == "MIN_EPSILON")
                    config.MIN_EPSILON = std::stof(value);
                else if (key == "EPSILON_DECAY_RATE")
                    config.EPSILON_DECAY_RATE = std::stof(value);
                else if (key == "TRAINING_LOG_INTERVAL")
                    config.TRAINING_LOG_INTERVAL = std::stoi(value);
                else if (key == "TRAINING_EPISODES")
                    config.TRAINING_EPISODES = std::stoi(value);
            }
            catch (...) {
                std::cerr << "Warning: Failed to parse value for key: " << key << std::endl;
            }
        }
    }

    return config;
}
}  // namespace F1RL