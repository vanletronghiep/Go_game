#pragma once

#include "Board.h"
#include "Game.h"
#include "Settings.h"
#include "Theme.h"

#include <SFML/Graphics.hpp>
#include <optional>
#include <vector>

class BoardView {
public:
    BoardView(const sf::Font& font);

    void updateLayout(sf::FloatRect availableArea, int boardSize);
    std::optional<Point> screenToPoint(const sf::Vector2f& screenPos) const;
    sf::Vector2f pointToScreen(Point point) const;
    sf::FloatRect getBounds() const { return bounds_; }

    void render(sf::RenderTarget& target, const Game& game, const Settings& settings,
        const std::optional<Point>& hoverPoint);

private:
    void renderGrid(sf::RenderTarget& target, int size, const BoardThemeColors& colors, bool showCoords);
    void renderStarPoints(sf::RenderTarget& target, int size, const BoardThemeColors& colors);
    void renderCoordinates(sf::RenderTarget& target, int size, const BoardThemeColors& colors);
    void renderAreaTerritory(sf::RenderTarget& target, const Game& game);

    const sf::Font& font_;
    sf::FloatRect bounds_;
    float margin_ = 40.0f;
    float cellPitch_ = 32.0f;
    float stoneRadius_ = 14.0f;
    int boardSize_ = 19;
};
