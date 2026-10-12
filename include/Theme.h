#pragma once

#include "Settings.h"
#include "Types.h"

#include <SFML/Graphics.hpp>
#include <string>

struct BoardThemeColors {
    sf::Color background;
    sf::Color wood;
    sf::Color gridLine;
    sf::Color starPoint;
    sf::Color labelText;
    sf::Color border;
};

BoardThemeColors getBoardThemeColors(int themeIndex);

void drawStone(sf::RenderTarget& target, sf::Vector2f center, float radius, Stone color,
    int styleIndex, bool isHover = false, bool isDead = false, bool isLastMove = false);
