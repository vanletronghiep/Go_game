#include "Game.h"

std::string difficultyName(Difficulty difficulty)
{
    switch (difficulty) {
    case Difficulty::Easy:
        return "Easy";
    case Difficulty::Medium:
        return "Medium";
    case Difficulty::Hard:
        return "Hard";
    }
    return "Unknown";
}

Game::Game(GameConfig config)
    : config_(config)
    , current_(freshState())
{
}

Game::State Game::freshState() const
{
    State state{Position(config_.boardSize), {}, Phase::Playing, {}};
    state.deadStones.assign(static_cast<std::size_t>(state.position.board().area()), false);
    return state;
}

Stone Game::aiColor() const
{
    return isAIGame() ? opponent(config_.humanColor) : Stone::Empty;
}

bool Game::isAITurn() const
{
    return isAIGame() && phase() == Phase::Playing && toMove() == aiColor();
}

std::string Game::playerLabel(Stone stone) const
{
    const std::string color = stoneName(stone);
    if (!isAIGame()) {
        return (stone == Stone::White ? "Player 1 (" : "Player 2 (") + color + ")";
    }
    if (stone == config_.humanColor) {
        return "You (" + color + ")";
    }
    return "AI " + difficultyName(config_.difficulty) + " (" + color + ")";
}

MoveReport Game::play(const Move& move)
{
    MoveReport report;
    report.player = toMove();
    report.move = move;
    if (phase() != Phase::Playing) {
        report.error = MoveError::GameNotInProgress;
        return report;
    }

    State next = current_;
    MoveOutcome outcome = next.position.play(move);
    if (!outcome.ok()) {
        report.error = outcome.error;
        return report;
    }

    next.moves.push_back(move);
    if (next.position.consecutivePasses() >= 2) {
        next.phase = Phase::Scoring;
        next.deadStones.assign(next.deadStones.size(), false);
        report.enteredScoring = true;
    }
    undoStack_.push_back(std::move(current_));
    current_ = std::move(next);
    redoStack_.clear();
    report.captured = std::move(outcome.captured);
    return report;
}

void Game::reset()
{
    current_ = freshState();
    undoStack_.clear();
    redoStack_.clear();
}

bool Game::canUndo() const
{
    return !undoStack_.empty() && phase() != Phase::Finished;
}

bool Game::canRedo() const
{
    return !redoStack_.empty() && phase() != Phase::Finished;
}

bool Game::undo()
{
    if (!canUndo()) {
        return false;
    }
    redoStack_.push_back(std::move(current_));
    current_ = std::move(undoStack_.back());
    undoStack_.pop_back();
    return true;
}

bool Game::redo()
{
    if (!canRedo()) {
        return false;
    }
    undoStack_.push_back(std::move(current_));
    current_ = std::move(redoStack_.back());
    redoStack_.pop_back();
    return true;
}

int Game::undoTurn()
{
    if (!isAIGame()) {
        return undo() ? 1 : 0;
    }
    int steps = 0;
    while (undo()) {
        ++steps;
        if (toMove() == config_.humanColor) {
            break;
        }
    }
    if (steps > 0 && toMove() != config_.humanColor) {
        for (int i = 0; i < steps; ++i) {
            redo();
        }
        return 0;
    }
    return steps;
}

int Game::redoTurn()
{
    if (!isAIGame()) {
        return redo() ? 1 : 0;
    }
    int steps = 0;
    while (redo()) {
        ++steps;
        if (toMove() == config_.humanColor || phase() != Phase::Playing) {
            break;
        }
    }
    return steps;
}

std::vector<Move> Game::redoMoves() const
{
    std::vector<Move> result;
    for (auto state = redoStack_.rbegin(); state != redoStack_.rend(); ++state) {
        if (!state->moves.empty()) {
            result.push_back(state->moves.back());
        }
    }
    return result;
}

bool Game::toggleDeadGroup(Point point)
{
    if (phase() != Phase::Scoring || board().at(point) == Stone::Empty) {
        return false;
    }
    const Group group = board().groupAt(point);
    const bool markDead = !current_.deadStones[deadIndex(point)];
    for (const Point& stone : group.stones) {
        current_.deadStones[deadIndex(stone)] = markDead;
    }
    return true;
}

bool Game::isDead(Point point) const
{
    return board().contains(point) && current_.deadStones[deadIndex(point)];
}

ScoreBreakdown Game::score() const
{
    return scoreByArea(board(), current_.deadStones, config_.komi);
}

bool Game::resumePlay()
{
    if (phase() != Phase::Scoring) {
        return false;
    }
    current_.phase = Phase::Playing;
    current_.position.clearPasses();
    current_.deadStones.assign(current_.deadStones.size(), false);
    redoStack_.clear();
    return true;
}

bool Game::acceptScore()
{
    if (phase() != Phase::Scoring) {
        return false;
    }
    current_.phase = Phase::Finished;
    redoStack_.clear();
    return true;
}
