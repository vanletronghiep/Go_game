#include "UI.h"

#include "Paths.h"
#include "SaveManager.h"

#include <algorithm>
#include <iomanip>
#include <sstream>

UI::UI()
    : window_(sf::VideoMode({1180, 820}), "Go Game - Modern Desktop Go", sf::Style::Default)
    , game_(GameConfig{19})
    , settings_(loadSettings(settingsFile()))
    , boardView_(fontRegular_)
    , banner_(fontRegular_)
    , newGameDialog_(fontRegular_)
    , saveDialog_(fontRegular_)
    , loadDialog_(fontRegular_)
    , settingsDialog_(fontRegular_)
    , scoringDialog_(fontRegular_)
    , passButton_("Pass", {}, fontRegular_, 15)
    , undoButton_("Undo", {}, fontRegular_, 15)
    , redoButton_("Redo", {}, fontRegular_, 15)
    , resetButton_("Reset", {}, fontRegular_, 15)
    , newGameButton_("New Game", {}, fontRegular_, 15)
    , saveButton_("Save", {}, fontRegular_, 15)
    , loadButton_("Load", {}, fontRegular_, 15)
    , settingsButton_("Settings", {}, fontRegular_, 15)
    , exitButton_("Exit", {}, fontRegular_, 15)
{
    const auto fontRegPath = assetPath("fonts/Poppins-Regular.ttf");
    if (!fontRegular_.openFromFile(fontRegPath)) {
    }
    const auto fontSemiPath = assetPath("fonts/Poppins-SemiBold.ttf");
    if (!fontSemiBold_.openFromFile(fontSemiPath)) {
        fontSemiBold_ = fontRegular_;
    }

    passButton_.setAccent(true);
    newGameButton_.setAccent(true);

    audioManager_.applySettings(settings_);
    updateLayout();
}

UI::~UI()
{
    aiPlayer_.cancel();
    saveSettings(settings_, settingsFile());
}

void UI::updateLayout()
{
    const sf::Vector2u winSize = window_.getSize();
    const float w = static_cast<float>(winSize.x);
    const float h = static_cast<float>(winSize.y);

    const float panelW = std::clamp(w * 0.28f, 290.0f, 360.0f);
    sidePanelBounds_ = sf::FloatRect({w - panelW, 0.0f}, {panelW, h});

    const sf::FloatRect boardArea({0.0f, 0.0f}, {w - panelW, h});
    boardView_.updateLayout(boardArea, game_.board().size());

    const float px = sidePanelBounds_.position.x + 20.0f;
    const float pw = sidePanelBounds_.size.x - 40.0f;
    const float btnHalfW = (pw - 12.0f) * 0.5f;

    float curY = 320.0f;
    passButton_.setBounds(sf::FloatRect({px, curY}, {pw, 38.0f}));

    curY += 48.0f;
    undoButton_.setBounds(sf::FloatRect({px, curY}, {btnHalfW, 36.0f}));
    redoButton_.setBounds(sf::FloatRect({px + btnHalfW + 12.0f, curY}, {btnHalfW, 36.0f}));

    curY += 46.0f;
    resetButton_.setBounds(sf::FloatRect({px, curY}, {btnHalfW, 36.0f}));
    newGameButton_.setBounds(sf::FloatRect({px + btnHalfW + 12.0f, curY}, {btnHalfW, 36.0f}));

    curY += 46.0f;
    saveButton_.setBounds(sf::FloatRect({px, curY}, {btnHalfW, 36.0f}));
    loadButton_.setBounds(sf::FloatRect({px + btnHalfW + 12.0f, curY}, {btnHalfW, 36.0f}));

    curY += 46.0f;
    settingsButton_.setBounds(sf::FloatRect({px, curY}, {btnHalfW, 36.0f}));
    exitButton_.setBounds(sf::FloatRect({px + btnHalfW + 12.0f, curY}, {btnHalfW, 36.0f}));

    newGameDialog_.updateLayout(winSize);
    saveDialog_.updateLayout(winSize);
    loadDialog_.updateLayout(winSize);
    settingsDialog_.updateLayout(winSize);
    scoringDialog_.updateLayout(winSize);
}

void UI::run()
{
    sf::Clock clock;
    while (window_.isOpen()) {
        const float dt = clock.restart().asSeconds();
        processEvents();
        update(dt);
        render();
    }
}

