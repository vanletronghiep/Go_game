#pragma once

#include "Game.h"
#include "Gui.h"
#include "Settings.h"

#include <SFML/Graphics.hpp>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

enum class DialogType {
    None,
    NewGame,
    Save,
    Load,
    Settings,
    Scoring
};

struct NewGameResult {
    GameConfig config;
};

struct SaveLoadResult {
    bool confirmed = false;
    std::filesystem::path path;
};

class NewGameDialog {
public:
    NewGameDialog(const sf::Font& font);

    void show(const GameConfig& currentConfig);
    void updateLayout(sf::Vector2u windowSize);
    std::optional<NewGameResult> handleEvent(const sf::Vector2f& mousePos,
        bool mousePressed, bool mouseReleased, bool& closed);
    void render(sf::RenderTarget& target, sf::Vector2u windowSize) const;

private:
    const sf::Font& font_;
    sf::FloatRect bounds_;
    int modeIndex_ = 0;
    int difficultyIndex_ = 1;
    int boardSizeIndex_ = 2;
    int colorIndex_ = 0;

    SegmentedControl modeControl_;
    SegmentedControl difficultyControl_;
    SegmentedControl sizeControl_;
    SegmentedControl colorControl_;
    Button startButton_;
    Button cancelButton_;
};

class SaveDialog {
public:
    SaveDialog(const sf::Font& font);

    void show(const std::string& defaultName);
    void updateLayout(sf::Vector2u windowSize);
    std::optional<SaveLoadResult> handleEvent(const sf::Vector2f& mousePos,
        bool mousePressed, bool mouseReleased, char32_t unicode, bool& closed);
    void render(sf::RenderTarget& target, sf::Vector2u windowSize) const;

private:
    const sf::Font& font_;
    sf::FloatRect bounds_;
    TextInput nameInput_;
    Button saveButton_;
    Button cancelButton_;
};

class LoadDialog {
public:
    LoadDialog(const sf::Font& font);

    void show();
    void updateLayout(sf::Vector2u windowSize);
    std::optional<SaveLoadResult> handleEvent(const sf::Vector2f& mousePos,
        bool mousePressed, bool mouseReleased, bool& closed);
    void render(sf::RenderTarget& target, sf::Vector2u windowSize) const;

private:
    const sf::Font& font_;
    sf::FloatRect bounds_;
    std::vector<std::filesystem::path> files_;
    int selectedIndex_ = -1;
    Button loadButton_;
    Button cancelButton_;
};

class SettingsDialog {
public:
    SettingsDialog(const sf::Font& font);

    void show(const Settings& current);
    void updateLayout(sf::Vector2u windowSize);
    std::optional<Settings> handleEvent(const sf::Vector2f& mousePos,
        bool mousePressed, bool mouseReleased, bool& closed);
    void render(sf::RenderTarget& target, sf::Vector2u windowSize) const;

private:
    const sf::Font& font_;
    sf::FloatRect bounds_;
    Settings current_;

    SegmentedControl themeControl_;
    SegmentedControl stoneStyleControl_;
    Checkbox soundCheckbox_;
    SegmentedControl soundThemeControl_;
    Slider soundVolumeSlider_;
    Checkbox musicCheckbox_;
    SegmentedControl musicTrackControl_;
    Slider musicVolumeSlider_;
    Checkbox coordsCheckbox_;

    Button applyButton_;
    Button cancelButton_;
};

enum class ScoringDialogAction {
    None,
    Resume,
    Accept,
    NewGame
};

class ScoringDialog {
public:
    ScoringDialog(const sf::Font& font);

    void show();
    void updateLayout(sf::Vector2u windowSize);
    ScoringDialogAction handleEvent(const sf::Vector2f& mousePos,
        bool mousePressed, bool mouseReleased, Phase currentPhase);
    void render(sf::RenderTarget& target, sf::Vector2u windowSize, const Game& game) const;

private:
    const sf::Font& font_;
    sf::FloatRect bounds_;
    Button resumeButton_;
    Button acceptButton_;
    Button newGameButton_;
};
