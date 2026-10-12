#include "Screens.h"

#include "Paths.h"
#include "SaveManager.h"

#include <algorithm>
#include <iomanip>
#include <sstream>

namespace {

void drawModalBackdrop(sf::RenderTarget& target, sf::Vector2u windowSize)
{
    sf::RectangleShape backdrop(sf::Vector2f(static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)));
    backdrop.setFillColor(sf::Color(10, 14, 20, 205));
    target.draw(backdrop);
}

void drawModalFrame(sf::RenderTarget& target, const sf::FloatRect& bounds, const std::string& title,
    const sf::Font& font)
{
    sf::RectangleShape shadow(bounds.size);
    shadow.setPosition({bounds.position.x + 8.0f, bounds.position.y + 10.0f});
    shadow.setFillColor(sf::Color(0, 0, 0, 90));
    target.draw(shadow);

    sf::RectangleShape frame(bounds.size);
    frame.setPosition(bounds.position);
    frame.setFillColor(sf::Color(28, 32, 42));
    frame.setOutlineColor(sf::Color(70, 80, 100));
    frame.setOutlineThickness(2.0f);
    target.draw(frame);

    const float headerH = 46.0f;
    sf::RectangleShape header({bounds.size.x, headerH});
    header.setPosition(bounds.position);
    header.setFillColor(sf::Color(36, 42, 54));
    target.draw(header);

    sf::Text titleText(font, title, 18);
    titleText.setFillColor(sf::Color(245, 248, 255));
    const sf::FloatRect tb = titleText.getLocalBounds();
    titleText.setPosition({bounds.position.x + 20.0f, bounds.position.y + (headerH - tb.size.y) * 0.5f - tb.position.y});
    target.draw(titleText);
}

}

NewGameDialog::NewGameDialog(const sf::Font& font)
    : font_(font)
    , modeControl_({"2 Players (PvP)", "Player vs AI"}, {}, 0, font, 14)
    , difficultyControl_({"Easy", "Medium", "Hard"}, {}, 1, font, 14)
    , sizeControl_({"9 x 9", "13 x 13", "19 x 19"}, {}, 2, font, 14)
    , colorControl_({"Play Black (first)", "Play White"}, {}, 0, font, 14)
    , startButton_("Start Game", {}, font, 15)
    , cancelButton_("Cancel", {}, font, 15)
{
    startButton_.setAccent(true);
}

void NewGameDialog::show(const GameConfig& currentConfig)
{
    modeIndex_ = (currentConfig.mode == GameMode::PlayerVsAI) ? 1 : 0;
    modeControl_.setSelectedIndex(modeIndex_);

    difficultyIndex_ = static_cast<int>(currentConfig.difficulty);
    difficultyControl_.setSelectedIndex(difficultyIndex_);

    if (currentConfig.boardSize == 9) {
        boardSizeIndex_ = 0;
    } else if (currentConfig.boardSize == 13) {
        boardSizeIndex_ = 1;
    } else {
        boardSizeIndex_ = 2;
    }
    sizeControl_.setSelectedIndex(boardSizeIndex_);

    colorIndex_ = (currentConfig.humanColor == Stone::White) ? 1 : 0;
    colorControl_.setSelectedIndex(colorIndex_);
}

void NewGameDialog::updateLayout(sf::Vector2u windowSize)
{
    const float w = 480.0f;
    const float h = 420.0f;
    const float x = (static_cast<float>(windowSize.x) - w) * 0.5f;
    const float y = (static_cast<float>(windowSize.y) - h) * 0.5f;
    bounds_ = sf::FloatRect({x, y}, {w, h});

    const float padX = 30.0f;
    const float contentW = w - padX * 2.0f;

    modeControl_.setBounds(sf::FloatRect({x + padX, y + 80.0f}, {contentW, 36.0f}));
    difficultyControl_.setBounds(sf::FloatRect({x + padX, y + 155.0f}, {contentW, 36.0f}));
    colorControl_.setBounds(sf::FloatRect({x + padX, y + 225.0f}, {contentW, 36.0f}));
    sizeControl_.setBounds(sf::FloatRect({x + padX, y + 295.0f}, {contentW, 36.0f}));

    const float btnW = (contentW - 16.0f) * 0.5f;
    cancelButton_.setBounds(sf::FloatRect({x + padX, y + 355.0f}, {btnW, 40.0f}));
    startButton_.setBounds(sf::FloatRect({x + padX + btnW + 16.0f, y + 355.0f}, {btnW, 40.0f}));
}

