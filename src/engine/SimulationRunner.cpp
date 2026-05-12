#include "SimulationRunner.hpp"

#include <iostream>

namespace F1RL {

SimulationRunner::SimulationRunner(GridWorld& world, QLearningAgent& agent, const AppConfig& appConfig)
    : world_(world), agent_(agent), appConfig_(appConfig) {
    window_.create(sf::VideoMode({static_cast<unsigned int>(world_.getWidth() * appConfig.cellSize),
                                  static_cast<unsigned int>(world_.getHeight() * appConfig.cellSize)}),
                   appConfig.mode == RunMode::AI ? "F1 Grid World - AI Mode" : "F1 Grid World - Manual Mode");

    window_.setFramerateLimit(60);
}

void SimulationRunner::runSimulation() {
    sf::Clock aiClock;

    world_.reset();

    while (window_.isOpen()) {
        std::optional<Action> action;

        while (const std::optional event = window_.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window_.close();
            }

            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::R) {
                    world_.reset();
                }

                if (appConfig_.mode == RunMode::Manual) {
                    action = getManualAction(*keyPressed);
                }
            }
        }

        if (appConfig_.mode == RunMode::AI) {
            if (aiClock.getElapsedTime().asSeconds() >= appConfig_.aiStepDelaySeconds) {
                aiClock.restart();

                Observation obs = world_.getObservation();

                action = agent_.act(obs, true);
            }
        }

        if (action.has_value()) {
            StepResult result = world_.step(*action);

            printStepInfo(result);

            if (result.done) {
                std::cout << "Episode finished. Resetting..." << std::endl;

                world_.reset();
            }
        }

        window_.clear(sf::Color(30, 30, 30));

        drawWorld();

        window_.display();
    }
}

std::optional<Action> SimulationRunner::getManualAction(const sf::Event::KeyPressed& keyPressed) {
    switch (keyPressed.code) {
        case sf::Keyboard::Key::Up:
            return Action::Accelerate;

        case sf::Keyboard::Key::Down:
            return Action::Brake;

        case sf::Keyboard::Key::Left:
            return Action::TurnLeft;

        case sf::Keyboard::Key::Right:
            return Action::TurnRight;

        case sf::Keyboard::Key::Space:
            return Action::Keep;

        default:
            return std::nullopt;
    }
}

const void SimulationRunner::drawWorld() {
    float cellSize = appConfig_.cellSize;

    sf::RectangleShape rect({cellSize - 1.0f, cellSize - 1.0f});

    for (int y = 0; y < world_.getHeight(); ++y) {
        for (int x = 0; x < world_.getWidth(); ++x) {
            const Cell& cell = world_.getCell(x, y);

            rect.setPosition({x * cellSize, y * cellSize});
            rect.setFillColor(getCellColor(cell));

            window_.draw(rect);
        }
    }

    const State& state = world_.getState();

    sf::CircleShape car(cellSize / 3.0f);
    car.setFillColor(sf::Color::Cyan);

    car.setOrigin({cellSize / 3.0f, cellSize / 3.0f});

    car.setPosition({state.position.x * cellSize + cellSize / 2.0f, state.position.y * cellSize + cellSize / 2.0f});

    window_.draw(car);
}

sf::Color SimulationRunner::getCellColor(const Cell& cell) {
    if (cell.surface == Surface::OutOfTrack) return sf::Color(50, 150, 50);

    switch (cell.feature) {
        case Feature::StartFinishLine:
            return sf::Color::White;

        case Feature::PitEntry:
            return sf::Color(200, 200, 0);

        case Feature::PitLane:
            return sf::Color(100, 100, 255);

        case Feature::PitBox:
            return sf::Color(0, 0, 255);

        default:
            return sf::Color(80, 80, 80);
    }
}

void SimulationRunner::printStepInfo(const StepResult& result) {
    const State& state = world_.getState();

    std::cout << "Speed: " << state.speed << " | Tire Health: " << state.tire_health
              << " | Lap Count: " << state.lap_count << " | Reward: " << result.reward << std::endl;
}

void SimulationRunner::printObservation(const Observation& obs) {
    std::cout << "Observation => "
              << "turn_dist: " << static_cast<int>(obs.turn_dist) << ", turn_dir: " << static_cast<int>(obs.turn_dir)
              << ", pit_dist: " << static_cast<int>(obs.pit_dist) << ", pit_dir: " << static_cast<int>(obs.pit_dir)
              << ", speed: " << static_cast<int>(obs.speed) << ", tire_state: " << static_cast<int>(obs.tire_state)
              << std::endl;
}

}  // namespace F1RL