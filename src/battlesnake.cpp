//
// Created by Benny Gaechter on 24.04.2024.
//

#include "battlesnake.h"

#include <iostream>
#include <utility>


namespace battlesnake {
    BattleSnake::BattleSnake() {
        Info const i{};
        info = i;
    }

    std::string BattleSnake::getInfo() const {
        return info.GetInfo();
    }

    std::string BattleSnake::end() {
        return "End";
    }

    std::string BattleSnake::make_move(const json& state) {
        //Print the state
        //std::cout << state.dump() << std::endl;
        auto const gameState = GameState(state);
        //Get my next move
        std::string my_move = gameState.getMyMove();

        
        // Create response object
        json response{};
        response["move"] = my_move;
        response["shout"] = "I'm walkin here!";

        return response.dump();
    }

    std::string BattleSnake::start() {
        return "";
    }

    Coord::Coord(json coord): x(coord["x"]), y(coord["y"]) {
    }

    Coord::Coord(int xcoord, int ycoord): x(xcoord), y(ycoord) {
    }

    Board::Board(const json& board): height(board["height"]), width(board["width"]) {
        for (const auto &food_coordinates: board["food"]) {
            food.emplace_back(food_coordinates);
        }
        for (const auto &hazard_coordinates: board["hazards"]) {
            hazards.emplace_back(hazard_coordinates);
        }
        for (const auto &snake: board["snakes"]) {
            snakes.emplace_back(snake);
        }
    }

    //Returns all adjacent positions which are in-bounds
    std::vector<Coord> Board::getNeighbors(const Coord& pos) const {
        std::vector<Coord> neighbors;
        if (pos.x > 0) {
            neighbors.emplace_back(pos.x - 1, pos.y);
        }
        if (pos.x < width - 1) {
            neighbors.emplace_back(pos.x + 1, pos.y);
        }
        if (pos.y > 0) {
            neighbors.emplace_back(pos.x, pos.y - 1);
        }
        if (pos.y < height - 1) {
            neighbors.emplace_back(pos.x, pos.y + 1);
        }
        return neighbors;
    }

    //Returns 2d vector of bools for all board positions, true when likely lethal to move into
    std::vector<std::vector<bool>> Board::getObstacles() const {
        std::vector<std::vector<bool>> obstacles(height, std::vector<bool>(width, false));
        for (const Snake &s : snakes) {
            std::vector<Coord> next_body = s.getNextTurnBody();
            for (Coord &c : next_body) {
                obstacles[c.y][c.x] = true;
            }
        }
        return obstacles;
    }

    Snake::Snake(const json& snake): head(snake["head"]), customizations(snake["customizations"]) {
        id = snake["id"];
        name = snake["name"];
        health = snake["health"];
        for (const auto &snake_body: snake["body"]) {
            body.emplace_back(snake_body);
        }
        length = snake["length"];
        shout = snake["shout"];
        expanding = false;

        // The implementation varies, sometime it is an int sometimes a string
        if(snake["latency"].is_string()) {
            try {
                latency = std::stoi(snake["latency"].get<std::string>());
            } catch (...) {
                // Not a valid number
                latency = 0;
            }
        } else if(snake["latency"].is_number()) {
            latency = snake["latency"];
        }
    }

    //Returns coords of where this snake's body will be next turn (not head)
    std::vector<Coord> Snake::getNextTurnBody() const {
        std::vector<Coord> next_body = body;
        int i = next_body.size()-1;
        while (next_body[i] == next_body[i-1]) {
            next_body.pop_back();
            i--;
        }
        return next_body;
    }

    //Returns strings up, down, left, or right based on relative direction from snake's head
    // destination must be exactly one space away from snake's head
    std::string Snake::getDirectionStr(const Coord& destination) const {
        assert((abs(head.x - destination.x) + abs(head.y - destination.y)) == 1);

        if (head.x == destination.x) {
            if (head.y < destination.y) {
                return "up";
            } else {
                return "down";
            }
        } else {
            if (head.x < destination.x) {
                return "right";
            } else {
                return "left";
            }
        }
    }

    std::string Snake::getMove(const Board& board) const {
        //Get in-bounds adjacent coords
        std::vector<Coord> neighbors = board.getNeighbors(head);
        //Avoid obstacles
        std::vector<std::vector<bool>> obstacles = board.getObstacles();
        std::vector<Coord> obstacle_free;
        for (const Coord& c : neighbors) {
            if (!obstacles[c.y][c.x]) {
                obstacle_free.push_back(c);
            }
        }
        if (obstacle_free.size() == 0) {
            std::cout << "Crap I'm surrounded!" << std::endl;
            std::cout << "Neighbors: \n";
            for (const Coord& c : neighbors) {
                std::cout << c.x << ", " << c.y << std::endl;
            }
            std::cout << "Obstacles: \n";
            for (int i=obstacles.size()-1; i>=0; i--) {
                std::vector<bool>& row = obstacles[i];
                for (bool cell : row) {
                    if (cell) {
                        std::cout << "X";
                    } else {
                        std::cout << "O";
                    }
                }
                std::cout << "\n";
            }
            return "up";
        } else {
            //Choose randomly from options
            return getDirectionStr(obstacle_free[rand() % obstacle_free.size()]);
        }
    }

    RulesetSettings::RulesetSettings(json ruleset_settings): foodspawnChance(ruleset_settings["foodSpawnChance"]),
                                                            minimumFood(ruleset_settings["minimumFood"]) {
        if(ruleset_settings.contains("hazardDamagePerTurn")) {
            hazardDamagePerTurn = ruleset_settings["hazardDamagePerTurn"];
        } else {
            hazardDamagePerTurn = 0;
        }
    }

    Ruleset::Ruleset(json ruleset): settings(ruleset["settings"]) {
        name = ruleset["name"];
        version = ruleset["version"];
    }

    Game::Game(const json& game): ruleset(game["ruleset"]) {
        id = game["id"];
        map = game["map"];
        source = game["source"];
        timeout = game["timeout"];
    }

    GameState::GameState(const json& state): game(state["game"]), board(state["board"]), you(Snake{state["you"]}) {
        turn = state["turn"];
    }

    std::string GameState::getMyMove() const {
        return you.getMove(board);
    }

    Customizations::Customizations(json customizations) {
        color = customizations["color"];
        head = customizations["head"];
        tail = customizations["tail"];
    }

    Info::Info() {
        info["APIVersion"] = "1";
        info["Author"] = "C++";
        info["Color"] = "#c0ffee";
        info["Head"] = "default";
        info["Tail"] = "default";
    }

    Info::Info(const std::string &apiVersion, const std::string &author, const std::string &color,
               const std::string &head, const std::string &tail) {
        info["APIVersion"] = apiVersion;
        info["Author"] = author;
        info["Color"] = color;
        info["Head"] = head;
        info["Tail"] = tail;
    }

    std::string Info::GetInfo() const {
        return info.dump();
    }
} // battlesnake
