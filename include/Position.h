#pragma once

#include "Board.h"

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

enum class MoveError { None, GameNotInProgress, OffBoard, Occupied, Suicide, Superko };

std::string describeMoveError(MoveError error);

struct MoveOutcome {
    MoveError error = MoveError::None;
    std::vector<Point> captured;

    bool ok() const { return error == MoveError::None; }
};

class Position {
public:
    explicit Position(int boardSize = Board::maxSize);

    const Board& board() const { return board_; }
    Stone toMove() const { return toMove_; }
    int capturesBy(Stone stone) const;
    int consecutivePasses() const { return consecutivePasses_; }
    const std::optional<Move>& lastMove() const { return lastMove_; }
    const std::vector<std::uint64_t>& positionHistory() const { return positionHistory_; }

    MoveError checkMove(const Move& move) const;
    bool isLegal(const Move& move) const { return checkMove(move) == MoveError::None; }
    MoveOutcome play(const Move& move);
    void clearPasses() { consecutivePasses_ = 0; }

private:
    MoveError tryPlace(Point point, Board& result, std::vector<Point>& captured) const;
    bool repeatsEarlierPosition(std::uint64_t hash) const;

    Board board_;
    Stone toMove_ = Stone::Black;
    std::array<int, 2> captures_{0, 0};
    int consecutivePasses_ = 0;
    std::optional<Move> lastMove_;
    std::vector<std::uint64_t> positionHistory_;
};
