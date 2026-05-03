#include <SFML/Graphics.hpp>
#include <iostream>

#include "common/Types.hpp"
#include "engine/GridWorld.hpp"
#include "io/WorldLoader.hpp"
#include "io/ConfigLoader.hpp"

using namespace F1RL;

int main() {
    MapData map = WorldLoader::loadFromJson("assets/track1.json");
    SimulationConfig simulationConfig = ConfigLoader::loadSimulationConfig("assets/simulation_config.config");
    GridWorld world(map, simulationConfig);

    float cellSize = 60.0f;
    sf::RenderWindow window(
        sf::VideoMode({(unsigned int)(world.getWidth() * cellSize), (unsigned int)(world.getHeight() * cellSize)}),
        "F1 Grid World");
    window.setFramerateLimit(60);

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();

            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                Action act = Action::Keep;
                if (keyPressed->code == sf::Keyboard::Key::Up)
                    act = Action::Accelerate;
                else if (keyPressed->code == sf::Keyboard::Key::Down)
                    act = Action::Brake;
                else if (keyPressed->code == sf::Keyboard::Key::Left)
                    act = Action::TurnLeft;
                else if (keyPressed->code == sf::Keyboard::Key::Right)
                    act = Action::TurnRight;
                else if (keyPressed->code == sf::Keyboard::Key::R)
                    world.reset();

                if (act != Action::Keep || keyPressed->code == sf::Keyboard::Key::Space) {
                    StepResult result = world.step(act);
                    std::cout << " | Speed: " << world.getState().speed
                              << " | Tire Health: " << world.getState().tire_health
                              << " | Lap Count: " << world.getState().lap_count << " | Reward: " << result.reward
                              << std::endl;
                    if (result.done) {
                        world.reset();
                    }
                }
            }
        }

        window.clear(sf::Color(30, 30, 30));

        sf::RectangleShape rect({cellSize - 1.0f, cellSize - 1.0f});

        for (int y = 0; y < world.getHeight(); ++y) {
            for (int x = 0; x < world.getWidth(); ++x) {
                const Cell& cell = world.getCell(x, y);
                rect.setPosition({x * cellSize, y * cellSize});

                // Base Surface
                if (cell.surface == Surface::OutOfTrack)
                    rect.setFillColor(sf::Color(50, 150, 50));
                else
                    rect.setFillColor(sf::Color(80, 80, 80));

                // Features Overlay
                switch (cell.feature) {
                    case Feature::StartFinishLine:
                        rect.setFillColor(sf::Color::White);
                        break;
                    case Feature::PitEntry:
                        rect.setFillColor(sf::Color(200, 200, 0));
                        break;
                    case Feature::PitLane:
                        rect.setFillColor(sf::Color(100, 100, 255));
                        break;
                    case Feature::PitBox:
                        rect.setFillColor(sf::Color(0, 0, 255));
                        break;
                    default:
                        break;
                }

                window.draw(rect);
            }
        }

        // Draw Car
        State state = world.getState();
        sf::CircleShape car(cellSize / 3);
        car.setFillColor(sf::Color::Cyan);
        car.setOrigin({cellSize / 3, cellSize / 3});
        car.setPosition({state.position.x * cellSize + cellSize / 2, state.position.y * cellSize + cellSize / 2});
        window.draw(car);
        window.display();
    }

    return 0;
}
