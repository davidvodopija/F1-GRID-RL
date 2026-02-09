#pragma once
#include <map>
#include <ostream>
#include <string>ž
#include "../common/Types.hpp"

namespace F1RL {

class ConfigLoader {
   public:
    static SimulationConfig loadSimulationConfig(const std::string& filepath);
};
}  // namespace F1RL