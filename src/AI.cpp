#include "AI.h"

#include "Evaluator.h"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <random>
#include <unordered_set>
#include <vector>

namespace {

using Clock = std::chrono::steady_clock;

constexpr double infinity = 1e18;

struct SearchLimits {
    int rootWidth = 8;
    int innerWidth = 6;
    int maxDepth = 2;
    std::chrono::milliseconds budget{1500};
    bool useAlphaBeta = false;
    double noise = 0.0;
};

SearchLimits limitsFor(Difficulty difficulty, int boardSize)
{
    if (difficulty == Difficulty::Hard) {
        const bool largeBoard = boardSize >= 19;
        return {largeBoard ? 12 : 14, 7, 4, std::chrono::milliseconds(largeBoard ? 3000 : 2500), true, 0.0};
    }
    return {8, 6, 2, std::chrono::milliseconds(1500), false, 1.2};
}

struct Node {
    Board board;
    Stone toMove = Stone::Black;
    int blackCaptures = 0;
    int whiteCaptures = 0;
    std::optional<Point> lastPoint;
};

struct Candidate {
    Point point;
    double priority = 0.0;
    Node child;
};

int edgeDistance(Point point, int size)
{
    return std::min({point.row, point.col, size - 1 - point.row, size - 1 - point.col});
}

std::vector<Point> openingPoints(int size)
{
    const int near = size >= 13 ? 3 : 2;
    const int far = size - 1 - near;
    std::vector<Point> points{{near, near}, {near, far}, {far, near}, {far, far}};
    if (size >= 19) {
        points.insert(points.end(), {{2, 3}, {3, 2}, {2, far}, {far, 2}, {far, size - 3}, {size - 3, far}});
    } else {
        points.push_back({size / 2, size / 2});
    }
    return points;
}

class Searcher {
public:
    Searcher(const Position& position, Difficulty difficulty, double komi, std::uint64_t seed,
        const std::atomic<bool>& stopRequested);

    AIDecision decide();

private:
    AIDecision decideEasy();
    AIDecision decideBySearch();
    bool shouldPass(const std::vector<Candidate>& rootCandidates) const;
    bool opponentJustPassed() const;
    bool gameIsTooLong() const;

    std::vector<Point> candidatePoints(const Node& node) const;
    bool isSettled(const Board& board, const std::vector<Stone>& owner, Point point) const;
    bool tryMove(const Node& node, Point point, Node& child, int& captured) const;
    std::vector<Candidate> expand(const Node& node, int width, bool atRoot);
    double movePriority(const Node& node, Point point, const Node& child, int captured);
    double evaluate(const Node& node) const;
    double minimax(const Node& node, int depth, double alpha, double beta);
    bool outOfTime();

