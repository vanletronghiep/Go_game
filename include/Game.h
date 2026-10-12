#pragma once

#include "Position.h"
#include "Scoring.h"

#include <string>
#include <vector>

enum class GameMode { PlayerVsPlayer, PlayerVsAI };
enum class Difficulty { Easy, Medium, Hard };
enum class Phase { Playing, Scoring, Finished };

std::string difficultyName(Difficulty difficulty);

struct GameConfig {
    int boardSize = Board::maxSize;
    GameMode mode = GameMode::PlayerVsPlayer;
    Difficulty difficulty = Difficulty::Medium;
    Stone humanColor = Stone::Black;
    double komi = 7.5;
};

struct MoveReport {
    MoveError error = MoveError::None;
    Stone player = Stone::Empty;
    Move move;
    std::vector<Point> captured;
    bool enteredScoring = false;

    bool ok() const { return error == MoveError::None; }
};

class Game {
public:
    explicit Game(GameConfig config = {});

    const GameConfig& config() const { return config_; }
    const Position& position() const { return current_.position; }
    const Board& board() const { return current_.position.board(); }
    Stone toMove() const { return current_.position.toMove(); }
    Phase phase() const { return current_.phase; }
    const std::vector<Move>& moves() const { return current_.moves; }

    bool isAIGame() const { return config_.mode == GameMode::PlayerVsAI; }
    Stone aiColor() const;
    bool isAITurn() const;
    std::string playerLabel(Stone stone) const;

    MoveReport play(const Move& move);
    void reset();

    bool canUndo() const;
    bool canRedo() const;
    bool undo();
    bool redo();
    int undoTurn();
    int redoTurn();
    std::vector<Move> redoMoves() const;

    bool toggleDeadGroup(Point point);
    bool isDead(Point point) const;
    const std::vector<bool>& deadStones() const { return current_.deadStones; }
    ScoreBreakdown score() const;
    bool resumePlay();
    bool acceptScore();

private:
    struct State {
        Position position;
        std::vector<Move> moves;
        Phase phase = Phase::Playing;
        std::vector<bool> deadStones;
    };

    State freshState() const;
    int deadIndex(Point point) const { return point.row * board().size() + point.col; }

    GameConfig config_;
    State current_;
    std::vector<State> undoStack_;
    std::vector<State> redoStack_;
};
