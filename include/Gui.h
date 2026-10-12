#pragma once

#include <SFML/Graphics.hpp>
#include <functional>
#include <string>
#include <vector>

struct WidgetStyle {
    sf::Color background{45, 50, 60};
    sf::Color hoverBackground{60, 68, 82};
    sf::Color activeBackground{30, 35, 42};
    sf::Color text{240, 242, 245};
    sf::Color border{80, 90, 105};
    sf::Color accent{70, 130, 220};
    sf::Color disabledText{120, 125, 135};
};

class Button {
public:
    Button(const std::string& label, sf::FloatRect bounds, const sf::Font& font,
        unsigned int fontSize = 16);

    void setBounds(sf::FloatRect bounds);
    sf::FloatRect getBounds() const { return bounds_; }
    void setLabel(const std::string& label);
    void setEnabled(bool enabled) { enabled_ = enabled; }
    bool isEnabled() const { return enabled_; }
    void setAccent(bool isAccent) { isAccent_ = isAccent; }

    bool handleEvent(const sf::Vector2f& mousePos, bool mousePressed, bool mouseReleased);
    void render(sf::RenderTarget& target) const;

private:
    std::string label_;
    sf::FloatRect bounds_;
    const sf::Font& font_;
    unsigned int fontSize_;
    bool enabled_ = true;
    bool isHovered_ = false;
    bool isPressed_ = false;
    bool isAccent_ = false;
};

class Checkbox {
public:
    Checkbox(const std::string& label, sf::Vector2f position, bool checked,
        const sf::Font& font, unsigned int fontSize = 16);

    void setPosition(sf::Vector2f position);
    bool isChecked() const { return checked_; }
    void setChecked(bool checked) { checked_ = checked; }

    bool handleEvent(const sf::Vector2f& mousePos, bool mouseReleased);
    void render(sf::RenderTarget& target) const;

private:
    std::string label_;
    sf::Vector2f position_;
    bool checked_;
    const sf::Font& font_;
    unsigned int fontSize_;
    bool isHovered_ = false;
    sf::FloatRect bounds_;
};

class Slider {
public:
    Slider(const std::string& label, sf::FloatRect bounds, float value,
        const sf::Font& font, unsigned int fontSize = 15);

    void setBounds(sf::FloatRect bounds);
    float getValue() const { return value_; }
    void setValue(float value);

    bool handleEvent(const sf::Vector2f& mousePos, bool mousePressed, bool mouseReleased);
    void render(sf::RenderTarget& target) const;

private:
    std::string label_;
    sf::FloatRect bounds_;
    float value_;
    const sf::Font& font_;
    unsigned int fontSize_;
    bool isDragging_ = false;
};

class SegmentedControl {
public:
    SegmentedControl(const std::vector<std::string>& options, sf::FloatRect bounds,
        int selectedIndex, const sf::Font& font, unsigned int fontSize = 14);

    void setBounds(sf::FloatRect bounds);
    int getSelectedIndex() const { return selectedIndex_; }
    void setSelectedIndex(int index);

    bool handleEvent(const sf::Vector2f& mousePos, bool mouseReleased);
    void render(sf::RenderTarget& target) const;

private:
    std::vector<std::string> options_;
    sf::FloatRect bounds_;
    int selectedIndex_;
    const sf::Font& font_;
    unsigned int fontSize_;
    int hoveredIndex_ = -1;
};

class TextInput {
public:
    TextInput(const std::string& initialText, sf::FloatRect bounds,
        const sf::Font& font, unsigned int fontSize = 16);

    void setBounds(sf::FloatRect bounds);
    std::string getText() const { return text_; }
    void setText(const std::string& text) { text_ = text; }

    void handleEvent(const sf::Vector2f& mousePos, bool mousePressed, char32_t unicode);
    void render(sf::RenderTarget& target) const;

private:
    std::string text_;
    sf::FloatRect bounds_;
    const sf::Font& font_;
    unsigned int fontSize_;
    bool isFocused_ = false;
};

class NotificationBanner {
public:
    NotificationBanner(const sf::Font& font);

    void show(const std::string& message, bool isError = false, float durationSeconds = 3.5f);
    void update(float dt);
    void render(sf::RenderTarget& target, float windowWidth) const;

private:
    std::string message_;
    bool isError_ = false;
    float timer_ = 0.0f;
    float maxDuration_ = 3.5f;
    const sf::Font& font_;
};
