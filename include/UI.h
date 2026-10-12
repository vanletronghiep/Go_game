#pragma once

#include "AI.h"
#include "Audio.h"
#include "BoardView.h"
#include "Game.h"
#include "Gui.h"
#include "Screens.h"
#include "Settings.h"

#include <SFML/Graphics.hpp>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class UI {
public:
    UI();
    ~UI();

    void run();

private:
    void processEvents();
    void update(float dt);
    void render();

    void updateLayout();
    void handleBoardClick(Point point);
    void triggerAIMoveIfNeeded();

    void onNewGame(const GameConfig& config);
    void onPass();
    void onUndo();
    void onRedo();
    void onReset();
    void onSaveGame(const std::filesystem::path& path);
    void onLoadGame(const std::filesystem::path& path);
    void onApplySettings(const Settings& settings);

    sf::RenderWindow window_;
    sf::Font fontRegular_;
    sf::Font fontSemiBold_;

    Game game_;
    AIPlayer aiPlayer_;
    Settings settings_;
    AudioManager audioManager_;

    BoardView boardView_;
    NotificationBanner banner_;

    DialogType activeDialog_ = DialogType::None;
    NewGameDialog newGameDialog_;
    SaveDialog saveDialog_;
    LoadDialog loadDialog_;
    SettingsDialog settingsDialog_;
    ScoringDialog scoringDialog_;

    Button passButton_;
    Button undoButton_;
    Button redoButton_;
    Button resetButton_;
    Button newGameButton_;
    Button saveButton_;
    Button loadButton_;
    Button settingsButton_;
    Button exitButton_;

    sf::FloatRect sidePanelBounds_;
    std::optional<Point> hoverPoint_;
    bool isLeftPressed_ = false;
};
