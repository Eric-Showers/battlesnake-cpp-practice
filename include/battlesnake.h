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

    class Snake {
    public:
        explicit Snake(const json& snake);
        std::vector<Coord> getNextTurnBody() const;
        std::string getDirectionStr(const Coord& destination) const;
        std::string getMove(const Board& board) const;
        std::string id;

    private:
        std::string name;
        int health;
        std::vector<Coord> body;
        Coord head;
        int length;
        int latency;
        std::string shout;
        Customizations customizations;
        bool expanding;
    };

    class Board {
    public:
        explicit Board(const json& board);
        std::vector<Coord> getNeighbors(const Coord& pos) const;
        std::vector<std::vector<bool>> getObstacles() const;

    private:
        int height;
        int width;
        std::vector<Coord> food;
        std::vector<Coord> hazards;
        std::vector<Snake> snakes;
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
        std::string name;
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
        Snake you;
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
