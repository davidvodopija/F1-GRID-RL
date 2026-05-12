#pragma once
#include <compare>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>

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
};

struct MapData {
    std::size_t width;
    std::size_t height;
    std::vector<Cell> grid;
    Position pitEntry;
};

enum class RunMode { Manual, AI };

struct AppConfig {
    RunMode mode = RunMode::AI;

    std::string trackPath;
    std::string simulationConfigPath;

    float cellSize = 60.0f;
    float aiStepDelaySeconds = 0.5f;
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
    int PIT_STOP_DURATION = 5;
    int MAX_SPEED = 3; // DO NOT CHANGE (toHash change required)
    int PITLANE_LIMITER = 1;
    int OBSERVATION_LOOKAHEAD = 6; // DO NOT CHANGE (toHash change required)
    int PIT_LATERAL_THRESHOLD = 1;
    float TIME_PENALTY = -1.0f;
    float SPEEDING_PENALTY = -3.0f;
    float CRASH_PENALTY = -10000.0f;
    float TIRE_WEAR_TARMAC = 0.002f;
    float TIRE_HEALTH_WORN_THRESHOLD = 0.5f;
    float TIRE_HEALTH_CRITICAL_THRESHOLD = 0.2f;
    float LAP_REWARD = 100.0f;
    float TRACK_LIMIT_PENALTY = -5.0f;
    float DRIVING_BACKWARD_PENALTY = -1000.0f;
    float DRIVING_CORRECT_WAY_REWARD = 0.8f;
    float ALPHA = 0.1f;
    float GAMMA = 0.99f;
    float EPSILON = 1.0f;
    float MIN_EPSILON = 0.01f;
    float EPSILON_DECAY_RATE = 0.999f;
    int TRAINING_LOG_INTERVAL = 1000;
    int TRAINING_EPISODES = 15000;
};

struct Observation {
    uint8_t turn_dist;  // 0 (Sitting on the corner), 1..6 = too far -> OK FOR SPEED 3
    uint8_t turn_dir;   // 0 (Straight), 1 (Left), 2 (Right)

    uint8_t pit_dist;  // same as turn
    uint8_t pit_dir;   // same as turn

    uint8_t speed;
    uint8_t tire_state;  // Contained: healthy(1), worn(2), critical(3)

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