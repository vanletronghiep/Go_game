#pragma once

#include "Game.h"

#include <atomic>
#include <cstdint>
#include <future>
#include <memory>
#include <optional>

struct AIDecision {
    Move move;
    int searchDepth = 0;
    long long nodesVisited = 0;
};

AIDecision chooseAIMove(const Position& position, Difficulty difficulty, double komi, std::uint64_t seed,
    const std::atomic<bool>& stopRequested);

class AIPlayer {
public:
    AIPlayer() = default;
    AIPlayer(const AIPlayer&) = delete;
    AIPlayer& operator=(const AIPlayer&) = delete;
    ~AIPlayer();

    void start(const Position& position, Difficulty difficulty, double komi);
    bool isThinking() const { return pending_.valid(); }
    std::optional<AIDecision> takeDecision();
    void cancel();

private:
    std::future<AIDecision> pending_;
    std::shared_ptr<std::atomic<bool>> stopFlag_;
};
