//
// Created by Benny Gaechter on 24.04.2024.
//

#include "battlesnake.h"

#include <iostream>
#include <utility>
#include <iomanip>
#include <queue>


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

    Board::Board(const json& board): m_height(board["height"]), m_width(board["width"]) {
        for (const auto &food_coordinates: board["food"]) {
            m_food.emplace_back(food_coordinates);
        }
        for (const auto &hazard_coordinates: board["hazards"]) {
            m_hazards.emplace_back(hazard_coordinates);
        }
        for (const auto &snake: board["snakes"]) {
            m_snakes.emplace_back(snake);
        }
        // Create obstacles, heads, and food array for faster retrieval
        m_obstacles_array = std::vector<std::vector<int>>(m_height, std::vector<int>(m_width, 0));
        m_heads_array = std::vector<std::vector<int>>(m_height, std::vector<int>(m_width, 0));
        for (const Snake& s : m_snakes) {
            std::vector<Coord> sbody = s.getBody();
            int slength = s.getLength();
            m_heads_array[sbody[0].y][sbody[0].x] = slength;
            for (int i=0; i<slength-1; i++) {
                //Distance to tail minus one is the optimistic number of turns until this space can be moved into
                m_obstacles_array[sbody[i].y][sbody[i].x] = slength - (i+1);
            }
        }
        m_food_array = std::vector<std::vector<bool>>(m_height, std::vector<bool>(m_width, false));
        for (const Coord& c : m_food) {
            m_food_array[c.y][c.x] = true;
        }
    }

    //Returns all adjacent positions which are in-bounds
    std::vector<Coord> Board::getNeighbors(const Coord& pos) const {
        std::vector<Coord> neighbors;
        if (pos.x > 0) {
            neighbors.emplace_back(pos.x - 1, pos.y);
        }
        if (pos.x < m_width - 1) {
            neighbors.emplace_back(pos.x + 1, pos.y);
        }
        if (pos.y > 0) {
            neighbors.emplace_back(pos.x, pos.y - 1);
        }
        if (pos.y < m_height - 1) {
            neighbors.emplace_back(pos.x, pos.y + 1);
        }
        return neighbors;
    }

    //Returns 2d vector of bools for all board positions, true when likely lethal to move into
    std::vector<std::vector<int>> Board::getObstacles() const {
        return m_obstacles_array;
    }

    //Returns 2d vector of bools for all food positions
    std::vector<std::vector<bool>> Board::getFood() const {
        return m_food_array;
    }

    std::vector<std::vector<int>> Board::getHeadsArray() const {
        return m_heads_array;
    }

    //Returns unordered_map<snake_id, snake_length> 
    std::unordered_map<std::string, int> Board::getSnakeLengths() const {
        std::unordered_map<std::string, int> snake_lengths;
        for (const Snake& s : m_snakes) {
            snake_lengths.insert({s.m_id, s.getLength()});
        }
        return snake_lengths;
    }

    //Simulates available movement options from a given position, sim_turns in the future
    std::vector<Coord> Board::simulateOptions(const Coord& pos, const int& sim_time) const {
        std::vector<Coord> neighbors = getNeighbors(pos);
        std::vector<Coord> safe;
        for (Coord c : neighbors) {
            if (sim_time > m_obstacles_array[c.y][c.x]) {
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
            m_height, std::vector<int>(m_width, 0)
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

    int Board::manDist(const Coord& start_pos, const Coord& end_pos) const {
        return abs(end_pos.x - start_pos.x) + abs(end_pos.y - start_pos.y);
    }

    //Performs A* search from start_pos to end_pos. Returns shortest path.
    std::vector<Coord> Board::aStar(const Coord& start_pos, const Coord& end_pos) const {
        //Init frontier
        std::priority_queue<AStarFrontierNode, std::vector<AStarFrontierNode>, std::greater<AStarFrontierNode>> frontier;
        frontier.push({
            {start_pos},
            m_food_array[start_pos.y][start_pos.x] ? 1 : 0,
            manDist(start_pos, end_pos)
        });
        std::vector<std::vector<int>> explored(m_height, std::vector<int>(m_width, -1));
        while (!frontier.empty()) {
            //Pop next position to explore from frontier
            AStarFrontierNode cur_node = frontier.top();
            frontier.pop();
            Coord cur_pos = cur_node.path.back();
            //Return path if we reached the end
            if (cur_pos == end_pos) {
                //std::cout<<"Path from "<<start_pos.x<<", "<<start_pos.y<<" to "<<end_pos.x<<", "<<end_pos.y<<std::endl;
                //std::cout<<"Length: "<<cur_node.path.size()<<std::endl;
                return cur_node.path;
            }
            //If position unexplored or explored from a longer path, explore it
            if (explored[cur_pos.y][cur_pos.x] == -1 || explored[cur_pos.y][cur_pos.x] > cur_node.path.size()) {
                explored[cur_pos.y][cur_pos.x] =  cur_node.path.size();
                std::vector<Coord> to_expand = simulateOptions(cur_pos, cur_node.path.size());
                for (Coord c : to_expand) {
                    std::vector<Coord> new_path = cur_node.path;
                    new_path.push_back(c);
                    frontier.push({
                        new_path,
                        m_food_array[c.y][c.x] ? cur_node.food_count+1 : cur_node.food_count,
                        manDist(c, end_pos)
                    });
                }
            }
        }
        //No path found
        return {};
    }

    //Returns distance of nearest food to pos using A* pathfinding
    int Board::getFoodDist(const Coord& pos) const {
        int shortest_dist = 9999; //Arbitrary upper bound..?
        for (const Coord& food_pos : m_food) {
            //Manhattan distance is a lower bound on A* distance
            if (manDist(pos, food_pos) > shortest_dist) {
                continue;
            }
            std::vector<Coord> path = aStar(pos, food_pos);
            if (!path.empty()) {
                if (path.size() < shortest_dist) {
                    shortest_dist = path.size();
                }
            }
        }
        return shortest_dist;
    }

    Snake::Snake(const json& snake): m_head(snake["head"]), m_customizations(snake["customizations"]) {
        m_id = snake["id"];
        m_name = snake["name"];
        m_health = snake["health"];
        for (const auto &snake_body: snake["body"]) {
            m_body.emplace_back(snake_body);
        }
        m_length = snake["length"];
        m_shout = snake["shout"];

        // The implementation varies, sometime it is an int sometimes a string
        if(snake["latency"].is_string()) {
            try {
                m_latency = std::stoi(snake["latency"].get<std::string>());
            } catch (...) {
                // Not a valid number
                m_latency = 0;
            }
        } else if(snake["latency"].is_number()) {
            m_latency = snake["latency"];
        }
    }

    int Snake::getLength() const {
        return m_length;
    }

    std::vector<Coord> Snake::getBody() const {
        return m_body;
    }

    //Returns strings up, down, left, or right based on relative direction from snake's head
    // destination must be exactly one space away from snake's head
    std::string Snake::getDirectionStr(const Coord& destination) const {
        assert((abs(m_head.x - destination.x) + abs(m_head.y - destination.y)) == 1);

        if (m_head.x == destination.x) {
            if (m_head.y < destination.y) {
                return "up";
            } else {
                return "down";
            }
        } else {
            if (m_head.x < destination.x) {
                return "right";
            } else {
                return "left";
            }
        }
    }

    //Retuns a bool that is true if this snake needs to eat soon
    bool Snake::getHunger(const Board& board) const {
        //Hungry no matter what if we are running out of time to run for food
        int dist_to_food = board.getFoodDist(m_head) - 1;   //Counts starting position, we can remove
        if (m_health < dist_to_food + 10) {
            return true;
        }
        std::unordered_map<std::string, int> snake_lengths = board.getSnakeLengths();
        //Hungry anytime we aren't the biggest snake by 2
        for (auto& id_length : snake_lengths) {
            if (id_length.second >= m_length+2  && id_length.first != m_id) {
                return true;
            }
        }
        return false;
    }

    //Filters out certain death moves and then compares different risk categories
    std::string Snake::getMove(const Board& board) const {
        //Get in-bounds adjacent coords
        std::vector<Coord> candidate_moves = board.getNeighbors(m_head);
        //Filter out any self body parts
        candidate_moves.erase(
            std::remove_if(
                candidate_moves.begin(), candidate_moves.end(),
                [this](const Coord& c) {
                    //Ignore last body part as it will move forwards
                    return std::find(m_body.begin(), m_body.end() - 1, c) != m_body.end() - 1;
                }
            ),
            candidate_moves.end()
        );
        //Filter out body parts of other snakes above 1 health
        candidate_moves.erase(
            std::remove_if(
                candidate_moves.begin(), candidate_moves.end(),
                [board](const Coord& c) {
                    for (const Snake& s : board.m_snakes) {
                        if (std::find(s.m_body.begin(), s.m_body.end() - 1, c) != s.m_body.end()-1) {
                            if (s.m_health > 1) {
                                return true;
                            } else {
                                return false;
                            }
                        }
                    }
                    return false;
                }
            ),
            candidate_moves.end()
        );
        if (!candidate_moves.empty()) {
            bool is_hungry = getHunger(board);
            //Now do risk analysis
            std::vector<int> final_risks;
            std::vector<int> food_distances;
            std::cout << "Candidate scores: \n";
            std::cout << "move, final, volume, head on, eating, food distance\n";
            for (const Coord& c : candidate_moves) {
                int volume_risk;
                int head_on_risk = 0;
                int eating_risk = 0;
                int volume = board.measureVolume(c, m_length);
                if (volume < m_length) {
                    volume_risk = -100 - (m_length - volume);
                } else {
                    volume_risk = 0;
                }
                for (const Coord& adj_c : board.getNeighbors(c)) {
                    if (!(adj_c == m_head) && board.m_heads_array[adj_c.y][adj_c.x] != 0) {
                        if (board.m_heads_array[adj_c.y][adj_c.x] >= m_length) {
                            head_on_risk = -10;
                        } else if (board.m_heads_array[adj_c.y][adj_c.x] < m_length) {
                            head_on_risk = 10;
                        }
                    }
                }
                //If a snakes body is still a candidate then it MUST be at 1 health
                //So check if there is risk of this snake surviving the turn by eating
                if (!board.m_obstacles_array[c.y][c.x] == 0) {
                    //TODO: better way to look this up
                    int could_eat = 0;
                    for (const Snake& s : board.m_snakes) {
                        for (const Coord& body_part : s.m_body) {
                            if (c == body_part) {
                                //Found the snake in question, check if it can eat
                                std::vector<Coord> possible_moves = board.getNeighbors(s.m_head);
                                for (const Coord& one_move : possible_moves) {
                                    if (board.m_food_array[one_move.y][one_move.x]) {
                                        could_eat = -10;
                                        break;
                                    }
                                }
                            }
                            if (could_eat == -10) {
                                break;
                            }
                        }
                        if (could_eat == -10) {
                            break;
                        }
                    }
                    eating_risk = could_eat;
                }
                int dist_to_food;
                if (is_hungry) {
                    dist_to_food = board.getFoodDist(c);
                    food_distances.push_back(dist_to_food);
                }
                int final_risk = 0;
                final_risk += volume_risk;
                final_risk += head_on_risk;
                final_risk += eating_risk;
                std::cout << getDirectionStr(c) << ": ";
                std::cout << final_risk << ", ";
                std::cout << volume_risk << ", ";
                std::cout << head_on_risk << ", ";
                std::cout << eating_risk << ", ";
                std::cout << (is_hungry ? std::to_string(dist_to_food) : "N/A") << std::endl;
                final_risks.push_back(final_risk);
            }
            if (is_hungry) {
                int min_distance = *std::min_element(food_distances.begin(), food_distances.end());
                for (size_t i=0; i<food_distances.size(); i++){
                    if (food_distances[i] == min_distance) {
                        final_risks[i] += 1;
                    }
                }
            }
            int high_score = *std::max_element(final_risks.begin(), final_risks.end());
            std::cout << "High score: " << high_score << std::endl;
            //Now choose move from candidates with highest risk score
            std::vector<Coord> final_candidates;
            int i_candidate = 0;
            for (Coord c : candidate_moves) {
                if (final_risks[i_candidate] == high_score) {
                    final_candidates.push_back(c);
                }
                i_candidate++;
            }
            return getDirectionStr(final_candidates[rand() % final_candidates.size()]);
        } else {
            std::cout << "Crap I'm surrounded!" << std::endl;
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
        m_name = ruleset["name"];
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
