#include "Gui.h"

#include <algorithm>
#include <cmath>

Button::Button(const std::string& label, sf::FloatRect bounds, const sf::Font& font,
    unsigned int fontSize)
    : label_(label)
    , bounds_(bounds)
    , font_(font)
    , fontSize_(fontSize)
{
}

void Button::setBounds(sf::FloatRect bounds)
{
    bounds_ = bounds;
}

void Button::setLabel(const std::string& label)
{
    label_ = label;
}

bool Button::handleEvent(const sf::Vector2f& mousePos, bool mousePressed, bool mouseReleased)
{
    if (!enabled_) {
        isHovered_ = false;
        isPressed_ = false;
        return false;
    }

    isHovered_ = bounds_.contains(mousePos);

    if (mousePressed && isHovered_) {
        isPressed_ = true;
    }

    if (mouseReleased) {
        const bool clicked = (isPressed_ && isHovered_);
        isPressed_ = false;
        return clicked;
    }

    return false;
}

void Button::render(sf::RenderTarget& target) const
{
    sf::RectangleShape bg(bounds_.size);
    bg.setPosition(bounds_.position);

    if (!enabled_) {
        bg.setFillColor(sf::Color(38, 42, 50, 180));
        bg.setOutlineColor(sf::Color(60, 65, 75, 120));
    } else if (isPressed_) {
        bg.setFillColor(isAccent_ ? sf::Color(35, 90, 175) : sf::Color(30, 35, 42));
        bg.setOutlineColor(sf::Color(100, 110, 125));
    } else if (isHovered_) {
        bg.setFillColor(isAccent_ ? sf::Color(65, 130, 230) : sf::Color(65, 74, 90));
        bg.setOutlineColor(isAccent_ ? sf::Color(140, 190, 255) : sf::Color(120, 135, 155));
    } else {
        bg.setFillColor(isAccent_ ? sf::Color(50, 110, 205) : sf::Color(45, 52, 64));
        bg.setOutlineColor(isAccent_ ? sf::Color(90, 150, 240) : sf::Color(75, 85, 100));
    }
    bg.setOutlineThickness(1.5f);
    target.draw(bg);

    sf::Text text(font_, label_, fontSize_);
    if (!enabled_) {
        text.setFillColor(sf::Color(120, 125, 135));
    } else {
        text.setFillColor(sf::Color(245, 248, 252));
    }

    const sf::FloatRect textBounds = text.getLocalBounds();
    const float tx = bounds_.position.x + (bounds_.size.x - textBounds.size.x) * 0.5f - textBounds.position.x;
    const float ty = bounds_.position.y + (bounds_.size.y - textBounds.size.y) * 0.5f - textBounds.position.y;
    text.setPosition({std::round(tx), std::round(ty)});
    target.draw(text);
}

Checkbox::Checkbox(const std::string& label, sf::Vector2f position, bool checked,
    const sf::Font& font, unsigned int fontSize)
    : label_(label)
    , position_(position)
    , checked_(checked)
    , font_(font)
    , fontSize_(fontSize)
{
    bounds_ = sf::FloatRect(position_, {220.0f, 28.0f});
}

void Checkbox::setPosition(sf::Vector2f position)
{
    position_ = position;
    bounds_.position = position;
}

bool Checkbox::handleEvent(const sf::Vector2f& mousePos, bool mouseReleased)
{
    isHovered_ = bounds_.contains(mousePos);
    if (mouseReleased && isHovered_) {
        checked_ = !checked_;
        return true;
    }
    return false;
}

void Checkbox::render(sf::RenderTarget& target) const
{
    const float boxSize = 20.0f;
    sf::RectangleShape box({boxSize, boxSize});
    box.setPosition(position_);
    box.setFillColor(isHovered_ ? sf::Color(55, 63, 78) : sf::Color(38, 44, 55));
    box.setOutlineColor(checked_ ? sf::Color(70, 140, 240) : sf::Color(90, 100, 118));
    box.setOutlineThickness(1.5f);
    target.draw(box);

    if (checked_) {
        sf::RectangleShape checkMark({boxSize - 8.0f, boxSize - 8.0f});
        checkMark.setPosition({position_.x + 4.0f, position_.y + 4.0f});
        checkMark.setFillColor(sf::Color(70, 140, 240));
        target.draw(checkMark);
    }

    sf::Text text(font_, label_, fontSize_);
    text.setPosition({position_.x + boxSize + 10.0f, position_.y - 1.0f});
    text.setFillColor(isHovered_ ? sf::Color(255, 255, 255) : sf::Color(225, 230, 238));
    target.draw(text);
}

Slider::Slider(const std::string& label, sf::FloatRect bounds, float value,
    const sf::Font& font, unsigned int fontSize)
    : label_(label)
    , bounds_(bounds)
    , value_(std::clamp(value, 0.0f, 1.0f))
    , font_(font)
    , fontSize_(fontSize)
{
}

void Slider::setBounds(sf::FloatRect bounds)
{
    bounds_ = bounds;
}

void Slider::setValue(float value)
{
    value_ = std::clamp(value, 0.0f, 1.0f);
}

