#include "battlesnake.h"
#include "httplib.h"
#include "json.h"
#include <iostream>
#include <sstream>

using json = nlohmann::json;

int main() {
    battlesnake::BattleSnake bs{};
    httplib::Server server;

    srand(time(NULL));

    std::string const SERVER_ID = "bgaechter/battlesnake-starter-cpp";

    server.set_post_routing_handler([&SERVER_ID](const auto &req [[maybe_unused]], auto &res) {
        res.set_header("Server", SERVER_ID);
    });

    server.set_logger([](const auto &req, const auto &res) {
        std::cout << req.method << " - " << req.path << ": " << res.body << std::endl;
    });

    server.set_exception_handler([](const auto &req [[maybe_unused]], auto &res, std::exception_ptr ep) {
        std::ostringstream oss;
        try {
            std::rethrow_exception(ep);
        } catch (const std::exception &e) {
            oss << R"({ "move":"up", "error":")" << e.what() << "\" }";
        } catch (...) {
            oss << R"({ "move":"up", "error":"Unknown Exception" })";
        }
        std::string result = oss.str();
        std::cout << "[ERROR] " << result << std::endl;

        res.set_content(result, "application/json");
        res.status = httplib::StatusCode::OK_200; // Not returning 500 to keep snake alive
      });

    server.Get("/", [&bs](const httplib::Request &req [[maybe_unused]], httplib::Response &res) {
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

    server.Post("/end", [&bs](const httplib::Request &req [[maybe_unused]], httplib::Response &res) {
        res.set_content(bs.End(), "text/plain");
    });
  std::cout << "Server listening at http://127.0.0.1:8080" << std::endl;
    server.listen("0.0.0.0", 8080);
}