std::optional<NewGameResult> NewGameDialog::handleEvent(const sf::Vector2f& mousePos,
    bool mousePressed, bool mouseReleased, bool& closed)
{
    if (modeControl_.handleEvent(mousePos, mouseReleased)) {
        modeIndex_ = modeControl_.getSelectedIndex();
    }
    if (modeIndex_ == 1) {
        if (difficultyControl_.handleEvent(mousePos, mouseReleased)) {
            difficultyIndex_ = difficultyControl_.getSelectedIndex();
        }
        if (colorControl_.handleEvent(mousePos, mouseReleased)) {
            colorIndex_ = colorControl_.getSelectedIndex();
        }
    }
    if (sizeControl_.handleEvent(mousePos, mouseReleased)) {
        boardSizeIndex_ = sizeControl_.getSelectedIndex();
    }

    if (cancelButton_.handleEvent(mousePos, mousePressed, mouseReleased)) {
        closed = true;
        return std::nullopt;
    }

    if (startButton_.handleEvent(mousePos, mousePressed, mouseReleased)) {
        closed = true;
        GameConfig config;
        config.mode = (modeIndex_ == 1) ? GameMode::PlayerVsAI : GameMode::PlayerVsPlayer;
        config.difficulty = static_cast<Difficulty>(difficultyIndex_);
        config.humanColor = (colorIndex_ == 1) ? Stone::White : Stone::Black;
        if (boardSizeIndex_ == 0) {
            config.boardSize = 9;
        } else if (boardSizeIndex_ == 1) {
            config.boardSize = 13;
        } else {
            config.boardSize = 19;
        }
        return NewGameResult{config};
    }

    return std::nullopt;
}

void NewGameDialog::render(sf::RenderTarget& target, sf::Vector2u windowSize) const
{
    drawModalBackdrop(target, windowSize);
    drawModalFrame(target, bounds_, "Start New Game", font_);

    sf::Text modeLbl(font_, "Game Mode", 14);
    modeLbl.setPosition({bounds_.position.x + 30.0f, bounds_.position.y + 58.0f});
    modeLbl.setFillColor(sf::Color(180, 190, 205));
    target.draw(modeLbl);
    modeControl_.render(target);

    if (modeIndex_ == 1) {
        sf::Text diffLbl(font_, "AI Difficulty", 14);
        diffLbl.setPosition({bounds_.position.x + 30.0f, bounds_.position.y + 132.0f});
        diffLbl.setFillColor(sf::Color(180, 190, 205));
        target.draw(diffLbl);
        difficultyControl_.render(target);

        sf::Text colLbl(font_, "Your Side", 14);
        colLbl.setPosition({bounds_.position.x + 30.0f, bounds_.position.y + 202.0f});
        colLbl.setFillColor(sf::Color(180, 190, 205));
        target.draw(colLbl);
        colorControl_.render(target);
    }

    sf::Text sizeLbl(font_, "Board Dimensions", 14);
    sizeLbl.setPosition({bounds_.position.x + 30.0f, bounds_.position.y + 272.0f});
    sizeLbl.setFillColor(sf::Color(180, 190, 205));
    target.draw(sizeLbl);
    sizeControl_.render(target);

    cancelButton_.render(target);
    startButton_.render(target);
}

SaveDialog::SaveDialog(const sf::Font& font)
    : font_(font)
    , nameInput_("my_game", {}, font, 15)
    , saveButton_("Save Game", {}, font, 15)
    , cancelButton_("Cancel", {}, font, 15)
{
    saveButton_.setAccent(true);
}

void SaveDialog::show(const std::string& defaultName)
{
    nameInput_.setText(defaultName.empty() ? "my_game" : defaultName);
}

void SaveDialog::updateLayout(sf::Vector2u windowSize)
{
    const float w = 440.0f;
    const float h = 230.0f;
    const float x = (static_cast<float>(windowSize.x) - w) * 0.5f;
    const float y = (static_cast<float>(windowSize.y) - h) * 0.5f;
    bounds_ = sf::FloatRect({x, y}, {w, h});

    nameInput_.setBounds(sf::FloatRect({x + 30.0f, y + 95.0f}, {w - 60.0f, 38.0f}));

    const float btnW = (w - 76.0f) * 0.5f;
    cancelButton_.setBounds(sf::FloatRect({x + 30.0f, y + 160.0f}, {btnW, 38.0f}));
    saveButton_.setBounds(sf::FloatRect({x + 30.0f + btnW + 16.0f, y + 160.0f}, {btnW, 38.0f}));
}