bool Slider::handleEvent(const sf::Vector2f& mousePos, bool mousePressed, bool mouseReleased)
{
    const float trackY = bounds_.position.y + 24.0f;
    const float trackHeight = 16.0f;
    const sf::FloatRect hitArea({bounds_.position.x, trackY - 6.0f}, {bounds_.size.x, trackHeight + 12.0f});

    if (mousePressed && hitArea.contains(mousePos)) {
        isDragging_ = true;
    }
    if (mouseReleased) {
        isDragging_ = false;
    }

    if (isDragging_) {
        const float relX = mousePos.x - bounds_.position.x;
        const float newVal = std::clamp(relX / bounds_.size.x, 0.0f, 1.0f);
        if (std::abs(newVal - value_) > 0.001f) {
            value_ = newVal;
            return true;
        }
    }
    return false;
}

void Slider::render(sf::RenderTarget& target) const
{
    sf::Text labelText(font_, label_, fontSize_);
    labelText.setPosition(bounds_.position);
    labelText.setFillColor(sf::Color(220, 226, 235));
    target.draw(labelText);

    const int pct = static_cast<int>(std::round(value_ * 100.0f));
    sf::Text valText(font_, std::to_string(pct) + "%", fontSize_);
    const sf::FloatRect valBounds = valText.getLocalBounds();
    valText.setPosition({bounds_.position.x + bounds_.size.x - valBounds.size.x, bounds_.position.y});
    valText.setFillColor(sf::Color(160, 175, 195));
    target.draw(valText);

    const float trackY = bounds_.position.y + 26.0f;
    const float trackH = 6.0f;
    sf::RectangleShape trackBg({bounds_.size.x, trackH});
    trackBg.setPosition({bounds_.position.x, trackY});
    trackBg.setFillColor(sf::Color(32, 38, 48));
    target.draw(trackBg);

    sf::RectangleShape trackFill({bounds_.size.x * value_, trackH});
    trackFill.setPosition({bounds_.position.x, trackY});
    trackFill.setFillColor(sf::Color(65, 130, 225));
    target.draw(trackFill);

    const float thumbRadius = 9.0f;
    sf::CircleShape thumb(thumbRadius);
    thumb.setOrigin({thumbRadius, thumbRadius});
    thumb.setPosition({bounds_.position.x + bounds_.size.x * value_, trackY + trackH * 0.5f});
    thumb.setFillColor(sf::Color(240, 245, 255));
    thumb.setOutlineColor(sf::Color(50, 110, 200));
    thumb.setOutlineThickness(2.0f);
    target.draw(thumb);
}

SegmentedControl::SegmentedControl(const std::vector<std::string>& options, sf::FloatRect bounds,
    int selectedIndex, const sf::Font& font, unsigned int fontSize)
    : options_(options)
    , bounds_(bounds)
    , selectedIndex_(selectedIndex)
    , font_(font)
    , fontSize_(fontSize)
{
}

void SegmentedControl::setBounds(sf::FloatRect bounds)
{
    bounds_ = bounds;
}

void SegmentedControl::setSelectedIndex(int index)
{
    selectedIndex_ = std::clamp(index, 0, static_cast<int>(options_.size()) - 1);
}

bool SegmentedControl::handleEvent(const sf::Vector2f& mousePos, bool mouseReleased)
{
    hoveredIndex_ = -1;
    if (options_.empty()) {
        return false;
    }

    const float segWidth = bounds_.size.x / static_cast<float>(options_.size());
    for (std::size_t i = 0; i < options_.size(); ++i) {
        const sf::FloatRect segRect(
            {bounds_.position.x + static_cast<float>(i) * segWidth, bounds_.position.y},
            {segWidth, bounds_.size.y}
        );
        if (segRect.contains(mousePos)) {
            hoveredIndex_ = static_cast<int>(i);
            if (mouseReleased && hoveredIndex_ != selectedIndex_) {
                selectedIndex_ = hoveredIndex_;
                return true;
            }
        }
    }
    return false;
}