void UI::processEvents()
{
    sf::Vector2f mousePos(sf::Mouse::getPosition(window_));
    bool mousePressed = false;
    bool mouseReleased = false;
    char32_t unicodeChar = 0;

    while (const auto event = window_.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window_.close();
            return;
        }

        if (const auto* resize = event->getIf<sf::Event::Resized>()) {
            sf::View view(sf::FloatRect({0.0f, 0.0f}, {static_cast<float>(resize->size.x), static_cast<float>(resize->size.y)}));
            window_.setView(view);
            updateLayout();
        } else if (const auto* mPressed = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (mPressed->button == sf::Mouse::Button::Left) {
                mousePressed = true;
                isLeftPressed_ = true;
            }
        } else if (const auto* mReleased = event->getIf<sf::Event::MouseButtonReleased>()) {
            if (mReleased->button == sf::Mouse::Button::Left) {
                mouseReleased = true;
                isLeftPressed_ = false;
            }
        } else if (const auto* textEv = event->getIf<sf::Event::TextEntered>()) {
            unicodeChar = textEv->unicode;
        }
    }

    mousePos = sf::Vector2f(sf::Mouse::getPosition(window_));

    if (activeDialog_ != DialogType::None) {
        bool closed = false;
        if (activeDialog_ == DialogType::NewGame) {
            auto res = newGameDialog_.handleEvent(mousePos, mousePressed, mouseReleased, closed);
            if (res) {
                onNewGame(res->config);
            }
        } else if (activeDialog_ == DialogType::Save) {
            auto res = saveDialog_.handleEvent(mousePos, mousePressed, mouseReleased, unicodeChar, closed);
            if (res && res->confirmed) {
                onSaveGame(res->path);
            }
        } else if (activeDialog_ == DialogType::Load) {
            auto res = loadDialog_.handleEvent(mousePos, mousePressed, mouseReleased, closed);
            if (res && res->confirmed) {
                onLoadGame(res->path);
            }
        } else if (activeDialog_ == DialogType::Settings) {
            auto res = settingsDialog_.handleEvent(mousePos, mousePressed, mouseReleased, closed);
            if (res) {
                onApplySettings(*res);
            }
        } else if (activeDialog_ == DialogType::Scoring) {
            ScoringDialogAction act = scoringDialog_.handleEvent(mousePos, mousePressed, mouseReleased, game_.phase());
            if (act == ScoringDialogAction::Resume) {
                game_.resumePlay();
                activeDialog_ = DialogType::None;
                banner_.show("Resumed game play", false);
            } else if (act == ScoringDialogAction::Accept) {
                game_.acceptScore();
                activeDialog_ = DialogType::Scoring;
                banner_.show("Game finished. Result accepted.", false);
            } else if (act == ScoringDialogAction::NewGame) {
                activeDialog_ = DialogType::NewGame;
                newGameDialog_.show(game_.config());
            }
        }
        if (closed) {
            activeDialog_ = DialogType::None;
        }
        return;
    }

    hoverPoint_ = boardView_.screenToPoint(mousePos);

    if (mouseReleased && hoverPoint_) {
        handleBoardClick(*hoverPoint_);
    }

    const bool aiThinking = aiPlayer_.isThinking();
    passButton_.setEnabled(!aiThinking && game_.phase() == Phase::Playing);
    undoButton_.setEnabled(!aiThinking && game_.canUndo());
    redoButton_.setEnabled(!aiThinking && game_.canRedo());
    resetButton_.setEnabled(!aiThinking);

    if (passButton_.handleEvent(mousePos, mousePressed, mouseReleased)) {
        onPass();
    }
    if (undoButton_.handleEvent(mousePos, mousePressed, mouseReleased)) {
        onUndo();
    }
    if (redoButton_.handleEvent(mousePos, mousePressed, mouseReleased)) {
        onRedo();
    }
    if (resetButton_.handleEvent(mousePos, mousePressed, mouseReleased)) {
        onReset();
    }
    if (newGameButton_.handleEvent(mousePos, mousePressed, mouseReleased)) {
        activeDialog_ = DialogType::NewGame;
        newGameDialog_.show(game_.config());
    }
    if (saveButton_.handleEvent(mousePos, mousePressed, mouseReleased)) {
        activeDialog_ = DialogType::Save;
        saveDialog_.show("game_" + std::to_string(game_.moves().size()) + "moves");
    }
    if (loadButton_.handleEvent(mousePos, mousePressed, mouseReleased)) {
        activeDialog_ = DialogType::Load;
        loadDialog_.show();
    }
    if (settingsButton_.handleEvent(mousePos, mousePressed, mouseReleased)) {
        activeDialog_ = DialogType::Settings;
        settingsDialog_.show(settings_);
    }
    if (exitButton_.handleEvent(mousePos, mousePressed, mouseReleased)) {
        window_.close();
    }
}