std::optional<SaveLoadResult> SaveDialog::handleEvent(const sf::Vector2f& mousePos,
    bool mousePressed, bool mouseReleased, char32_t unicode, bool& closed)
{
    nameInput_.handleEvent(mousePos, mousePressed, unicode);

    if (cancelButton_.handleEvent(mousePos, mousePressed, mouseReleased)) {
        closed = true;
        return std::nullopt;
    }

    if (saveButton_.handleEvent(mousePos, mousePressed, mouseReleased)) {
        std::string raw = nameInput_.getText();
        if (raw.empty()) {
            raw = "game";
        }
        std::string safeName = SaveManager::sanitizeFileName(raw);
        if (safeName.empty()) {
            safeName = "game";
        }
        const auto savePath = savesDirectory() / (safeName + std::string(SaveManager::fileExtension));
        closed = true;
        return SaveLoadResult{true, savePath};
    }

    return std::nullopt;
}

void SaveDialog::render(sf::RenderTarget& target, sf::Vector2u windowSize) const
{
    drawModalBackdrop(target, windowSize);
    drawModalFrame(target, bounds_, "Save Game State", font_);

    sf::Text hint(font_, "Enter file name:", 14);
    hint.setPosition({bounds_.position.x + 30.0f, bounds_.position.y + 68.0f});
    hint.setFillColor(sf::Color(180, 190, 205));
    target.draw(hint);

    nameInput_.render(target);
    cancelButton_.render(target);
    saveButton_.render(target);
}

LoadDialog::LoadDialog(const sf::Font& font)
    : font_(font)
    , loadButton_("Load Game", {}, font, 15)
    , cancelButton_("Cancel", {}, font, 15)
{
    loadButton_.setAccent(true);
}

void LoadDialog::show()
{
    files_ = SaveManager::listSaves(savesDirectory());
    selectedIndex_ = files_.empty() ? -1 : 0;
}

void LoadDialog::updateLayout(sf::Vector2u windowSize)
{
    const float w = 520.0f;
    const float h = 380.0f;
    const float x = (static_cast<float>(windowSize.x) - w) * 0.5f;
    const float y = (static_cast<float>(windowSize.y) - h) * 0.5f;
    bounds_ = sf::FloatRect({x, y}, {w, h});

    const float btnW = (w - 76.0f) * 0.5f;
    cancelButton_.setBounds(sf::FloatRect({x + 30.0f, y + 315.0f}, {btnW, 38.0f}));
    loadButton_.setBounds(sf::FloatRect({x + 30.0f + btnW + 16.0f, y + 315.0f}, {btnW, 38.0f}));
    loadButton_.setEnabled(selectedIndex_ >= 0);
}

std::optional<SaveLoadResult> LoadDialog::handleEvent(const sf::Vector2f& mousePos,
    bool mousePressed, bool mouseReleased, bool& closed)
{
    const float listY = bounds_.position.y + 65.0f;
    const float itemH = 34.0f;

    if (mouseReleased) {
        for (std::size_t i = 0; i < std::min<std::size_t>(files_.size(), 6); ++i) {
            const sf::FloatRect itemRect(
                {bounds_.position.x + 30.0f, listY + static_cast<float>(i) * (itemH + 4.0f)},
                {bounds_.size.x - 60.0f, itemH}
            );
            if (itemRect.contains(mousePos)) {
                selectedIndex_ = static_cast<int>(i);
                loadButton_.setEnabled(true);
                break;
            }
        }
    }

    if (cancelButton_.handleEvent(mousePos, mousePressed, mouseReleased)) {
        closed = true;
        return std::nullopt;
    }

    if (loadButton_.handleEvent(mousePos, mousePressed, mouseReleased)) {
        if (selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(files_.size())) {
            closed = true;
            return SaveLoadResult{true, files_[selectedIndex_]};
        }
    }

    return std::nullopt;
}

