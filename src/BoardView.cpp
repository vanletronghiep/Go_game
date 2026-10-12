#include "BoardView.h"

#include <algorithm>
#include <cmath>

namespace {

std::vector<Point> getStarPoints(int size)
{
    if (size == 19) {
        return {
            {3, 3}, {3, 9}, {3, 15},
            {9, 3}, {9, 9}, {9, 15},
            {15, 3}, {15, 9}, {15, 15}
        };
    }
    if (size == 13) {
        return {
            {3, 3}, {3, 9},
            {6, 6},
            {9, 3}, {9, 9}
        };
    }
    if (size == 9) {
        return {
            {2, 2}, {2, 6},
            {4, 4},
            {6, 2}, {6, 6}
        };
    }
    return {};
}

const char columnChars[] = "ABCDEFGHJKLMNOPQRST";

}

BoardView::BoardView(const sf::Font& font)
    : font_(font)
{
}

void BoardView::updateLayout(sf::FloatRect availableArea, int boardSize)
{
    boardSize_ = std::max(2, boardSize);

    const float minDimension = std::min(availableArea.size.x, availableArea.size.y);
    const float side = std::max(200.0f, minDimension - 24.0f);

    const float boardX = availableArea.position.x + (availableArea.size.x - side) * 0.5f;
    const float boardY = availableArea.position.y + (availableArea.size.y - side) * 0.5f;
    bounds_ = sf::FloatRect({boardX, boardY}, {side, side});

    margin_ = side * 0.065f;
    const float gridSpan = side - margin_ * 2.0f;
    cellPitch_ = gridSpan / static_cast<float>(boardSize_ - 1);
    stoneRadius_ = cellPitch_ * 0.475f;
}

std::optional<Point> BoardView::screenToPoint(const sf::Vector2f& screenPos) const
{
    const float gridLeft = bounds_.position.x + margin_;
    const float gridTop = bounds_.position.y + margin_;

    const float relX = screenPos.x - gridLeft;
    const float relY = screenPos.y - gridTop;

    const int col = static_cast<int>(std::round(relX / cellPitch_));
    const int row = static_cast<int>(std::round(relY / cellPitch_));

    if (row < 0 || row >= boardSize_ || col < 0 || col >= boardSize_) {
        return std::nullopt;
    }

    const sf::Vector2f interPos = pointToScreen({row, col});
    const float dx = screenPos.x - interPos.x;
    const float dy = screenPos.y - interPos.y;
    const float distSq = dx * dx + dy * dy;

    if (distSq <= stoneRadius_ * stoneRadius_ * 1.35f) {
        return Point{row, col};
    }
    return std::nullopt;
}

sf::Vector2f BoardView::pointToScreen(Point point) const
{
    const float x = bounds_.position.x + margin_ + static_cast<float>(point.col) * cellPitch_;
    const float y = bounds_.position.y + margin_ + static_cast<float>(point.row) * cellPitch_;
    return {x, y};
}

void BoardView::render(sf::RenderTarget& target, const Game& game, const Settings& settings,
    const std::optional<Point>& hoverPoint)
{
    const BoardThemeColors colors = getBoardThemeColors(settings.boardTheme);

    sf::RectangleShape shadow(bounds_.size);
    shadow.setPosition({bounds_.position.x + 8.0f, bounds_.position.y + 10.0f});
    shadow.setFillColor(sf::Color(0, 0, 0, 60));
    target.draw(shadow);

    sf::RectangleShape boardBg(bounds_.size);
    boardBg.setPosition(bounds_.position);
    boardBg.setFillColor(colors.wood);
    boardBg.setOutlineColor(colors.border);
    boardBg.setOutlineThickness(2.5f);
    target.draw(boardBg);

    renderGrid(target, game.board().size(), colors, settings.showCoordinates);
    renderStarPoints(target, game.board().size(), colors);

    if (settings.showCoordinates) {
        renderCoordinates(target, game.board().size(), colors);
    }

    if (game.phase() == Phase::Scoring || game.phase() == Phase::Finished) {
        renderAreaTerritory(target, game);
    }

    const std::optional<Move>& lastMove = game.position().lastMove();
    std::optional<Point> lastPoint;
    if (lastMove && !lastMove->isPass()) {
        lastPoint = lastMove->point;
    }

    for (int row = 0; row < game.board().size(); ++row) {
        for (int col = 0; col < game.board().size(); ++col) {
            const Point pt{row, col};
            const Stone stone = game.board().at(pt);
            if (stone != Stone::Empty) {
                const bool isLast = (lastPoint && *lastPoint == pt);
                const bool isDead = game.isDead(pt);
                drawStone(target, pointToScreen(pt), stoneRadius_, stone, settings.stoneStyle,
                    false, isDead, isLast);
            }
        }
    }

    if (game.phase() == Phase::Playing && hoverPoint && game.board().at(*hoverPoint) == Stone::Empty) {
        if (game.position().isLegal(Move::place(*hoverPoint))) {
            drawStone(target, pointToScreen(*hoverPoint), stoneRadius_, game.toMove(),
                settings.stoneStyle, true, false, false);
        }
    }
}

