#include "Evaluator.h"

#include <algorithm>
#include <limits>

namespace {

std::vector<int> distanceFrom(const Board& board, Stone color, int reach)
{
    const int size = board.size();
    std::vector<int> distance(static_cast<std::size_t>(board.area()), std::numeric_limits<int>::max());
    std::vector<Point> current;
    for (int row = 0; row < size; ++row) {
        for (int col = 0; col < size; ++col) {
            if (board.at({row, col}) == color) {
                distance[row * size + col] = 0;
                current.push_back({row, col});
            }
        }
    }
    for (int step = 1; step <= reach && !current.empty(); ++step) {
        std::vector<Point> next;
        for (const Point& point : current) {
            for (const Point& neighbor : board.neighbors(point)) {
                int& known = distance[neighbor.row * size + neighbor.col];
                if (board.at(neighbor) == Stone::Empty && known == std::numeric_limits<int>::max()) {
                    known = step;
                    next.push_back(neighbor);
                }
            }
        }
        current = std::move(next);
    }
    return distance;
}

}

std::vector<GroupSummary> summarizeGroups(const Board& board)
{
    const int size = board.size();
    std::vector<int> label(static_cast<std::size_t>(board.area()), -1);
    std::vector<int> libertyStamp(static_cast<std::size_t>(board.area()), -1);
    std::vector<GroupSummary> groups;
    std::vector<Point> frontier;

    for (int row = 0; row < size; ++row) {
        for (int col = 0; col < size; ++col) {
            const Stone color = board.at({row, col});
            if (color == Stone::Empty || label[row * size + col] != -1) {
                continue;
            }
            const int id = static_cast<int>(groups.size());
            GroupSummary group{color, 0, 0};
            label[row * size + col] = id;
            frontier.assign(1, Point{row, col});
            while (!frontier.empty()) {
                const Point current = frontier.back();
                frontier.pop_back();
                ++group.size;
                for (const Point& next : board.neighbors(current)) {
                    const int index = next.row * size + next.col;
                    const Stone stone = board.at(next);
                    if (stone == Stone::Empty) {
                        if (libertyStamp[index] != id) {
                            libertyStamp[index] = id;
                            ++group.liberties;
                        }
                    } else if (stone == color && label[index] == -1) {
                        label[index] = id;
                        frontier.push_back(next);
                    }
                }
            }
            groups.push_back(group);
        }
    }
    return groups;
}

std::vector<Stone> estimateOwnership(const Board& board, int reach)
{
    const std::vector<int> blackDistance = distanceFrom(board, Stone::Black, reach);
    const std::vector<int> whiteDistance = distanceFrom(board, Stone::White, reach);
    std::vector<Stone> owner(static_cast<std::size_t>(board.area()), Stone::Empty);
    for (std::size_t i = 0; i < owner.size(); ++i) {
        if (blackDistance[i] < whiteDistance[i]) {
            owner[i] = Stone::Black;
        } else if (whiteDistance[i] < blackDistance[i]) {
            owner[i] = Stone::White;
        }
    }
    return owner;
}

double estimateScoreMargin(const Board& board, Stone perspective, double komi)
{
    const std::vector<Stone> owner = estimateOwnership(board);
    const auto mine = std::count(owner.begin(), owner.end(), perspective);
    const auto theirs = std::count(owner.begin(), owner.end(), opponent(perspective));
    const double komiShare = perspective == Stone::White ? komi : -komi;
    return static_cast<double>(mine - theirs) + komiShare;
}

double evaluatePosition(const Board& board, Stone perspective, Stone toMove, int captureBalance, double komi)
{
    double value = estimateScoreMargin(board, perspective, komi);
    for (const GroupSummary& group : summarizeGroups(board)) {
        double groupValue = 0.0;
        if (group.liberties == 1) {
            groupValue -= group.color == toMove ? 0.6 * group.size : 1.8 * group.size + 1.0;
        } else if (group.liberties == 2) {
            groupValue -= 0.35 * group.size;
        }
        groupValue += 0.08 * std::min(group.liberties, 6);
        groupValue -= 0.2;
        value += group.color == perspective ? groupValue : -groupValue;
    }
    return value + 0.5 * captureBalance;
}

bool isOwnEye(const Board& board, Point point, Stone color)
{
    if (board.at(point) != Stone::Empty) {
        return false;
    }
    for (const Point& neighbor : board.neighbors(point)) {
        if (board.at(neighbor) != color) {
            return false;
        }
    }
    int diagonalsOnBoard = 0;
    int enemyDiagonals = 0;
    for (int dr : {-1, 1}) {
        for (int dc : {-1, 1}) {
            const Point diagonal{point.row + dr, point.col + dc};
            if (!board.contains(diagonal)) {
                continue;
            }
            ++diagonalsOnBoard;
            if (board.at(diagonal) == opponent(color)) {
                ++enemyDiagonals;
            }
        }
    }
    return diagonalsOnBoard == 4 ? enemyDiagonals <= 1 : enemyDiagonals == 0;
}
