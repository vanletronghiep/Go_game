#pragma once

#include <cstdint>
#include <optional>
#include <string>

enum class Stone : std::uint8_t { Empty, Black, White };

Stone opponent(Stone stone);
std::string stoneName(Stone stone);

struct Point {
    int row = 0;
    int col = 0;

    friend bool operator==(const Point&, const Point&) = default;
};

enum class MoveType { Place, Pass };

struct Move {
    MoveType type = MoveType::Pass;
    Point point;

    static Move place(Point point);
    static Move pass();
    bool isPass() const;

    friend bool operator==(const Move&, const Move&) = default;
};

std::string toGoCoordinate(Point point, int boardSize);
std::optional<Point> parseGoCoordinate(const std::string& text, int boardSize);
std::string moveToText(const Move& move, int boardSize);
std::optional<Move> parseMoveText(const std::string& text, int boardSize);