    Node root_;
    Stone me_;
    Difficulty difficulty_;
    double komi_;
    std::mt19937_64 rng_;
    const std::atomic<bool>& stopRequested_;
    std::unordered_set<std::uint64_t> history_;
    std::size_t historyLength_;
    std::optional<Move> lastMove_;
    int consecutivePasses_;
    SearchLimits limits_;
    std::vector<std::uint64_t> path_;
    Clock::time_point deadline_;
    bool aborted_ = false;
    long long nodes_ = 0;
};

Searcher::Searcher(const Position& position, Difficulty difficulty, double komi, std::uint64_t seed,
    const std::atomic<bool>& stopRequested)
    : root_{position.board(), position.toMove(), position.capturesBy(Stone::Black),
          position.capturesBy(Stone::White), std::nullopt}
    , me_(position.toMove())
    , difficulty_(difficulty)
    , komi_(komi)
    , rng_(seed)
    , stopRequested_(stopRequested)
    , history_(position.positionHistory().begin(), position.positionHistory().end())
    , historyLength_(position.positionHistory().size())
    , lastMove_(position.lastMove())
    , consecutivePasses_(position.consecutivePasses())
    , limits_(limitsFor(difficulty, position.board().size()))
{
    if (lastMove_ && !lastMove_->isPass()) {
        root_.lastPoint = lastMove_->point;
    }
}

AIDecision Searcher::decide()
{
    if (gameIsTooLong()) {
        return {Move::pass(), 0, 0};
    }
    return difficulty_ == Difficulty::Easy ? decideEasy() : decideBySearch();
}

bool Searcher::opponentJustPassed() const
{
    return lastMove_ && lastMove_->isPass() && consecutivePasses_ > 0;
}

bool Searcher::gameIsTooLong() const
{
    return historyLength_ > static_cast<std::size_t>(root_.board.area()) * 2;
}

AIDecision Searcher::decideEasy()
{
    const Board& board = root_.board;
    const int stones = board.area() - board.stoneCount(Stone::Empty);
    if (opponentJustPassed() && (stones * 5 >= board.area() * 2 || estimateScoreMargin(board, me_, komi_) > 0)) {
        return {Move::pass(), 0, 0};
    }

    std::vector<Point> points = candidatePoints(root_);
    std::uniform_int_distribution<int> percent(0, 99);
    if (points.empty() || percent(rng_) < 30) {
        points.clear();
        for (int row = 0; row < board.size(); ++row) {
            for (int col = 0; col < board.size(); ++col) {
                if (!isOwnEye(board, {row, col}, me_)) {
                    points.push_back({row, col});
                }
            }
        }
    }
    std::shuffle(points.begin(), points.end(), rng_);
    for (const Point& point : points) {
        Node child;
        int captured = 0;
        ++nodes_;
        if (tryMove(root_, point, child, captured)) {
            return {Move::place(point), 0, nodes_};
        }
    }
    return {Move::pass(), 0, nodes_};
}

AIDecision Searcher::decideBySearch()
{
    std::vector<Candidate> rootCandidates = expand(root_, limits_.rootWidth, true);
    if (rootCandidates.empty() || shouldPass(rootCandidates)) {
        return {Move::pass(), 0, nodes_};
    }

    deadline_ = Clock::now() + limits_.budget;
    std::uniform_real_distribution<double> noise(-limits_.noise, limits_.noise);
    Point bestPoint = rootCandidates.front().point;
    int completedDepth = 0;
    const int firstDepth = limits_.useAlphaBeta ? 1 : limits_.maxDepth;

    for (int depth = firstDepth; depth <= limits_.maxDepth; ++depth) {
        double alpha = -infinity;
        double bestValue = -infinity;
        std::size_t bestIndex = 0;
        for (std::size_t i = 0; i < rootCandidates.size(); ++i) {
            const Candidate& candidate = rootCandidates[i];
            path_.assign(1, candidate.child.board.hash());
            double value = minimax(candidate.child, depth - 1, alpha, infinity);
            if (aborted_) {
                break;
            }
            if (limits_.noise > 0.0) {
                value += noise(rng_);
            }
            if (value > bestValue) {
                bestValue = value;
                bestIndex = i;
            }
            if (limits_.useAlphaBeta) {
                alpha = std::max(alpha, bestValue);
            }
        }
        const bool foundAny = bestValue > -infinity;
        if (aborted_ && completedDepth > 0) {
            break;
        }
        if (foundAny) {
            bestPoint = rootCandidates[bestIndex].point;
            std::rotate(rootCandidates.begin(), rootCandidates.begin() + static_cast<long>(bestIndex),
                rootCandidates.begin() + static_cast<long>(bestIndex) + 1);
        }
        if (aborted_) {
            break;
        }
        completedDepth = depth;
    }
    return {Move::place(bestPoint), completedDepth, nodes_};
}

bool Searcher::shouldPass(const std::vector<Candidate>& rootCandidates) const
{
    const Board& board = root_.board;
    if (opponentJustPassed() && estimateScoreMargin(board, me_, komi_) > 0) {
        return true;
    }
    const int stones = board.area() - board.stoneCount(Stone::Empty);
    if (stones * 2 < board.area()) {
        return false;
    }
    Node passed = root_;
    passed.toMove = opponent(me_);
    const double passValue = evaluate(passed);
    double bestMoveValue = -infinity;
    for (const Candidate& candidate : rootCandidates) {
        bestMoveValue = std::max(bestMoveValue, evaluate(candidate.child));
    }
    return bestMoveValue <= passValue + 0.3;
}

bool Searcher::isSettled(const Board& board, const std::vector<Stone>& owner, Point point) const
{
    const int size = board.size();
    const Stone holder = owner[point.row * size + point.col];
    if (holder == Stone::Empty) {
        return false;
    }
    for (const Point& neighbor : board.neighbors(point)) {
        const Stone stone = board.at(neighbor);
        if (stone == opponent(holder)) {
            return false;
        }
        if (stone == Stone::Empty && owner[neighbor.row * size + neighbor.col] != holder) {
            return false;
        }
    }
    for (int dr = -2; dr <= 2; ++dr) {
        for (int dc = -2; dc <= 2; ++dc) {
            if (std::abs(dr) + std::abs(dc) > 2) {
                continue;
            }
            if (board.at({point.row + dr, point.col + dc}) == opponent(holder)) {
                return false;
            }
        }
    }
    return true;
}

std::vector<Point> Searcher::candidatePoints(const Node& node) const
{
    const Board& board = node.board;
    const int size = board.size();
    const int stones = board.area() - board.stoneCount(Stone::Empty);
    std::vector<char> marked(static_cast<std::size_t>(board.area()), 0);
    std::vector<Point> points;

    auto mark = [&](Point point) {
        if (board.contains(point) && board.at(point) == Stone::Empty && !marked[point.row * size + point.col]) {
            marked[point.row * size + point.col] = 1;
            points.push_back(point);
        }
    };

    if (stones * 10 < board.area()) {
        for (const Point& point : openingPoints(size)) {
            mark(point);
        }
    }
    for (int row = 0; row < size; ++row) {
        for (int col = 0; col < size; ++col) {
            if (board.at({row, col}) == Stone::Empty) {
                continue;
            }
            for (int dr = -2; dr <= 2; ++dr) {
                for (int dc = -2; dc <= 2; ++dc) {
                    if (std::abs(dr) + std::abs(dc) <= 3) {
                        mark({row + dr, col + dc});
                    }
                }
            }
        }
    }

    const bool filterSettled = stones * 4 >= board.area() || opponentJustPassed();
    const std::vector<Stone> owner = filterSettled ? estimateOwnership(board) : std::vector<Stone>{};
    std::erase_if(points, [&](const Point& point) {
        return isOwnEye(board, point, node.toMove) || (filterSettled && isSettled(board, owner, point));
    });
    return points;
}

bool Searcher::tryMove(const Node& node, Point point, Node& child, int& captured) const
{
    child = node;
    const PlaceResult result = child.board.place(point, node.toMove);
    if (result.status != PlaceStatus::Ok) {
        return false;
    }
    const std::uint64_t hash = child.board.hash();
    if (history_.contains(hash) || std::find(path_.begin(), path_.end(), hash) != path_.end()) {
        return false;
    }
    captured = static_cast<int>(result.captured.size());
    if (node.toMove == Stone::Black) {
        child.blackCaptures += captured;
    } else {
        child.whiteCaptures += captured;
    }
    child.toMove = opponent(node.toMove);
    child.lastPoint = point;
    return true;
}

double Searcher::movePriority(const Node& node, Point point, const Node& child, int captured)
{
    const Stone mover = node.toMove;
    const Stone enemy = opponent(mover);
    double priority = captured > 0 ? 3.0 + 4.0 * captured : 0.0;

    const int liberties = child.board.libertyCount(point);
    priority += liberties == 1 ? -6.0 : 0.4 * std::min(liberties, 4);

    for (const Point& neighbor : node.board.neighbors(point)) {
        const Stone stone = node.board.at(neighbor);
        if (stone == mover) {
            const Group group = node.board.groupAt(neighbor);
            if (group.liberties.size() == 1) {
                priority += 3.0 + static_cast<double>(group.stones.size());
            }
        } else if (stone == enemy && child.board.at(neighbor) == enemy) {
            const int enemyLiberties = child.board.libertyCount(neighbor);
            priority += enemyLiberties == 1 ? 2.5 : (enemyLiberties == 2 ? 0.8 : 0.0);
        }
    }

    const int line = edgeDistance(point, node.board.size());
    if (line == 0) {
        priority -= 1.5;
    } else if (line == 1) {
        priority -= 0.3;
    } else if (line <= 3) {
        priority += 0.4;
    }
    if (node.lastPoint && std::abs(node.lastPoint->row - point.row) + std::abs(node.lastPoint->col - point.col) <= 2) {
        priority += 1.0;
    }
    std::uniform_real_distribution<double> jitter(0.0, 0.3);
    return priority + jitter(rng_);
}

std::vector<Candidate> Searcher::expand(const Node& node, int width, bool atRoot)
{
    std::vector<Candidate> candidates;
    for (const Point& point : candidatePoints(node)) {
        Candidate candidate{point, 0.0, {}};
        int captured = 0;
        if (!tryMove(node, point, candidate.child, captured)) {
            continue;
        }
        candidate.priority = movePriority(node, point, candidate.child, captured);
        if (atRoot && limits_.useAlphaBeta) {
            candidate.priority += evaluate(candidate.child);
        }
        candidates.push_back(std::move(candidate));
    }
    std::sort(candidates.begin(), candidates.end(),
        [](const Candidate& left, const Candidate& right) { return left.priority > right.priority; });
    if (static_cast<int>(candidates.size()) > width) {
        candidates.resize(static_cast<std::size_t>(width));
    }
    return candidates;
}

double Searcher::evaluate(const Node& node) const
{
    const int balance = me_ == Stone::Black ? node.blackCaptures - node.whiteCaptures
                                            : node.whiteCaptures - node.blackCaptures;
    return evaluatePosition(node.board, me_, node.toMove, balance, komi_);
}

bool Searcher::outOfTime()
{
    return stopRequested_.load() || Clock::now() >= deadline_;
}

double Searcher::minimax(const Node& node, int depth, double alpha, double beta)
{
    ++nodes_;
    if ((nodes_ & 31) == 0 && outOfTime()) {
        aborted_ = true;
    }
    if (aborted_) {
        return 0.0;
    }
    if (depth == 0) {
        return evaluate(node);
    }

    const std::vector<Candidate> children = expand(node, limits_.innerWidth, false);
    if (children.empty()) {
        return evaluate(node);
    }

    const bool maximizing = node.toMove == me_;
    double best = maximizing ? -infinity : infinity;
    for (const Candidate& candidate : children) {
        path_.push_back(candidate.child.board.hash());
        const double value = minimax(candidate.child, depth - 1, alpha, beta);
        path_.pop_back();
        if (aborted_) {
            return best;
        }
        if (maximizing) {
            best = std::max(best, value);
            alpha = std::max(alpha, best);
        } else {
            best = std::min(best, value);
            beta = std::min(beta, best);
        }
        if (limits_.useAlphaBeta && beta <= alpha) {
            break;
        }
    }
    return best;
}

}

AIDecision chooseAIMove(const Position& position, Difficulty difficulty, double komi, std::uint64_t seed,
    const std::atomic<bool>& stopRequested)
{
    Searcher searcher(position, difficulty, komi, seed, stopRequested);
    AIDecision decision = searcher.decide();
    if (!position.isLegal(decision.move)) {
        decision.move = Move::pass();
    }
    return decision;
}

AIPlayer::~AIPlayer()
{
    cancel();
}

void AIPlayer::start(const Position& position, Difficulty difficulty, double komi)
{
    cancel();
    stopFlag_ = std::make_shared<std::atomic<bool>>(false);
    const std::uint64_t seed = std::random_device{}();
    pending_ = std::async(std::launch::async, [position, difficulty, komi, seed, flag = stopFlag_]() {
        return chooseAIMove(position, difficulty, komi, seed, *flag);
    });
}

std::optional<AIDecision> AIPlayer::takeDecision()
{
    if (!pending_.valid() || pending_.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
        return std::nullopt;
    }
    return pending_.get();
}

void AIPlayer::cancel()
{
    if (pending_.valid()) {
        stopFlag_->store(true);
        pending_.wait();
        pending_ = {};
    }
}
