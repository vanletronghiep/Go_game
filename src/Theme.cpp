#include "Theme.h"

#include <cmath>

BoardThemeColors getBoardThemeColors(int themeIndex)
{
    switch (themeIndex) {
    case 1:
        return {
            sf::Color(240, 230, 210),
            sf::Color(230, 210, 168),
            sf::Color(90, 70, 45),
            sf::Color(70, 50, 30),
            sf::Color(80, 60, 40),
            sf::Color(160, 130, 90)
        };
    case 2:
        return {
            sf::Color(50, 40, 35),
            sf::Color(139, 90, 43),
            sf::Color(45, 25, 10),
            sf::Color(35, 15, 5),
            sf::Color(220, 200, 180),
            sf::Color(80, 50, 20)
        };
    case 3:
        return {
            sf::Color(25, 30, 40),
            sf::Color(62, 74, 89),
            sf::Color(140, 160, 185),
            sf::Color(190, 210, 235),
            sf::Color(180, 200, 220),
            sf::Color(40, 50, 65)
        };
    case 4:
        return {
            sf::Color(30, 45, 40),
            sf::Color(107, 142, 129),
            sf::Color(40, 65, 55),
            sf::Color(25, 45, 35),
            sf::Color(210, 235, 225),
            sf::Color(60, 90, 80)
        };
    case 0:
    default:
        return {
            sf::Color(235, 225, 205),
            sf::Color(220, 179, 92),
            sf::Color(70, 50, 25),
            sf::Color(50, 35, 15),
            sf::Color(70, 50, 25),
            sf::Color(150, 110, 50)
        };
    }
}

