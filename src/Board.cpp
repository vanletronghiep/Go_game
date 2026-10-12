#include "Board.h"

#include <algorithm>

namespace {

std::uint64_t splitMix64(std::uint64_t value)
{
    value += 0x9E3779B97F4A7C15ULL;
    value = (value ^ (value >> 30)) * 0xBF58476D1CE4E5B9ULL;
    value = (value ^ (value >> 27)) * 0x94D049BB133111EBULL;
    return value ^ (value >> 31);
}

std::uint64_t zobristKey(Point point, Stone stone)
{
    if (stone == Stone::Empty) {
        return 0;
    }
    const auto cell = static_cast<std::uint64_t>(point.row * Board::maxSize + point.col);
    const auto color = stone == Stone::Black ? 1ULL : 2ULL;
    return splitMix64(cell * 4 + color);
}

}

bool isValidBoardSize(int size)
{
    return size == 9 || size == 13 || size == 19;
}

Board::Board(int size)
    : size_(std::clamp(size, 2, maxSize))
    , cells_(static_cast<std::size_t>(size_ * size_), Stone::Empty)
{
}

bool Board::contains(Point point) const
{
    return point.row >= 0 && point.row < size_ && point.col >= 0 && point.col < size_;
}

Stone Board::at(Point point) const
{
    return contains(point) ? cells_[index(point)] : Stone::Empty;
}

void Board::set(Point point, Stone stone)
{
    if (!contains(point)) {
        return;
    }
    Stone& cell = cells_[index(point)];
    hash_ ^= zobristKey(point, cell);
    cell = stone;
    hash_ ^= zobristKey(point, cell);
}

int Board::stoneCount(Stone stone) const
{
    return static_cast<int>(std::count(cells_.begin(), cells_.end(), stone));
}

bool Board::isEmpty() const
{
    return std::all_of(cells_.begin(), cells_.end(), [](Stone stone) { return stone == Stone::Empty; });
}

Neighbors Board::neighbors(Point point) const
{
    Neighbors result;
    const std::array<Point, 4> candidates{{
        {point.row - 1, point.col},
        {point.row + 1, point.col},
        {point.row, point.col - 1},
        {point.row, point.col + 1},
    }};
    for (const Point& candidate : candidates) {
        if (contains(candidate)) {
            result.points[result.count++] = candidate;
        }
    }
    return result;
}

Group Board::groupAt(Point point) const
{
    Group group;
    group.color = at(point);
    if (group.color == Stone::Empty) {
        return group;
    }

    std::vector<char> visitedStone(cells_.size(), 0);
    std::vector<char> countedLiberty(cells_.size(), 0);
    std::vector<Point> frontier{point};
    visitedStone[index(point)] = 1;

    while (!frontier.empty()) {
        const Point current = frontier.back();
        frontier.pop_back();
        group.stones.push_back(current);
        for (const Point& next : neighbors(current)) {
            const Stone stone = at(next);
            if (stone == Stone::Empty) {
                if (!countedLiberty[index(next)]) {
                    countedLiberty[index(next)] = 1;
                    group.liberties.push_back(next);
                }
            } else if (stone == group.color && !visitedStone[index(next)]) {
                visitedStone[index(next)] = 1;
                frontier.push_back(next);
            }
        }
    }
    return group;
}

int Board::libertyCount(Point point) const
{
    return static_cast<int>(groupAt(point).liberties.size());
}

PlaceResult Board::place(Point point, Stone stone)
{
    if (!contains(point)) {
        return {PlaceStatus::OffBoard, {}};
    }
    if (at(point) != Stone::Empty || stone == Stone::Empty) {
        return {PlaceStatus::Occupied, {}};
    }

    set(point, stone);
    PlaceResult result;
    const Stone enemy = opponent(stone);
    for (const Point& next : neighbors(point)) {
        if (at(next) != enemy) {
            continue;
        }
        Group enemyGroup = groupAt(next);
        if (!enemyGroup.liberties.empty()) {
            continue;
        }
        for (const Point& captured : enemyGroup.stones) {
            set(captured, Stone::Empty);
            result.captured.push_back(captured);
        }
    }

    if (result.captured.empty() && libertyCount(point) == 0) {
        set(point, Stone::Empty);
        return {PlaceStatus::Suicide, {}};
    }
    return result;
}

bool operator==(const Board& left, const Board& right)
{
    return left.size_ == right.size_ && left.cells_ == right.cells_;
}