void UI::handleBoardClick(Point point)
{
    if (game_.phase() == Phase::Scoring) {
        if (game_.toggleDeadGroup(point)) {
            audioManager_.playEffect(SoundEffect::Place);
            banner_.show(game_.isDead(point) ? "Group marked as dead" : "Group marked as living", false);
        }
        return;
    }

    if (game_.phase() != Phase::Playing || game_.isAITurn() || aiPlayer_.isThinking()) {
        return;
    }

    const Move move = Move::place(point);
    const MoveReport report = game_.play(move);
    if (!report.ok()) {
        audioManager_.playEffect(SoundEffect::Error);
        banner_.show(describeMoveError(report.error), true);
        return;
    }

    if (!report.captured.empty()) {
        audioManager_.playEffect(SoundEffect::Capture);
        banner_.show(stoneName(report.player) + " captured " + std::to_string(report.captured.size()) + " stone(s)!", false);
    } else {
        audioManager_.playEffect(SoundEffect::Place);
    }

    if (report.enteredScoring) {
        audioManager_.playEffect(SoundEffect::Pass);
        activeDialog_ = DialogType::Scoring;
        scoringDialog_.show();
        banner_.show("Two passes. Entering scoring review.", false);
        return;
    }

    triggerAIMoveIfNeeded();
}

void UI::triggerAIMoveIfNeeded()
{
    if (game_.isAITurn() && !aiPlayer_.isThinking()) {
        aiPlayer_.start(game_.position(), game_.config().difficulty, game_.config().komi);
    }
}

void UI::onPass()
{
    if (game_.phase() != Phase::Playing || game_.isAITurn() || aiPlayer_.isThinking()) {
        return;
    }

    const MoveReport report = game_.play(Move::pass());
    audioManager_.playEffect(SoundEffect::Pass);

    if (report.enteredScoring) {
        activeDialog_ = DialogType::Scoring;
        scoringDialog_.show();
        banner_.show("Two consecutive passes. Scoring review started.", false);
    } else {
        banner_.show(stoneName(report.player) + " passed their turn", false);
        triggerAIMoveIfNeeded();
    }
}

void UI::onUndo()
{
    aiPlayer_.cancel();
    const int count = game_.undoTurn();
    if (count > 0) {
        banner_.show("Undid " + std::to_string(count) + " move(s)", false);
        audioManager_.playEffect(SoundEffect::Place);
    }
}

void UI::onRedo()
{
    aiPlayer_.cancel();
    const int count = game_.redoTurn();
    if (count > 0) {
        banner_.show("Redid " + std::to_string(count) + " move(s)", false);
        audioManager_.playEffect(SoundEffect::Place);
        triggerAIMoveIfNeeded();
    }
}

void UI::onReset()
{
    aiPlayer_.cancel();
    game_.reset();
    activeDialog_ = DialogType::None;
    banner_.show("Game reset to opening position", false);
    triggerAIMoveIfNeeded();
}

void UI::onNewGame(const GameConfig& config)
{
    aiPlayer_.cancel();
    game_ = Game(config);
    activeDialog_ = DialogType::None;
    updateLayout();
    banner_.show("Started new game (" + std::to_string(config.boardSize) + "x" + std::to_string(config.boardSize) + ")", false);
    triggerAIMoveIfNeeded();
}

void UI::onSaveGame(const std::filesystem::path& path)
{
    std::string err;
    if (SaveManager::saveToFile(game_, path, err)) {
        banner_.show("Game successfully saved to " + path.filename().string(), false);
    } else {
        banner_.show("Failed to save: " + err, true);
    }
}

