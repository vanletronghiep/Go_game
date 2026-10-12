#include "Types.h"

#include <cctype>
#include <string_view>

namespace {

constexpr std::string_view columnLetters = "ABCDEFGHJKLMNOPQRST";

}

Stone opponent(Stone stone)
{
    switch (stone) {
    case Stone::Black:
        return Stone::White;
    case Stone::White:
        return Stone::Black;
    default:
        return Stone::Empty;
    }
}

std::string stoneName(Stone stone)
{
    switch (stone) {
    case Stone::Black:
        return "Black";
    case Stone::White:
        return "White";
    default:
        return "Empty";
    }
}

Move Move::place(Point point)
{
    return Move{MoveType::Place, point};
}

Move Move::pass()
{
    return Move{MoveType::Pass, Point{}};
}

bool Move::isPass() const
{
    return type == MoveType::Pass;
}

std::string toGoCoordinate(Point point, int boardSize)
{
    if (point.col < 0 || point.col >= static_cast<int>(columnLetters.size()) || point.row < 0
        || point.row >= boardSize) {
        return "??";
    }
    return std::string(1, columnLetters[point.col]) + std::to_string(boardSize - point.row);
}

std::optional<Point> parseGoCoordinate(const std::string& text, int boardSize)
{
    if (text.size() < 2 || text.size() > 3) {
        return std::nullopt;
    }
    const char letter = static_cast<char>(std::toupper(static_cast<unsigned char>(text[0])));
    const auto column = columnLetters.find(letter);
    if (column == std::string_view::npos || static_cast<int>(column) >= boardSize) {
        return std::nullopt;
    }
    int number = 0;
    for (std::size_t i = 1; i < text.size(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(text[i]))) {
            return std::nullopt;
        }
        number = number * 10 + (text[i] - '0');
    }
    if (number < 1 || number > boardSize) {
        return std::nullopt;
    }
    return Point{boardSize - number, static_cast<int>(column)};
}

std::string moveToText(const Move& move, int boardSize)
{
    return move.isPass() ? "pass" : toGoCoordinate(move.point, boardSize);
}

std::optional<Move> parseMoveText(const std::string& text, int boardSize)
{
    if (text == "pass") {
        return Move::pass();
    }
    const auto point = parseGoCoordinate(text, boardSize);
    if (!point) {
        return std::nullopt;
    }
    return Move::place(*point);
}
