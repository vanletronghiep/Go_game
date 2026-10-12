#include "Position.h"

#include <algorithm>

std::string describeMoveError(MoveError error)
{
    switch (error) {
    case MoveError::None:
        return "Move accepted";
    case MoveError::GameNotInProgress:
        return "The game is not in progress";
    case MoveError::OffBoard:
        return "That point is outside the board";
    case MoveError::Occupied:
        return "That intersection is already occupied";
    case MoveError::Suicide:
        return "Suicide is not allowed: the stone would have no liberties";
    case MoveError::Superko:
        return "Ko rule: this move would repeat an earlier board position";
    }
    return "Unknown error";
}

Position::Position(int boardSize)
    : board_(boardSize)
    , positionHistory_{board_.hash()}
{
}

int Position::capturesBy(Stone stone) const
{
    if (stone == Stone::Black) {
        return captures_[0];
    }
    if (stone == Stone::White) {
        return captures_[1];
    }
    return 0;
}

MoveError Position::checkMove(const Move& move) const
{
    if (move.isPass()) {
        return MoveError::None;
    }
    Board scratch = board_;
    std::vector<Point> captured;
    return tryPlace(move.point, scratch, captured);
}

MoveOutcome Position::play(const Move& move)
{
    MoveOutcome outcome;
    if (move.isPass()) {
        ++consecutivePasses_;
    } else {
        Board next = board_;
        outcome.error = tryPlace(move.point, next, outcome.captured);
        if (!outcome.ok()) {
            outcome.captured.clear();
            return outcome;
        }
        board_ = std::move(next);
        captures_[toMove_ == Stone::Black ? 0 : 1] += static_cast<int>(outcome.captured.size());
        positionHistory_.push_back(board_.hash());
        consecutivePasses_ = 0;
    }
    lastMove_ = move;
    toMove_ = opponent(toMove_);
    return outcome;
}

MoveError Position::tryPlace(Point point, Board& result, std::vector<Point>& captured) const
{
    PlaceResult placement = result.place(point, toMove_);
    switch (placement.status) {
    case PlaceStatus::OffBoard:
        return MoveError::OffBoard;
    case PlaceStatus::Occupied:
        return MoveError::Occupied;
    case PlaceStatus::Suicide:
        return MoveError::Suicide;
    case PlaceStatus::Ok:
        break;
    }
    if (repeatsEarlierPosition(result.hash())) {
        return MoveError::Superko;
    }
    captured = std::move(placement.captured);
    return MoveError::None;
}

bool Position::repeatsEarlierPosition(std::uint64_t hash) const
{
    return std::find(positionHistory_.begin(), positionHistory_.end(), hash) != positionHistory_.end();
}