void UI::onLoadGame(const std::filesystem::path& path)
{
    aiPlayer_.cancel();
    SaveManager::LoadResult res = SaveManager::loadFromFile(path);
    if (res.game) {
        game_ = std::move(*res.game);
        updateLayout();
        banner_.show("Loaded save: " + path.filename().string(), false);
        if (game_.phase() == Phase::Scoring || game_.phase() == Phase::Finished) {
            activeDialog_ = DialogType::Scoring;
            scoringDialog_.show();
        } else {
            triggerAIMoveIfNeeded();
        }
    } else {
        banner_.show("Failed to load: " + res.error, true);
    }
}

void UI::onApplySettings(const Settings& settings)
{
    settings_ = settings;
    audioManager_.applySettings(settings_);
    saveSettings(settings_, settingsFile());
    banner_.show("Settings applied and saved", false);
}

void UI::update(float dt)
{
    banner_.update(dt);

    if (aiPlayer_.isThinking()) {
        auto decision = aiPlayer_.takeDecision();
        if (decision) {
            const MoveReport report = game_.play(decision->move);
            if (report.move.isPass()) {
                audioManager_.playEffect(SoundEffect::Pass);
                banner_.show("AI passed (" + difficultyName(game_.config().difficulty) + ")", false);
            } else if (!report.captured.empty()) {
                audioManager_.playEffect(SoundEffect::Capture);
                banner_.show("AI played " + moveToText(report.move, game_.board().size()) + " and captured " + std::to_string(report.captured.size()) + " stone(s)", false);
            } else {
                audioManager_.playEffect(SoundEffect::Place);
                banner_.show("AI played " + moveToText(report.move, game_.board().size()), false);
            }

            if (report.enteredScoring) {
                activeDialog_ = DialogType::Scoring;
                scoringDialog_.show();
            }
        }
    }
}

