//
// Created by Benny Gaechter on 24.04.2024.
//

#include "battlesnake.h"

#include <iostream>
#include <utility>
#include <iomanip>


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
        // Create obstacles, heads and food array for faster retrieval
        obstacles_array = std::vector<std::vector<int>>(height, std::vector<int>(width, 0));
        heads_array = std::vector<std::vector<int>>(height, std::vector<int>(width, 0));
        for (const Snake& s : snakes) {
            std::vector<Coord> sbody = s.getBody();
            int slength = s.getLength();
            heads_array[sbody[0].y][sbody[0].x] = slength;
            for (int i=0; i<slength-1; i++) {
                obstacles_array[sbody[i].y][sbody[i].x] = slength - (i+1);
            }
        }
        food_array = std::vector<std::vector<bool>>(height, std::vector<bool>(width, false));
        for (const Coord& c : food) {
            food_array[c.y][c.x] = true;
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
    std::vector<std::vector<int>> Board::getObstacles() const {
        return obstacles_array;
    }

    //Returns 2d vector of bools for all food positions
    std::vector<std::vector<bool>> Board::getFood() const {
        return food_array;
    }

    std::vector<std::vector<int>> Board::getHeads() const {
        return heads_array;
    }

    //Returns unordered_map<snake_id, snake_length> 
    std::unordered_map<std::string, int> Board::getSnakeLengths() const {
        std::unordered_map<std::string, int> snake_lengths;
        for (const Snake& s : snakes) {
            snake_lengths.insert({s.id, s.getLength()});
        }
        return snake_lengths;
    }

    //Simulates available movement options from a given position, sim_turns in the future
    std::vector<Coord> Board::simulateOptions(const Coord& pos, const int& sim_time) const {
        std::vector<Coord> neighbors = getNeighbors(pos);
        std::vector<Coord> safe;
        for (Coord c : neighbors) {
            if (sim_time >= obstacles_array[c.y][c.x]) {
                safe.push_back(c);
            }
        }
        return safe;
    }

    //Uses an optimistic DFS search to find the longest possible path from the starting position
    // optimistic because it doesn't account for the movement of snake heads
    int Board::measureVolume(const Coord& start, const int& subject_length) const {
        int volume = 0;
        std::vector<Coord> frontier_heads = {start};
        std::vector<std::vector<Coord>> frontier_paths = {{start}};
        std::vector<std::vector<int>> visited = std::vector<std::vector<int>>(
            height, std::vector<int>(width, 0)
        );
        while (volume <= subject_length) {
            if (frontier_heads.empty()) {
                return volume;
            }
            Coord next_pos = frontier_heads.back();
            frontier_heads.pop_back();
            std::vector<Coord> cur_path = frontier_paths.back();
            frontier_paths.pop_back();
            int cur_path_length = static_cast<int>(cur_path.size());
            if (cur_path_length > volume) {
                volume = cur_path_length;
            }
            std::vector<Coord> to_expand = simulateOptions(next_pos, cur_path_length);
            for (Coord c : to_expand) {
                //Check if this path intersects itself to soon
                bool self_intersect = false;
                for (int i=0; i<cur_path_length; i++) {
                    if (c == cur_path[i] && (cur_path_length - i) <= (subject_length)) {
                        self_intersect = true;
                    }
                }
                if (self_intersect) {
                    continue;
                }
                //Check if this is longest path found to this point
                if (cur_path_length+1 > visited[c.y][c.x]) {
                    //Mark visited and expand
                    visited[c.y][c.x] = cur_path_length + 1;
                    cur_path.push_back(c);
                    frontier_heads.push_back(c);
                    frontier_paths.push_back(cur_path);
                }
            }
        }
        return volume;
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

    int Snake::getLength() const {
        return length;
    }

    std::vector<Coord> Snake::getBody() const {
        return body;
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

    //Retuns a bool that is true if this snake needs to eat soon
    bool Snake:: getHunger(const Board& board) const {
        //Hungry no matter what, once health get's low
        if (health < 20) {
            return true;
        }
        std::unordered_map<std::string, int> snake_lengths = board.getSnakeLengths();
        //Hungry anytime we aren't the biggest snake by 2
        for (auto& id_length : snake_lengths) {
            if (id_length.second >= length+2  && id_length.first != id) {
                return true;
            }
        }
        return false;
    }

    std::string Snake::getMove(const Board& board) const {
        //Set priorities
        bool hungry = getHunger(board);
        //Get in-bounds adjacent coords
        std::vector<Coord> neighbors = board.getNeighbors(head);
        //Avoid obstacles
        std::vector<std::vector<int>> obstacles = board.getObstacles();
        std::vector<std::vector<int>> heads = board.getHeads();
        std::vector<Coord> obstacle_free;
        std::vector<Coord> head_on_risk;
        for (const Coord& c : neighbors) {
            if (!obstacles[c.y][c.x]) {
                bool head_risk = false;
                std::vector<Coord> maybe_heads = board.getNeighbors(c);
                for (Coord pos : maybe_heads) {
                    if (!(pos == head) && heads[pos.y][pos.x] >= length) {
                        head_risk = true;
                        head_on_risk.push_back(c);
                        break;
                    }
                }
                if (!head_risk) {
                    obstacle_free.push_back(c);
                }
            }
        }
        if (!obstacle_free.empty()) {
            std::vector<Coord> safe_volumes;
            for (Coord c : obstacle_free) {
                int volume = board.measureVolume(c, length);
                std::cout << "("<<c.x<<", "<<c.y<<"): "<<volume<<std::endl;
                if (volume >= length) {
                    safe_volumes.push_back(c);
                }
            }
            if (!safe_volumes.empty()) {
                //Choose available food if hungry, otherwise random
                if (hungry) {
                    std::vector<std::vector<bool>> food_array = board.getFood();
                    for (const Coord& c : safe_volumes) {
                        if (food_array[c.y][c.x]) {
                            return getDirectionStr(c);
                        }
                    }
                }
                //Not hungry, or no food available
                return getDirectionStr(safe_volumes[rand() % safe_volumes.size()]);
            } else {
                std::cout << "Crap I'm running out of room!" << std::endl;
                return getDirectionStr(obstacle_free[rand() % obstacle_free.size()]);
            }
        } else {
            if(!head_on_risk.empty()) {
                return getDirectionStr(head_on_risk[rand() % head_on_risk.size()]);
            }
            std::cout << "Crap I'm surrounded!" << std::endl;
            std::cout << "Neighbors: \n";
            for (const Coord& c : neighbors) {
                std::cout << c.x << ", " << c.y << std::endl;
            }
            std::cout << "Obstacles: \n";
            for (int i=obstacles.size()-1; i>=0; i--) {
                std::vector<int>& row = obstacles[i];
                for (int cell : row) {
                    std::cout.fill(' ');
                    std::cout.width(3);
                    std::cout << cell;
                }
                std::cout << "\n";
            }
            return "up";
        }
        std::cout << "Uh Oh :|" << std::endl;
        return "up";
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
        std::cout << "Turn " << turn << ":\n"; 
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
