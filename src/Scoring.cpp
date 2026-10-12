#include "Scoring.h"

namespace {

Board removeDeadStones(const Board& board, const std::vector<bool>& deadStones, ScoreBreakdown& score)
{
    Board alive = board;
    for (int row = 0; row < board.size(); ++row) {
        for (int col = 0; col < board.size(); ++col) {
            const Point point{row, col};
            const auto index = static_cast<std::size_t>(row * board.size() + col);
            const Stone stone = board.at(point);
            if (stone == Stone::Empty || index >= deadStones.size() || !deadStones[index]) {
                continue;
            }
            if (stone == Stone::Black) {
                ++score.deadBlackStones;
            } else {
                ++score.deadWhiteStones;
            }
            alive.set(point, Stone::Empty);
        }
    }
    return alive;
}

void fillEmptyRegion(const Board& board, Point start, std::vector<char>& visited, ScoreBreakdown& score)
{
    const int size = board.size();
    std::vector<Point> region;
    std::vector<Point> frontier{start};
    visited[start.row * size + start.col] = 1;
    bool touchesBlack = false;
    bool touchesWhite = false;

    while (!frontier.empty()) {
        const Point current = frontier.back();
        frontier.pop_back();
        region.push_back(current);
        for (const Point& next : board.neighbors(current)) {
            const Stone stone = board.at(next);
            if (stone == Stone::Black) {
                touchesBlack = true;
            } else if (stone == Stone::White) {
                touchesWhite = true;
            } else if (!visited[next.row * size + next.col]) {
                visited[next.row * size + next.col] = 1;
                frontier.push_back(next);
            }
        }
    }

    Stone owner = Stone::Empty;
    if (touchesBlack && !touchesWhite) {
        owner = Stone::Black;
        score.blackTerritory += static_cast<int>(region.size());
    } else if (touchesWhite && !touchesBlack) {
        owner = Stone::White;
        score.whiteTerritory += static_cast<int>(region.size());
    } else {
        score.neutralPoints += static_cast<int>(region.size());
    }
    for (const Point& point : region) {
        score.areaOwner[point.row * size + point.col] = owner;
    }
}

}

ScoreBreakdown scoreByArea(const Board& board, const std::vector<bool>& deadStones, double komi)
{
    ScoreBreakdown score;
    score.komi = komi;
    score.areaOwner.assign(static_cast<std::size_t>(board.area()), Stone::Empty);

    const Board alive = removeDeadStones(board, deadStones, score);
    std::vector<char> visited(static_cast<std::size_t>(board.area()), 0);

    for (int row = 0; row < board.size(); ++row) {
        for (int col = 0; col < board.size(); ++col) {
            const Point point{row, col};
            const Stone stone = alive.at(point);
            if (stone == Stone::Black) {
                ++score.blackStones;
                score.areaOwner[row * board.size() + col] = Stone::Black;
            } else if (stone == Stone::White) {
                ++score.whiteStones;
                score.areaOwner[row * board.size() + col] = Stone::White;
            } else if (!visited[row * board.size() + col]) {
                fillEmptyRegion(alive, point, visited, score);
            }
        }
    }

    score.blackTotal = score.blackStones + score.blackTerritory;
    score.whiteTotal = score.whiteStones + score.whiteTerritory + komi;
    if (score.blackTotal > score.whiteTotal) {
        score.winner = Stone::Black;
    } else if (score.whiteTotal > score.blackTotal) {
        score.winner = Stone::White;
    }
    return score;
}