void LoadDialog::render(sf::RenderTarget& target, sf::Vector2u windowSize) const
{
    drawModalBackdrop(target, windowSize);
    drawModalFrame(target, bounds_, "Load Saved Game", font_);

    const float listY = bounds_.position.y + 65.0f;
    const float itemH = 34.0f;

    if (files_.empty()) {
        sf::Text emptyText(font_, "No saved games found in saves/ directory.", 15);
        emptyText.setPosition({bounds_.position.x + 40.0f, bounds_.position.y + 140.0f});
        emptyText.setFillColor(sf::Color(140, 150, 165));
        target.draw(emptyText);
    } else {
        for (std::size_t i = 0; i < std::min<std::size_t>(files_.size(), 6); ++i) {
            const sf::FloatRect itemRect(
                {bounds_.position.x + 30.0f, listY + static_cast<float>(i) * (itemH + 4.0f)},
                {bounds_.size.x - 60.0f, itemH}
            );

            sf::RectangleShape itemBg(itemRect.size);
            itemBg.setPosition(itemRect.position);
            if (static_cast<int>(i) == selectedIndex_) {
                itemBg.setFillColor(sf::Color(55, 110, 195));
                itemBg.setOutlineColor(sf::Color(100, 160, 245));
            } else {
                itemBg.setFillColor(sf::Color(36, 42, 54));
                itemBg.setOutlineColor(sf::Color(60, 70, 88));
            }
            itemBg.setOutlineThickness(1.2f);
            target.draw(itemBg);

            sf::Text fName(font_, files_[i].stem().string(), 14);
            fName.setPosition({itemRect.position.x + 14.0f, itemRect.position.y + 6.0f});
            fName.setFillColor(sf::Color(240, 245, 255));
            target.draw(fName);
        }
    }

    cancelButton_.render(target);
    loadButton_.render(target);
}

SettingsDialog::SettingsDialog(const sf::Font& font)
    : font_(font)
    , themeControl_(boardThemeNames(), {}, 0, font, 13)
    , stoneStyleControl_(stoneStyleNames(), {}, 0, font, 13)
    , soundCheckbox_("Sound Effects Enabled", {}, true, font, 14)
    , soundThemeControl_(soundThemeNames(), {}, 0, font, 13)
    , soundVolumeSlider_("Sound Volume", {}, 0.7f, font, 13)
    , musicCheckbox_("Background Music Enabled", {}, true, font, 14)
    , musicTrackControl_(musicTrackNames(), {}, 0, font, 13)
    , musicVolumeSlider_("Music Volume", {}, 0.35f, font, 13)
    , coordsCheckbox_("Show Coordinates (A-T, 1-19)", {}, true, font, 14)
    , applyButton_("Save & Apply", {}, font, 15)
    , cancelButton_("Cancel", {}, font, 15)
{
    applyButton_.setAccent(true);
}

void SettingsDialog::show(const Settings& current)
{
    current_ = current;
    themeControl_.setSelectedIndex(current_.boardTheme);
    stoneStyleControl_.setSelectedIndex(current_.stoneStyle);
    soundCheckbox_.setChecked(current_.soundEnabled);
    soundThemeControl_.setSelectedIndex(current_.soundTheme);
    soundVolumeSlider_.setValue(current_.soundVolume);
    musicCheckbox_.setChecked(current_.musicEnabled);
    musicTrackControl_.setSelectedIndex(current_.musicTrack);
    musicVolumeSlider_.setValue(current_.musicVolume);
    coordsCheckbox_.setChecked(current_.showCoordinates);
}

void SettingsDialog::updateLayout(sf::Vector2u windowSize)
{
    const float w = 560.0f;
    const float h = 570.0f;
    const float x = (static_cast<float>(windowSize.x) - w) * 0.5f;
    const float y = (static_cast<float>(windowSize.y) - h) * 0.5f;
    bounds_ = sf::FloatRect({x, y}, {w, h});

    const float padX = 30.0f;
    const float contentW = w - padX * 2.0f;

    themeControl_.setBounds(sf::FloatRect({x + padX, y + 80.0f}, {contentW, 32.0f}));
    stoneStyleControl_.setBounds(sf::FloatRect({x + padX, y + 145.0f}, {contentW, 32.0f}));

    soundCheckbox_.setPosition({x + padX, y + 195.0f});
    soundThemeControl_.setBounds(sf::FloatRect({x + padX, y + 230.0f}, {contentW, 32.0f}));
    soundVolumeSlider_.setBounds(sf::FloatRect({x + padX, y + 275.0f}, {contentW, 38.0f}));

    musicCheckbox_.setPosition({x + padX, y + 330.0f});
    musicTrackControl_.setBounds(sf::FloatRect({x + padX, y + 365.0f}, {contentW, 32.0f}));
    musicVolumeSlider_.setBounds(sf::FloatRect({x + padX, y + 410.0f}, {contentW, 38.0f}));

    coordsCheckbox_.setPosition({x + padX, y + 465.0f});

    const float btnW = (contentW - 16.0f) * 0.5f;
    cancelButton_.setBounds(sf::FloatRect({x + padX, y + 505.0f}, {btnW, 38.0f}));
    applyButton_.setBounds(sf::FloatRect({x + padX + btnW + 16.0f, y + 505.0f}, {btnW, 38.0f}));
}

