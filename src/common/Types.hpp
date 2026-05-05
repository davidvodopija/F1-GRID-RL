#pragma once
#include <cstddef>
#include <compare>
#include <cstdint>
#include <vector>

namespace F1RL {

enum class Surface : uint8_t {
    Tarmac,
    OutOfTrack,
};

enum class Feature : uint8_t { None, StartFinishLine, PitEntry, PitLane, PitBox };

enum class Heading : uint8_t { North, East, South, West };

struct Cell {
    Surface surface = Surface::Tarmac;
    Feature feature = Feature::None;
    Heading expected_heading;
};

struct Position {
    int x = 0;
    int y = 0;
    auto operator<=>(const Position&) const = default;
};

struct MapData {
    std::size_t width;
    std::size_t height;
    std::vector<Cell> grid;
    Position pitEntry;
};

enum class Action : uint8_t { Accelerate, Brake, TurnLeft, TurnRight, Keep };

struct State {
    Position position;
    Heading heading;
    uint8_t speed;
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
    float TIRE_WEAR_TARMAC;
    float LAP_REWARD;
    float TRACK_LIMIT_PENALTY;
    float DRIVING_BACKWARD_PENALTY;
    float DRIVING_CORRECT_WAY_REWARD;
};

struct Observation {
    uint8_t turn_dist;  // 0 (Sitting on the corner), 1..6 = too far -> OK FOR SPEED 3
    uint8_t turn_dir;   // 0 (Straight), 1 (Left), 2 (Right)

    uint8_t pit_dist;  // same as turn
    uint8_t pit_dir;   // same as turn

    uint8_t speed;
    uint8_t tire_state;  // Contained: 0.5-1.0 healthy(1), 0.2-0.5 worn(2), <0.2 critical(3)

    [[nodiscard]] std::size_t toHash() const {
        std::size_t hash = 0;
        int offset = 0;

        hash |= (static_cast<size_t>(turn_dist) << offset);
        offset += 3;
        hash |= (static_cast<size_t>(turn_dir) << offset);
        offset += 2;
        hash |= (static_cast<size_t>(pit_dist) << offset);
        offset += 3;
        hash |= (static_cast<size_t>(pit_dir) << offset);
        offset += 2;
        hash |= (static_cast<size_t>(speed) << offset);
        offset += 2;
        hash |= (static_cast<size_t>(tire_state) << offset);
        offset += 2;

        return hash;
    }
};

}  // namespace F1RL