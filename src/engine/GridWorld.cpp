#include "GridWorld.hpp"

#include <algorithm>
#include <cmath>

namespace F1RL {

static Position getForwardVector(Heading h);
static uint8_t classifyRelativeDirection(Heading heading, const Position& offset);
static uint8_t classifyTurnDirection(Heading from, Heading to);

GridWorld::GridWorld(const MapData& map, const SimulationConfig& config) {
    width_ = map.width;
    height_ = map.height;
    grid_ = map.grid;
    pit_entry_ = map.pitEntry;
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

Observation GridWorld::getObservation() const {
    Observation obs{};
    obs.speed = current_state_.speed;

    if (current_state_.tire_health > config_.TIRE_HEALTH_WORN_THRESHOLD)
        obs.tire_state = 1;
    else if (current_state_.tire_health > config_.TIRE_HEALTH_CRITICAL_THRESHOLD)
        obs.tire_state = 2;
    else
        obs.tire_state = 3;

    const Position forward = getForwardVector(current_state_.heading);

    int turn_distance = 0;
    Heading turn_heading = current_state_.heading;

    const int lookahead = std::clamp(config_.OBSERVATION_LOOKAHEAD, 1, 6);

    for (int i = 0; i <= lookahead; ++i) {
        const Position sample{current_state_.position.x + forward.x * i, current_state_.position.y + forward.y * i};
        const Cell& cell = getCell(sample.x, sample.y);
        if (cell.surface == Surface::OutOfTrack) {
            break;
        }

        turn_distance = i;
        if (cell.surface == Surface::Tarmac && cell.expected_heading != current_state_.heading) {
            turn_heading = cell.expected_heading;
            break;
        }
    }

    obs.turn_dist = static_cast<uint8_t>(std::clamp(turn_distance, 0, lookahead));
    obs.turn_dir = classifyTurnDirection(current_state_.heading, turn_heading);

    const Position pit_offset{pit_entry_.x - current_state_.position.x, pit_entry_.y - current_state_.position.y};
   
    const int forward_dist = pit_offset.x * forward.x + pit_offset.y * forward.y;
    const int lateral_dist = std::abs(forward.x * pit_offset.y - forward.y * pit_offset.x);

    int pit_distance = lookahead;

    if (forward_dist >= 0 && lateral_dist <= config_.PIT_LATERAL_THRESHOLD) {
        pit_distance = std::min(forward_dist, lookahead);
    }

    obs.pit_dist = static_cast<uint8_t>(pit_distance);
    obs.pit_dir = classifyRelativeDirection(current_state_.heading, pit_offset);

    return obs;
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

static uint8_t classifyRelativeDirection(Heading heading, const Position& offset) {
    const Position forward = getForwardVector(heading);
    const int dot = offset.x * forward.x + offset.y * forward.y;
    const int cross = forward.x * offset.y - forward.y * offset.x;

    if (dot > 0 && std::abs(cross) <= dot) {
        return 0;
    }

    if (cross < 0) {
        return 1;
    }

    if (cross > 0) {
        return 2;
    }

    return 0;
}

static uint8_t classifyTurnDirection(Heading from, Heading to) {
    if (from == to) return 0;

    if ((from == Heading::North && to == Heading::West) ||
        (from == Heading::West && to == Heading::South) ||
        (from == Heading::South && to == Heading::East) ||
        (from == Heading::East && to == Heading::North)) {
        return 1;
    }

    if ((from == Heading::North && to == Heading::East) ||
        (from == Heading::East && to == Heading::South) ||
        (from == Heading::South && to == Heading::West) ||
        (from == Heading::West && to == Heading::North)) {
        return 2;
    }

    return 0;
}

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
        if (next_cell.surface == Surface::OutOfTrack) {
            current_state_.crashed = true;
            current_state_.speed = 0;
            reward += config_.CRASH_PENALTY;
            done = true;
            break;
        }

        current_state_.position = next_pos;

        current_state_.tire_health = std::max(0.0f, current_state_.tire_health - config_.TIRE_WEAR_TARMAC);
        //reward -= (1.0f - current_state_.tire_health) *
        //          (1.0f - current_state_.tire_health);  // Progresivno vise boli vozit na starim gumama

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