std::optional<Settings> SettingsDialog::handleEvent(const sf::Vector2f& mousePos,
    bool mousePressed, bool mouseReleased, bool& closed)
{
    if (themeControl_.handleEvent(mousePos, mouseReleased)) {
        current_.boardTheme = themeControl_.getSelectedIndex();
    }
    if (stoneStyleControl_.handleEvent(mousePos, mouseReleased)) {
        current_.stoneStyle = stoneStyleControl_.getSelectedIndex();
    }
    if (soundCheckbox_.handleEvent(mousePos, mouseReleased)) {
        current_.soundEnabled = soundCheckbox_.isChecked();
    }
    if (soundThemeControl_.handleEvent(mousePos, mouseReleased)) {
        current_.soundTheme = soundThemeControl_.getSelectedIndex();
    }
    if (soundVolumeSlider_.handleEvent(mousePos, mousePressed, mouseReleased)) {
        current_.soundVolume = soundVolumeSlider_.getValue();
    }
    if (musicCheckbox_.handleEvent(mousePos, mouseReleased)) {
        current_.musicEnabled = musicCheckbox_.isChecked();
    }
    if (musicTrackControl_.handleEvent(mousePos, mouseReleased)) {
        current_.musicTrack = musicTrackControl_.getSelectedIndex();
    }
    if (musicVolumeSlider_.handleEvent(mousePos, mousePressed, mouseReleased)) {
        current_.musicVolume = musicVolumeSlider_.getValue();
    }
    if (coordsCheckbox_.handleEvent(mousePos, mouseReleased)) {
        current_.showCoordinates = coordsCheckbox_.isChecked();
    }

    if (cancelButton_.handleEvent(mousePos, mousePressed, mouseReleased)) {
        closed = true;
        return std::nullopt;
    }

    if (applyButton_.handleEvent(mousePos, mousePressed, mouseReleased)) {
        closed = true;
        return current_;
    }

    return std::nullopt;
}

void SettingsDialog::render(sf::RenderTarget& target, sf::Vector2u windowSize) const
{
    drawModalBackdrop(target, windowSize);
    drawModalFrame(target, bounds_, "Game Settings", font_);

    sf::Text bThemeLbl(font_, "Board Color & Theme", 13);
    bThemeLbl.setPosition({bounds_.position.x + 30.0f, bounds_.position.y + 60.0f});
    bThemeLbl.setFillColor(sf::Color(180, 190, 205));
    target.draw(bThemeLbl);
    themeControl_.render(target);

    sf::Text sStyleLbl(font_, "Stone Design Style", 13);
    sStyleLbl.setPosition({bounds_.position.x + 30.0f, bounds_.position.y + 125.0f});
    sStyleLbl.setFillColor(sf::Color(180, 190, 205));
    target.draw(sStyleLbl);
    stoneStyleControl_.render(target);

    soundCheckbox_.render(target);
    soundThemeControl_.render(target);
    soundVolumeSlider_.render(target);

    musicCheckbox_.render(target);
    musicTrackControl_.render(target);
    musicVolumeSlider_.render(target);

    coordsCheckbox_.render(target);

    cancelButton_.render(target);
    applyButton_.render(target);
}

ScoringDialog::ScoringDialog(const sf::Font& font)
    : font_(font)
    , resumeButton_("Resume Game", {}, font, 14)
    , acceptButton_("Accept Score", {}, font, 14)
    , newGameButton_("New Game", {}, font, 14)
{
    acceptButton_.setAccent(true);
    newGameButton_.setAccent(true);
}

void ScoringDialog::show()
{
}

