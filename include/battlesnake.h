//
// Created by Benny Gaechter on 24.04.2024.
//

#ifndef BATTLESNAKE_H
#define BATTLESNAKE_H
#include "json.h"
#include <string>
using json = nlohmann::json;

namespace battlesnake {
    class Board;
    class Coord {
    public:
        explicit Coord(json coord);
        explicit Coord(int x, int y);
        bool operator==(const Coord& other) const {
            return x == other.x && y == other.y;
        }

        int x;
        int y;
    };

    class Customizations {
    public:
        explicit Customizations(json customizations);

    private:
        std::string color;
        std::string head;
        std::string tail;
    };

    class Board {
    public:
        struct Snake {
            std::string m_id;
            Coord m_head;
            Customizations m_customizations;
            std::string m_name;
            int m_health;
            int m_length;
            std::string m_shout;
            int m_latency;
            std::vector<Coord> m_body;

            explicit Snake(const json& snake);
            std::string getDirectionStr(const Coord& destination) const;
        };
        struct AStarFrontierNode {
            std::vector<Coord> path;
            int food_count;
            int man_dist;

            bool operator>(const AStarFrontierNode& other) const {
                return (man_dist + path.size()) > (other.man_dist + other.path.size());
            }
        };
        struct HeadThreatFNode {
            std::vector<Coord> path;
            int food_count;
        };

        explicit Board(const json& board);
        std::vector<Coord> getNeighbors(const Coord& pos) const;
        std::vector<std::vector<int>> getObstacles() const;
        std::vector<std::vector<bool>> getFood() const;
        std::vector<std::vector<int>> getHeadsArray() const;
        bool getHunger(const Snake& subject) const;
        std::vector<Coord> simulateOptions(const Coord& pos, const int& sim_time) const;
        int measureVolume(
            const Coord& start, const int& subject_length, bool avoid_heads, 
            const std::vector<std::vector<int>>* head_threats = nullptr
        ) const;
        int manDist(const Coord& start_pos, const Coord& end_pos) const;
        std::vector<Coord> aStar(const Coord& start_pos, const Coord& end_pos) const;
        int getFoodDist(const Coord& pos) const;
        std::vector<std::vector<int>> getHeadThreat(const Snake& subject) const;
        std::string getMove(const std::string& snake_id) const;

        int m_height;
        int m_width;
        std::vector<Snake> m_snakes;
        std::vector<std::vector<int>> m_heads_array;
        std::vector<std::vector<int>> m_obstacles_array;
        std::vector<std::vector<bool>> m_food_array;

    private:
        std::vector<Coord> m_food;
        std::vector<Coord> m_hazards;
    };

    class RulesetSettings {
    public:
        explicit RulesetSettings(json ruleset_settings);

    private:
        int foodspawnChance;
        int minimumFood;
        int hazardDamagePerTurn;
    };

    class Ruleset {
    public:
        explicit Ruleset(json ruleset);

    private:
        std::string m_name;
        std::string version;
        RulesetSettings settings;
    };

    class Game {
    public:
        explicit Game(const json& game);
        std::string id;

    private:
        Ruleset ruleset;
        std::string map;
        std::string source;
        int timeout;
    };

    class GameState {
    public:
        explicit GameState(const json& state);
        std::string getMyMove() const;

    private:
        Game game;
        int turn;
        Board board;
        std::string you_id;
    };

    class Move {
    };

    class Info {
    public:
        Info();

        Info(const std::string &apiVersion, const std::string &author, const std::string &color,
             const std::string &head, const std::string &tail);

        std::string GetInfo() const;

    private:
        json info;
    };

    class BattleSnake {
    public:
        BattleSnake();

        std::string getInfo() const;

        std::string start();

        std::string make_move(const json& state);

        std::string end();

    private:
        Info info;
    };
} // battlesnake

#endif //BATTLESNAKE_H
