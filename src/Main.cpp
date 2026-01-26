#include <SFML/Graphics.hpp>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main() {
#ifdef __clang__
    std::cout << "Compiling with Clang " << __clang_major__ << "." << __clang_minor__ << std::endl;
#else
    std::cout << "Compiling with MSVC " << std::endl;
#endif

    // 1. Test JSON
    json j;
    j["project"] = "F1 Grid World";
    j["tire_health"] = 1.0;
    std::cout << "JSON Config: " << j.dump(4) << std::endl;

    // 2. Test SFML Window
    sf::RenderWindow window(sf::VideoMode({800, 600}), "F1 RL Environment");
    sf::CircleShape car(20.f);
    car.setFillColor(sf::Color::Red);
    car.setPosition({400, 300});

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        window.clear(sf::Color::Black);
        window.draw(car);
        window.display();
    }

    return 0;
}