void UI::render()
{
    window_.clear(sf::Color(18, 22, 28));

    boardView_.render(window_, game_, settings_, hoverPoint_);

    sf::RectangleShape panelBg(sidePanelBounds_.size);
    panelBg.setPosition(sidePanelBounds_.position);
    panelBg.setFillColor(sf::Color(25, 29, 37));
    panelBg.setOutlineColor(sf::Color(45, 52, 65));
    panelBg.setOutlineThickness(1.5f);
    window_.draw(panelBg);

    const float px = sidePanelBounds_.position.x + 20.0f;
    const float pw = sidePanelBounds_.size.x - 40.0f;

    sf::Text appTitle(fontSemiBold_, "GO GAME", 22);
    appTitle.setPosition({px, 20.0f});
    appTitle.setFillColor(sf::Color(245, 248, 255));
    window_.draw(appTitle);

    std::string modeSub = (game_.config().mode == GameMode::PlayerVsAI)
        ? ("vs AI (" + difficultyName(game_.config().difficulty) + ")")
        : "Player vs Player (Local)";
    sf::Text modeText(fontRegular_, modeSub, 13);
    modeText.setPosition({px, 48.0f});
    modeText.setFillColor(sf::Color(140, 155, 175));
    window_.draw(modeText);

    sf::RectangleShape turnCard({pw, 88.0f});
    turnCard.setPosition({px, 78.0f});
    turnCard.setFillColor(sf::Color(34, 40, 52));
    turnCard.setOutlineColor(sf::Color(55, 65, 82));
    turnCard.setOutlineThickness(1.2f);
    window_.draw(turnCard);

    const Stone toMove = game_.toMove();
    drawStone(window_, {px + 28.0f, 78.0f + 44.0f}, 16.0f, toMove, settings_.stoneStyle);

    std::string turnLabel = (game_.phase() == Phase::Scoring) ? "Scoring Review" : ((game_.phase() == Phase::Finished) ? "Game Completed" : (stoneName(toMove) + "'s Turn"));
    sf::Text turnText(fontSemiBold_, turnLabel, 16);
    turnText.setPosition({px + 56.0f, 88.0f});
    turnText.setFillColor(sf::Color(250, 252, 255));
    window_.draw(turnText);

    std::string roleDesc = game_.playerLabel(toMove);
    if (aiPlayer_.isThinking()) {
        roleDesc = "AI is thinking...";
    }
    sf::Text roleText(fontRegular_, roleDesc, 13);
    roleText.setPosition({px + 56.0f, 114.0f});
    roleText.setFillColor(aiPlayer_.isThinking() ? sf::Color(100, 180, 255) : sf::Color(160, 175, 195));
    window_.draw(roleText);

    sf::RectangleShape capCard({pw, 74.0f});
    capCard.setPosition({px, 178.0f});
    capCard.setFillColor(sf::Color(30, 35, 45));
    capCard.setOutlineColor(sf::Color(50, 58, 72));
    capCard.setOutlineThickness(1.0f);
    window_.draw(capCard);

    sf::Text capTitle(fontRegular_, "Captured Stones", 12);
    capTitle.setPosition({px + 14.0f, 186.0f});
    capTitle.setFillColor(sf::Color(140, 150, 168));
    window_.draw(capTitle);

    drawStone(window_, {px + 24.0f, 226.0f}, 9.0f, Stone::Black, 0);
    sf::Text bCap(fontSemiBold_, "Black: " + std::to_string(game_.position().capturesBy(Stone::Black)), 14);
    bCap.setPosition({px + 40.0f, 216.0f});
    bCap.setFillColor(sf::Color(230, 235, 245));
    window_.draw(bCap);

    drawStone(window_, {px + pw * 0.52f + 10.0f, 226.0f}, 9.0f, Stone::White, 0);
    sf::Text wCap(fontSemiBold_, "White: " + std::to_string(game_.position().capturesBy(Stone::White)), 14);
    wCap.setPosition({px + pw * 0.52f + 26.0f, 216.0f});
    wCap.setFillColor(sf::Color(230, 235, 245));
    window_.draw(wCap);

    passButton_.render(window_);
    undoButton_.render(window_);
    redoButton_.render(window_);
    resetButton_.render(window_);
    newGameButton_.render(window_);
    saveButton_.render(window_);
    loadButton_.render(window_);
    settingsButton_.render(window_);
    exitButton_.render(window_);

    const float histY = 560.0f;
    const float histH = sidePanelBounds_.size.y - histY - 20.0f;
    if (histH > 60.0f) {
        sf::RectangleShape histBox({pw, histH});
        histBox.setPosition({px, histY});
        histBox.setFillColor(sf::Color(22, 26, 33));
        histBox.setOutlineColor(sf::Color(44, 52, 65));
        histBox.setOutlineThickness(1.0f);
        window_.draw(histBox);

        sf::Text histTitle(fontRegular_, "Recent Moves (" + std::to_string(game_.moves().size()) + ")", 12);
        histTitle.setPosition({px + 12.0f, histY + 8.0f});
        histTitle.setFillColor(sf::Color(140, 155, 175));
        window_.draw(histTitle);

        const auto& moves = game_.moves();
        const int maxLines = static_cast<int>((histH - 36.0f) / 22.0f);
        const int startIdx = std::max(0, static_cast<int>(moves.size()) - maxLines);

        for (int i = startIdx; i < static_cast<int>(moves.size()); ++i) {
            const int lineNum = i - startIdx;
            const Stone moverColor = (i % 2 == 0) ? Stone::Black : Stone::White;
            const std::string moveText = std::to_string(i + 1) + ". " + stoneName(moverColor) + " " + moveToText(moves[i], game_.board().size());

            sf::Text line(fontRegular_, moveText, 13);
            line.setPosition({px + 14.0f, histY + 32.0f + static_cast<float>(lineNum) * 22.0f});
            line.setFillColor((i == static_cast<int>(moves.size()) - 1) ? sf::Color(120, 190, 255) : sf::Color(190, 200, 215));
            window_.draw(line);
        }
    }

    banner_.render(window_, static_cast<float>(window_.getSize().x));

    const sf::Vector2u winSize = window_.getSize();
    if (activeDialog_ == DialogType::NewGame) {
        newGameDialog_.render(window_, winSize);
    } else if (activeDialog_ == DialogType::Save) {
        saveDialog_.render(window_, winSize);
    } else if (activeDialog_ == DialogType::Load) {
        loadDialog_.render(window_, winSize);
    } else if (activeDialog_ == DialogType::Settings) {
        settingsDialog_.render(window_, winSize);
    } else if (activeDialog_ == DialogType::Scoring) {
        scoringDialog_.render(window_, winSize, game_);
    }

    window_.display();
}
