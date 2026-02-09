#pragma once
#include <cstdint>
#include <vector>
#include <compare>

namespace F1RL {

enum class Surface : uint8_t {
    Tarmac,
    Gravel,
    Wall,
    OutOfTrack,
};

enum class Feature : uint8_t {
    None,
    StartFinishLine,
    PitEntry,
    PitLane,
    PitBox
};

enum class Heading : uint8_t { North, East, South, West };

struct Cell {
    Surface surface = Surface::Tarmac;
    Feature feature = Feature::None;
    Heading expected_heading;
};

struct MapData {
    size_t width;
    size_t height;
    std::vector<Cell> grid;
};

enum class Action : uint8_t { Accelerate, Brake, TurnLeft, TurnRight, Keep };

struct Position {
    int x = 0;
    int y = 0;
    auto operator<=>(const Position&) const = default;
};

struct State {
    Position position;
    Heading heading;
    int speed;
    float tire_health;  // 0.0 to 1.0
    bool crashed;
    int lap_count;
    int pit_timer;
    bool lap_invalidated;
};

struct StepResult {
    State next_state;
    float reward;
    bool done;
};

struct SimulationConfig {
    int PIT_STOP_DURATION;
    int MAX_SPEED;
    int PITLANE_LIMITER;
    float TIME_PENALTY;
    float SPEEDING_PENALTY;
    float CRASH_PENALTY;
    float TIRE_WEAR_GRAVEL;
    float TIRE_WEAR_TARMAC;
    float LAP_REWARD;
    float TRACK_LIMIT_PENALTY;
    float DRIVING_BACKWARD_PENALTY;
    float DRIVING_CORRECT_WAY_REWARD;
};

}  // namespace F1RL