void ScoringDialog::updateLayout(sf::Vector2u windowSize)
{
    const float w = 500.0f;
    const float h = 340.0f;
    const float x = (static_cast<float>(windowSize.x) - w) * 0.5f;
    const float y = (static_cast<float>(windowSize.y) - h) * 0.5f;
    bounds_ = sf::FloatRect({x, y}, {w, h});

    const float btnW = 135.0f;
    resumeButton_.setBounds(sf::FloatRect({x + 30.0f, y + 270.0f}, {btnW, 38.0f}));
    acceptButton_.setBounds(sf::FloatRect({x + 30.0f + btnW + 15.0f, y + 270.0f}, {btnW, 38.0f}));
    newGameButton_.setBounds(sf::FloatRect({x + 30.0f + (btnW + 15.0f) * 2.0f, y + 270.0f}, {btnW, 38.0f}));
}

ScoringDialogAction ScoringDialog::handleEvent(const sf::Vector2f& mousePos,
    bool mousePressed, bool mouseReleased, Phase currentPhase)
{
    if (currentPhase == Phase::Scoring) {
        if (resumeButton_.handleEvent(mousePos, mousePressed, mouseReleased)) {
            return ScoringDialogAction::Resume;
        }
        if (acceptButton_.handleEvent(mousePos, mousePressed, mouseReleased)) {
            return ScoringDialogAction::Accept;
        }
    } else if (currentPhase == Phase::Finished) {
        if (newGameButton_.handleEvent(mousePos, mousePressed, mouseReleased)) {
            return ScoringDialogAction::NewGame;
        }
    }
    return ScoringDialogAction::None;
}

void ScoringDialog::render(sf::RenderTarget& target, sf::Vector2u windowSize, const Game& game) const
{
    drawModalBackdrop(target, windowSize);
    drawModalFrame(target, bounds_, game.phase() == Phase::Finished ? "Final Result" : "End-Game Scoring Review", font_);

    const ScoreBreakdown score = game.score();

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1);

    sf::Text winnerText(font_, "", 18);
    if (score.winner == Stone::Black) {
        ss << "Black wins by " << score.margin() << " points!";
        winnerText.setFillColor(sf::Color(100, 210, 140));
    } else if (score.winner == Stone::White) {
        ss << "White wins by " << score.margin() << " points!";
        winnerText.setFillColor(sf::Color(100, 210, 140));
    } else {
        ss << "The game ended in a draw!";
        winnerText.setFillColor(sf::Color(220, 220, 100));
    }
    winnerText.setString(ss.str());
    winnerText.setPosition({bounds_.position.x + 30.0f, bounds_.position.y + 60.0f});
    target.draw(winnerText);

    auto drawRow = [&](const std::string& label, const std::string& bVal, const std::string& wVal, float rowY) {
        sf::Text lbl(font_, label, 14);
        lbl.setPosition({bounds_.position.x + 35.0f, rowY});
        lbl.setFillColor(sf::Color(180, 190, 205));
        target.draw(lbl);

        sf::Text b(font_, bVal, 14);
        b.setPosition({bounds_.position.x + 230.0f, rowY});
        b.setFillColor(sf::Color(240, 245, 255));
        target.draw(b);

        sf::Text w(font_, wVal, 14);
        w.setPosition({bounds_.position.x + 370.0f, rowY});
        w.setFillColor(sf::Color(240, 245, 255));
        target.draw(w);
    };

    drawRow("Category", "Black", "White", bounds_.position.y + 100.0f);

    sf::RectangleShape div({bounds_.size.x - 70.0f, 1.0f});
    div.setPosition({bounds_.position.x + 35.0f, bounds_.position.y + 124.0f});
    div.setFillColor(sf::Color(65, 75, 95));
    target.draw(div);

    drawRow("Living Stones", std::to_string(score.blackStones), std::to_string(score.whiteStones), bounds_.position.y + 134.0f);
    drawRow("Territory Area", std::to_string(score.blackTerritory), std::to_string(score.whiteTerritory), bounds_.position.y + 160.0f);

    std::ostringstream komiStream;
    komiStream << std::fixed << std::setprecision(1) << score.komi;
    drawRow("Komi Bonus", "-", "+" + komiStream.str(), bounds_.position.y + 186.0f);

    std::ostringstream bTot, wTot;
    bTot << std::fixed << std::setprecision(1) << score.blackTotal;
    wTot << std::fixed << std::setprecision(1) << score.whiteTotal;
    drawRow("Total Score", bTot.str(), wTot.str(), bounds_.position.y + 214.0f);

    if (game.phase() == Phase::Scoring) {
        resumeButton_.render(target);
        acceptButton_.render(target);
    } else {
        newGameButton_.render(target);
    }
}
