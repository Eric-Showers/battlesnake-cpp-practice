#include "battlesnake.h"
#include "httplib.h"
#include "json.h"
#include <iostream>

using json = nlohmann::json;

int main() {
    battlesnake::BattleSnake bs{};
    httplib::Server server;

    std::string const SERVER_ID = "bgaechter/battlesnake-starter-cpp";

    server.set_post_routing_handler([&SERVER_ID](const auto &req, auto &res) {
        res.set_header("Server", SERVER_ID);
    });

    server.Get("/", [&bs](const httplib::Request &req, httplib::Response &res) {
        res.set_content(bs.GetInfo(), "application/json");
    });

    server.Post("/start", [&bs](const httplib::Request &req, httplib::Response &res) {
        auto state = json::parse(req.body);
        res.set_content(bs.Start(), "text/plain");
    });

    server.Post("/move", [&bs](const httplib::Request &req, httplib::Response &res) {
        auto const state = json::parse(req.body);
        const auto response = bs.Move(state);
        res.set_content(response, "application/json");
    });

    server.Post("/end", [&bs](const httplib::Request &req, httplib::Response &res) {
        res.set_content(bs.End(), "text/plain");
    });
  std::cout << "Server listening at http://127.0.0.1:8080" << std::endl;
    server.listen("0.0.0.0", 8080);
}
