#pragma once
#include <string>

#include "../common/Types.hpp"

namespace F1RL {

class WorldLoader {
   public:
    static MapData loadFromJson(const std::string& filepath);
};

}  // namespace F1RL