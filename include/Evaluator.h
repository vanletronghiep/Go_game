#pragma once

#include "Board.h"

#include <vector>

struct GroupSummary {
    Stone color = Stone::Empty;
    int size = 0;
    int liberties = 0;
};

std::vector<GroupSummary> summarizeGroups(const Board& board);
std::vector<Stone> estimateOwnership(const Board& board, int reach = 4);
double estimateScoreMargin(const Board& board, Stone perspective, double komi);
double evaluatePosition(const Board& board, Stone perspective, Stone toMove, int captureBalance, double komi);
bool isOwnEye(const Board& board, Point point, Stone color);