void SegmentedControl::render(sf::RenderTarget& target) const
{
    if (options_.empty()) {
        return;
    }

    sf::RectangleShape baseBg(bounds_.size);
    baseBg.setPosition(bounds_.position);
    baseBg.setFillColor(sf::Color(32, 37, 46));
    baseBg.setOutlineColor(sf::Color(65, 75, 92));
    baseBg.setOutlineThickness(1.5f);
    target.draw(baseBg);

    const float segWidth = bounds_.size.x / static_cast<float>(options_.size());
    for (std::size_t i = 0; i < options_.size(); ++i) {
        const sf::FloatRect segRect(
            {bounds_.position.x + static_cast<float>(i) * segWidth, bounds_.position.y},
            {segWidth, bounds_.size.y}
        );

        const bool isSel = (static_cast<int>(i) == selectedIndex_);
        const bool isHov = (static_cast<int>(i) == hoveredIndex_);

        if (isSel) {
            sf::RectangleShape selBg({segWidth - 2.0f, bounds_.size.y - 2.0f});
            selBg.setPosition({segRect.position.x + 1.0f, segRect.position.y + 1.0f});
            selBg.setFillColor(sf::Color(60, 125, 215));
            target.draw(selBg);
        } else if (isHov) {
            sf::RectangleShape hovBg({segWidth - 2.0f, bounds_.size.y - 2.0f});
            hovBg.setPosition({segRect.position.x + 1.0f, segRect.position.y + 1.0f});
            hovBg.setFillColor(sf::Color(48, 55, 68));
            target.draw(hovBg);
        }

        if (i > 0 && !isSel && static_cast<int>(i - 1) != selectedIndex_) {
            sf::RectangleShape div({1.0f, bounds_.size.y * 0.6f});
            div.setPosition({segRect.position.x, bounds_.position.y + bounds_.size.y * 0.2f});
            div.setFillColor(sf::Color(60, 70, 85));
            target.draw(div);
        }

        sf::Text labelText(font_, options_[i], fontSize_);
        labelText.setFillColor(isSel ? sf::Color(255, 255, 255) : (isHov ? sf::Color(230, 235, 245) : sf::Color(170, 180, 195)));
        const sf::FloatRect tb = labelText.getLocalBounds();
        const float tx = segRect.position.x + (segWidth - tb.size.x) * 0.5f - tb.position.x;
        const float ty = segRect.position.y + (bounds_.size.y - tb.size.y) * 0.5f - tb.position.y;
        labelText.setPosition({std::round(tx), std::round(ty)});
        target.draw(labelText);
    }
}

TextInput::TextInput(const std::string& initialText, sf::FloatRect bounds,
    const sf::Font& font, unsigned int fontSize)
    : text_(initialText)
    , bounds_(bounds)
    , font_(font)
    , fontSize_(fontSize)
{
}

void TextInput::setBounds(sf::FloatRect bounds)
{
    bounds_ = bounds;
}

void TextInput::handleEvent(const sf::Vector2f& mousePos, bool mousePressed, char32_t unicode)
{
    if (mousePressed) {
        isFocused_ = bounds_.contains(mousePos);
    }

    if (isFocused_ && unicode != 0) {
        if (unicode == 8) {
            if (!text_.empty()) {
                text_.pop_back();
            }
        } else if (unicode >= 32 && unicode < 127 && text_.size() < 32) {
            text_.push_back(static_cast<char>(unicode));
        }
    }
}

void TextInput::render(sf::RenderTarget& target) const
{
    sf::RectangleShape bg(bounds_.size);
    bg.setPosition(bounds_.position);
    bg.setFillColor(sf::Color(26, 30, 38));
    bg.setOutlineColor(isFocused_ ? sf::Color(70, 140, 240) : sf::Color(75, 85, 102));
    bg.setOutlineThickness(1.5f);
    target.draw(bg);

    std::string display = text_ + (isFocused_ ? "|" : "");
    sf::Text text(font_, display, fontSize_);
    text.setPosition({bounds_.position.x + 10.0f, bounds_.position.y + (bounds_.size.y - fontSize_) * 0.5f - 2.0f});
    text.setFillColor(sf::Color(240, 245, 252));
    target.draw(text);
}

NotificationBanner::NotificationBanner(const sf::Font& font)
    : font_(font)
{
}

void NotificationBanner::show(const std::string& message, bool isError, float durationSeconds)
{
    message_ = message;
    isError_ = isError;
    timer_ = durationSeconds;
    maxDuration_ = durationSeconds;
}

void NotificationBanner::update(float dt)
{
    if (timer_ > 0.0f) {
        timer_ = std::max(0.0f, timer_ - dt);
    }
}

void NotificationBanner::render(sf::RenderTarget& target, float windowWidth) const
{
    if (timer_ <= 0.0f || message_.empty()) {
        return;
    }

    const float alphaFactor = std::min(1.0f, timer_ / 0.4f);
    const std::uint8_t alpha = static_cast<std::uint8_t>(240.0f * alphaFactor);

    sf::Text text(font_, message_, 15);
    text.setFillColor(sf::Color(255, 255, 255, alpha));
    const sf::FloatRect textBounds = text.getLocalBounds();

    const float padX = 24.0f;
    const float padY = 10.0f;
    const float bannerW = textBounds.size.x + padX * 2.0f;
    const float bannerH = textBounds.size.y + padY * 2.0f;
    const float bannerX = (windowWidth - bannerW) * 0.5f;
    const float bannerY = 14.0f;

    sf::RectangleShape banner({bannerW, bannerH});
    banner.setPosition({bannerX, bannerY});
    if (isError_) {
        banner.setFillColor(sf::Color(180, 40, 45, alpha));
        banner.setOutlineColor(sf::Color(240, 90, 95, alpha));
    } else {
        banner.setFillColor(sf::Color(35, 110, 75, alpha));
        banner.setOutlineColor(sf::Color(70, 185, 130, alpha));
    }
    banner.setOutlineThickness(1.5f);
    target.draw(banner);

    const float tx = bannerX + padX - textBounds.position.x;
    const float ty = bannerY + padY - textBounds.position.y;
    text.setPosition({std::round(tx), std::round(ty)});
    target.draw(text);
}
