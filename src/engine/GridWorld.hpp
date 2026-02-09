#pragma once
#include <vector>

#include "../common/Types.hpp"

namespace F1RL {

class GridWorld {
   public:
    explicit GridWorld(const MapData& map_data, const SimulationConfig& config);

    void reset();
    [[nodiscard]] StepResult step(Action action);

    // Getters
    [[nodiscard]] State getState() const { return current_state_; }
    [[nodiscard]] int getWidth() const { return width_; }
    [[nodiscard]] int getHeight() const { return height_; }
    [[nodiscard]] const Cell& getCell(int x, int y) const;

   private:
    int width_;
    int height_;
    std::vector<Cell> grid_;
    State current_state_;
    SimulationConfig config_;
};

}  // namespace F1RL