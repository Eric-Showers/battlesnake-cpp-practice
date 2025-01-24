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
        explicit Snake(json snake);
        std::vector<Coord> getNextTurnBody();
        std::string getDirectionStr(Coord destination) const;
        std::string getMove(Board board) const;
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
        explicit Board(json board);
        Snake getSnake(std::string snake_id);
        std::vector<Coord> getNeighbors(Coord pos);
        std::vector<std::vector<bool>> getObstacles();

    private:
        int height;
        int width;
        std::vector<Coord> food;
        std::vector<Coord> hazards;
        std::vector<Snake> snakes;
    };

    class RulesetSettings {
    public:
        explicit RulesetSettings(json rulesetSettings);

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
        explicit Game(json game);
        std::string id;

    private:
        Ruleset ruleset;
        std::string map;
        std::string source;
        int timeout;
    };

    class GameState {
    public:
        explicit GameState(json state);
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

        std::string GetInfo() const;

        std::string Start();

        std::string Move(json state);

        std::string End();

    private:
        Info info;
    };
} // battlesnake

#endif //BATTLESNAKE_H
