#include "WorldLoader.hpp"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace F1RL {
static void fillRect(MapData& map, int x1, int y1, int x2, int y2, Surface s, Feature f) {
    for (size_t y = y1; y <= y2; ++y) {
        for (size_t x = x1; x <= x2; ++x) {
            if (x >= 0 && x < map.width && y >= 0 && y < map.height) {
                map.grid[y * map.width + x] = {s, f};
            }
        }
    }
}

MapData WorldLoader::loadFromJson(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open JSON map file: " + filepath);
    }

    json j;
    file >> j;

    size_t w = j["map"]["width"];
    size_t h = j["map"]["height"];
    std::vector<Cell> grid(w * h);

    auto& legend = j["map"]["legend"];
    std::vector<std::string> layout = j["map"]["layout"];

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            std::string sym(1, layout[y][x]);
            std::string type = legend.value(sym, "Wall");

            Cell& c = grid[y * w + x];

            if (type == "OutOfTrack") {
                c.surface = Surface::OutOfTrack;
            }
            else {
                c.surface = Surface::Tarmac;
                if (type == "North") c.expected_heading = Heading::North;
                if (type == "South") c.expected_heading = Heading::South;
                if (type == "East") c.expected_heading = Heading::East;
                if (type == "West") c.expected_heading = Heading::West;
            }
        }
    }

    for (auto& feat : j["features"]) {
        std::string type = feat["type"];

        if (feat.contains("rect")) {
            auto r = feat["rect"];
            for (int y = r[1]; y <= (int)r[3]; ++y) {
                for (int x = r[0]; x <= (int)r[2]; ++x) {
                    if (type == "PitLane") grid[y * w + x].feature = Feature::PitLane;
                }
            }
        }
        else {
            int x = feat["x"];
            int y = feat["y"];
            if (type == "StartFinish") grid[y * w + x].feature = Feature::StartFinishLine;
            if (type == "PitEntry") grid[y * w + x].feature = Feature::PitEntry;
            if (type == "PitBox") grid[y * w + x].feature = Feature::PitBox;
        }
    }

    return MapData{w, h, grid};
}
}  // namespace F1RL
