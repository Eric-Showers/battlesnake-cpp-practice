//
// Created by Benny Gaechter on 24.04.2024.
//

#include "battlesnake.h"

#include <utility>

namespace battlesnake {
    BattleSnake::BattleSnake() {
        Info const i{};
        info = i;
    }

    std::string BattleSnake::GetInfo() const {
        return info.GetInfo();
    }

    std::string BattleSnake::End() {
        return "End";
    }

    std::string BattleSnake::Move(json state) {
        auto const gameState = GameState(std::move(state));

        //TODO: Decide how to move

        // Create response object
        json response{};
        response["move"] = "up";
        response["shout"] = "Just going up";

        return response.dump();
    }

    std::string BattleSnake::Start() {
        return "";
    }


    Coord::Coord(json coord): x(coord["x"]), y(coord["y"]) {
    }

    Board::Board(json board): height(board["height"]), width(board["width"]) {
        for (const auto &foodCoordinates: board["food"]) {
            food.emplace_back(foodCoordinates);
        }
        for (const auto &hazardCoordinates: board["hazards"]) {
            hazards.emplace_back(hazardCoordinates);
        }
        for (const auto &snake: board["snakes"]) {
            snakes.emplace_back(snake);
        }
    }

    Snake::Snake(json snake): head(snake["head"]), customizations(snake["customizations"]) {
        id = snake["id"];
        name = snake["name"];
        health = snake["health"];
        for (const auto &snakeBody: snake["body"]) {
            body.emplace_back(snakeBody);
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

    RulesetSettings::RulesetSettings(json rulesetSettings): foodspawnChance(rulesetSettings["foodSpawnChance"]),
                                                            minimumFood(rulesetSettings["minimumFood"]) {
        if(rulesetSettings.contains("hazardDamagePerTurn")) {
            hazardDamagePerTurn = rulesetSettings["hazardDamagePerTurn"];
        } else {
            hazardDamagePerTurn = 0;
        }
    }

    Ruleset::Ruleset(json ruleset): settings(ruleset["settings"]) {
        name = ruleset["name"];
        version = ruleset["version"];
    }

    Game::Game(json game): ruleset(game["ruleset"]) {
        id = game["id"];
        map = game["map"];
        source = game["source"];
        timeout = game["timeout"];
    }

    GameState::GameState(json state): game(state["game"]), board(state["board"]), you(Snake{state["you"]}) {
        turn = state["turn"];
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