void drawStone(sf::RenderTarget& target, sf::Vector2f center, float radius, Stone color,
    int styleIndex, bool isHover, bool isDead, bool isLastMove)
{
    if (color == Stone::Empty) {
        return;
    }

    const std::uint8_t alpha = isHover ? 110 : (isDead ? 130 : 255);
    const bool isBlack = (color == Stone::Black);

    sf::CircleShape shadow(radius * 1.02f);
    shadow.setOrigin({radius * 1.02f, radius * 1.02f});
    shadow.setPosition({center.x + radius * 0.12f, center.y + radius * 0.15f});
    shadow.setFillColor(sf::Color(0, 0, 0, isHover ? 30 : 70));
    target.draw(shadow);

    sf::CircleShape body(radius);
    body.setOrigin({radius, radius});
    body.setPosition(center);

    if (styleIndex == 0) {
        if (isBlack) {
            body.setFillColor(sf::Color(25, 25, 28, alpha));
            body.setOutlineColor(sf::Color(10, 10, 12, alpha));
            body.setOutlineThickness(1.0f);
            target.draw(body);

            sf::CircleShape specular(radius * 0.75f);
            specular.setOrigin({radius * 0.75f, radius * 0.75f});
            specular.setPosition({center.x - radius * 0.18f, center.y - radius * 0.2f});
            specular.setFillColor(sf::Color(65, 68, 75, static_cast<std::uint8_t>(alpha * 0.5f)));
            target.draw(specular);

            sf::CircleShape glint(radius * 0.28f);
            glint.setOrigin({radius * 0.28f, radius * 0.28f});
            glint.setPosition({center.x - radius * 0.3f, center.y - radius * 0.32f});
            glint.setFillColor(sf::Color(120, 125, 135, static_cast<std::uint8_t>(alpha * 0.4f)));
            target.draw(glint);
        } else {
            body.setFillColor(sf::Color(245, 245, 242, alpha));
            body.setOutlineColor(sf::Color(185, 185, 180, alpha));
            body.setOutlineThickness(1.2f);
            target.draw(body);

            sf::CircleShape shade(radius * 0.9f);
            shade.setOrigin({radius * 0.9f, radius * 0.9f});
            shade.setPosition({center.x + radius * 0.1f, center.y + radius * 0.1f});
            shade.setFillColor(sf::Color(215, 215, 210, static_cast<std::uint8_t>(alpha * 0.4f)));
            target.draw(shade);

            sf::CircleShape glint(radius * 0.35f);
            glint.setOrigin({radius * 0.35f, radius * 0.35f});
            glint.setPosition({center.x - radius * 0.28f, center.y - radius * 0.28f});
            glint.setFillColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(alpha * 0.85f)));
            target.draw(glint);
        }
    } else if (styleIndex == 1) {
        if (isBlack) {
            body.setFillColor(sf::Color(18, 20, 24, alpha));
            body.setOutlineColor(sf::Color(80, 90, 105, alpha));
            body.setOutlineThickness(1.5f);
            target.draw(body);

            sf::CircleShape centerPip(radius * 0.15f);
            centerPip.setOrigin({radius * 0.15f, radius * 0.15f});
            centerPip.setPosition(center);
            centerPip.setFillColor(sf::Color(70, 80, 95, static_cast<std::uint8_t>(alpha * 0.7f)));
            target.draw(centerPip);
        } else {
            body.setFillColor(sf::Color(250, 252, 255, alpha));
            body.setOutlineColor(sf::Color(160, 175, 195, alpha));
            body.setOutlineThickness(1.5f);
            target.draw(body);

            sf::CircleShape centerPip(radius * 0.15f);
            centerPip.setOrigin({radius * 0.15f, radius * 0.15f});
            centerPip.setPosition(center);
            centerPip.setFillColor(sf::Color(190, 205, 225, static_cast<std::uint8_t>(alpha * 0.7f)));
            target.draw(centerPip);
        }
    } else {
        if (isBlack) {
            body.setFillColor(sf::Color(40, 36, 48, alpha));
            body.setOutlineColor(sf::Color(12, 10, 16, alpha));
            body.setOutlineThickness(2.8f);
            target.draw(body);

            sf::CircleShape eye(radius * 0.32f);
            eye.setOrigin({radius * 0.32f, radius * 0.32f});
            eye.setPosition({center.x - radius * 0.28f, center.y - radius * 0.28f});
            eye.setFillColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(alpha * 0.8f)));
            target.draw(eye);

            sf::CircleShape eye2(radius * 0.12f);
            eye2.setOrigin({radius * 0.12f, radius * 0.12f});
            eye2.setPosition({center.x + radius * 0.25f, center.y + radius * 0.25f});
            eye2.setFillColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(alpha * 0.6f)));
            target.draw(eye2);
        } else {
            body.setFillColor(sf::Color(255, 250, 240, alpha));
            body.setOutlineColor(sf::Color(110, 85, 55, alpha));
            body.setOutlineThickness(2.8f);
            target.draw(body);

            sf::CircleShape eye(radius * 0.36f);
            eye.setOrigin({radius * 0.36f, radius * 0.36f});
            eye.setPosition({center.x - radius * 0.28f, center.y - radius * 0.28f});
            eye.setFillColor(sf::Color(255, 255, 255, alpha));
            target.draw(eye);
        }
    }

    if (isLastMove && !isHover) {
        sf::CircleShape ring(radius * 0.42f);
        ring.setOrigin({radius * 0.42f, radius * 0.42f});
        ring.setPosition(center);
        ring.setFillColor(sf::Color::Transparent);
        ring.setOutlineColor(isBlack ? sf::Color(220, 70, 70) : sf::Color(190, 40, 40));
        ring.setOutlineThickness(2.2f);
        target.draw(ring);
    }

    if (isDead) {
        const float arm = radius * 0.55f;
        sf::RectangleShape line1(sf::Vector2f(arm * 2.0f, 3.0f));
        line1.setOrigin({arm, 1.5f});
        line1.setPosition(center);
        line1.setRotation(sf::degrees(45.0f));
        line1.setFillColor(sf::Color(220, 40, 40, 220));
        target.draw(line1);

        sf::RectangleShape line2(sf::Vector2f(arm * 2.0f, 3.0f));
        line2.setOrigin({arm, 1.5f});
        line2.setPosition(center);
        line2.setRotation(sf::degrees(-45.0f));
        line2.setFillColor(sf::Color(220, 40, 40, 220));
        target.draw(line2);
    }
}