void BoardView::renderGrid(sf::RenderTarget& target, int size, const BoardThemeColors& colors, bool)
{
    const float gridLeft = bounds_.position.x + margin_;
    const float gridTop = bounds_.position.y + margin_;
    const float gridSpan = static_cast<float>(size - 1) * cellPitch_;

    for (int i = 0; i < size; ++i) {
        const float pos = static_cast<float>(i) * cellPitch_;
        const bool isOuter = (i == 0 || i == size - 1);
        const float thick = isOuter ? 2.0f : 1.0f;

        sf::RectangleShape hLine({gridSpan + thick, thick});
        hLine.setPosition({gridLeft - thick * 0.5f, gridTop + pos - thick * 0.5f});
        hLine.setFillColor(colors.gridLine);
        target.draw(hLine);

        sf::RectangleShape vLine({thick, gridSpan + thick});
        vLine.setPosition({gridLeft + pos - thick * 0.5f, gridTop - thick * 0.5f});
        vLine.setFillColor(colors.gridLine);
        target.draw(vLine);
    }
}

void BoardView::renderStarPoints(sf::RenderTarget& target, int size, const BoardThemeColors& colors)
{
    const float starRadius = std::max(2.5f, cellPitch_ * 0.085f);
    for (const Point& pt : getStarPoints(size)) {
        sf::CircleShape dot(starRadius);
        dot.setOrigin({starRadius, starRadius});
        dot.setPosition(pointToScreen(pt));
        dot.setFillColor(colors.starPoint);
        target.draw(dot);
    }
}

void BoardView::renderCoordinates(sf::RenderTarget& target, int size, const BoardThemeColors& colors)
{
    const unsigned int fontSize = static_cast<unsigned int>(std::clamp(cellPitch_ * 0.36f, 10.0f, 15.0f));

    for (int col = 0; col < size; ++col) {
        std::string letter(1, columnChars[col]);
        sf::Text topText(font_, letter, fontSize);
        topText.setFillColor(colors.labelText);
        const sf::FloatRect tb = topText.getLocalBounds();
        const sf::Vector2f pt = pointToScreen({0, col});
        topText.setPosition({pt.x - tb.size.x * 0.5f - tb.position.x, bounds_.position.y + (margin_ * 0.5f) - tb.size.y * 0.5f - tb.position.y});
        target.draw(topText);

        sf::Text botText(font_, letter, fontSize);
        botText.setFillColor(colors.labelText);
        botText.setPosition({pt.x - tb.size.x * 0.5f - tb.position.x, bounds_.position.y + bounds_.size.y - (margin_ * 0.6f) - tb.size.y * 0.5f - tb.position.y});
        target.draw(botText);
    }

    for (int row = 0; row < size; ++row) {
        std::string numStr = std::to_string(size - row);
        sf::Text leftText(font_, numStr, fontSize);
        leftText.setFillColor(colors.labelText);
        const sf::FloatRect tb = leftText.getLocalBounds();
        const sf::Vector2f pt = pointToScreen({row, 0});
        leftText.setPosition({bounds_.position.x + (margin_ * 0.45f) - tb.size.x * 0.5f - tb.position.x, pt.y - tb.size.y * 0.5f - tb.position.y});
        target.draw(leftText);

        sf::Text rightText(font_, numStr, fontSize);
        rightText.setFillColor(colors.labelText);
        rightText.setPosition({bounds_.position.x + bounds_.size.x - (margin_ * 0.55f) - tb.size.x * 0.5f - tb.position.x, pt.y - tb.size.y * 0.5f - tb.position.y});
        target.draw(rightText);
    }
}

void BoardView::renderAreaTerritory(sf::RenderTarget& target, const Game& game)
{
    const ScoreBreakdown score = game.score();
    const float markerSize = cellPitch_ * 0.28f;

    for (int row = 0; row < game.board().size(); ++row) {
        for (int col = 0; col < game.board().size(); ++col) {
            const Point pt{row, col};
            if (game.board().at(pt) != Stone::Empty && !game.isDead(pt)) {
                continue;
            }
            const std::size_t idx = static_cast<std::size_t>(row * game.board().size() + col);
            if (idx >= score.areaOwner.size()) {
                continue;
            }
            const Stone owner = score.areaOwner[idx];
            if (owner == Stone::Empty) {
                continue;
            }

            sf::RectangleShape marker({markerSize, markerSize});
            marker.setOrigin({markerSize * 0.5f, markerSize * 0.5f});
            marker.setPosition(pointToScreen(pt));
            if (owner == Stone::Black) {
                marker.setFillColor(sf::Color(15, 15, 20, 200));
                marker.setOutlineColor(sf::Color(255, 255, 255, 160));
            } else {
                marker.setFillColor(sf::Color(245, 245, 255, 220));
                marker.setOutlineColor(sf::Color(40, 40, 50, 160));
            }
            marker.setOutlineThickness(1.0f);
            target.draw(marker);
        }
    }
}
