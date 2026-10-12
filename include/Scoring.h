#pragma once

#include "Board.h"

#include <vector>

struct ScoreBreakdown {
    int blackStones = 0;
    int whiteStones = 0;
    int blackTerritory = 0;
    int whiteTerritory = 0;
    int neutralPoints = 0;
    int deadBlackStones = 0;
    int deadWhiteStones = 0;
    double komi = 0.0;
    double blackTotal = 0.0;
    double whiteTotal = 0.0;
    Stone winner = Stone::Empty;
    std::vector<Stone> areaOwner;

    double margin() const { return blackTotal > whiteTotal ? blackTotal - whiteTotal : whiteTotal - blackTotal; }
};

ScoreBreakdown scoreByArea(const Board& board, const std::vector<bool>& deadStones, double komi);
