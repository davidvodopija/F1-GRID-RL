#include "GridWorld.hpp"

#include <algorithm>
#include <cmath>

namespace F1RL {

GridWorld::GridWorld(const MapData& map, const SimulationConfig& config) {
    width_ = map.width;
    height_ = map.height;
    grid_ = map.grid;
    config_ = config;
    reset();
}

const Cell& GridWorld::getCell(int x, int y) const {
    if (x < 0 || x >= width_ || y < 0 || y >= height_) {
        static const Cell out_of_track_cell{Surface::OutOfTrack, Feature::None};
        return out_of_track_cell;
    }
    return grid_[y * width_ + x];
}

void GridWorld::reset() {
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            const Cell& cell = getCell(x, y);
            if (cell.feature == Feature::StartFinishLine) {
                current_state_ = State{
                    Position{x, y},
                    cell.expected_heading,
                    0,      // speed
                    1.0f,   // tire_health
                    false,  // crashed
                    0,      // lap_count
                    0,      // pit_timer
                    false   // lap_invalidated
                };
                return;
            }
        }
    }
}

static Position getForwardVector(Heading h) {
    switch (h) {
        case Heading::North:
            return {0, -1};
        case Heading::South:
            return {0, 1};
        case Heading::East:
            return {1, 0};
        case Heading::West:
            return {-1, 0};
        default:
            return {0, 0};
    }
}

static bool isPitTerrain(Feature f) { return f == Feature::PitLane || f == Feature::PitBox || f == Feature::PitEntry; }

bool isOppositeHeading(Heading a, Heading b) {
    if (a == Heading::North && b == Heading::South) return true;
    if (a == Heading::South && b == Heading::North) return true;
    if (a == Heading::East && b == Heading::West) return true;
    if (a == Heading::West && b == Heading::East) return true;
    return false;
}

StepResult GridWorld::step(Action action) {
    float reward = config_.TIME_PENALTY;
    bool done = false;

    // Handle Pit Timer
    if (current_state_.pit_timer > 0) {
        current_state_.pit_timer -= 1;
        current_state_.speed = 0;

        if (current_state_.pit_timer == 0) {
            current_state_.tire_health = 1.0f;
        }
        else {
            return StepResult{current_state_, reward, done};
        }
    }

    if (action == Action::Accelerate)
        current_state_.speed = std::min(current_state_.speed + 1, config_.MAX_SPEED);
    else if (action == Action::Brake)
        current_state_.speed = std::max(current_state_.speed - 1, 0);
    else if (action == Action::TurnLeft) {
        current_state_.heading = static_cast<Heading>((static_cast<int>(current_state_.heading) + 3) % 4);
    }
    else if (action == Action::TurnRight) {
        current_state_.heading = static_cast<Heading>((static_cast<int>(current_state_.heading) + 1) % 4);
    }

    const Cell& current_cell = getCell(current_state_.position.x, current_state_.position.y);

    if (isPitTerrain(current_cell.feature) && current_state_.speed > config_.PITLANE_LIMITER) {
        current_state_.speed = config_.PITLANE_LIMITER;
        reward += config_.SPEEDING_PENALTY;
    }

    Position forward = getForwardVector(current_state_.heading);

    for (int i = 0; i < current_state_.speed; ++i) {
        Position next_pos{current_state_.position.x + forward.x, current_state_.position.y + forward.y};
        const Cell& next_cell = getCell(next_pos.x, next_pos.y);
        if (next_cell.surface == Surface::Wall || next_cell.surface == Surface::OutOfTrack) {
            current_state_.crashed = true;
            current_state_.speed = 0;
            reward += config_.CRASH_PENALTY;
            done = true;
            break;
        }

        current_state_.position = next_pos;

        float wear = config_.TIRE_WEAR_TARMAC;
        if (next_cell.surface == Surface::Gravel) {
            wear = config_.TIRE_WEAR_GRAVEL;
            current_state_.lap_invalidated = true;
            current_state_.speed = std::max(0, current_state_.speed - 1);
            reward += config_.TRACK_LIMIT_PENALTY;
        }

        current_state_.tire_health = std::max(0.0f, current_state_.tire_health - wear);
        reward -= (1.0f - current_state_.tire_health) *
                  (1.0f - current_state_.tire_health);  // Progresivno vise boli vozit na starim gumama

        if (next_cell.surface == Surface::Tarmac) {
            if (isOppositeHeading(current_state_.heading, next_cell.expected_heading)) {
                reward += config_.DRIVING_BACKWARD_PENALTY;
            }
            else if (current_state_.heading == next_cell.expected_heading) {
                reward += config_.DRIVING_CORRECT_WAY_REWARD;
            }
        }

        if (next_cell.feature == Feature::StartFinishLine) {
            if (!current_state_.lap_invalidated) {
                current_state_.lap_count++;
                reward += config_.LAP_REWARD;
            }
            current_state_.lap_invalidated = false;
        }
        if (next_cell.feature == Feature::PitBox && current_state_.speed - i - 1 == 0) {
            current_state_.pit_timer = config_.PIT_STOP_DURATION;
        }
    }

    if (current_state_.tire_health <= 0.0f) {
        current_state_.crashed = true;
        current_state_.speed = 0;
        reward += config_.CRASH_PENALTY;
        done = true;
    }
    return StepResult{current_state_, reward, done};
}
}  // namespace F1RL