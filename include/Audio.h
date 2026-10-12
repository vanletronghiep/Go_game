#pragma once

#include "Settings.h"

#include <SFML/Audio.hpp>
#include <string>
#include <vector>

enum class SoundEffect {
    Place,
    Capture,
    Error,
    Pass
};

class AudioManager {
public:
    AudioManager();

    void applySettings(const Settings& settings);
    void playEffect(SoundEffect effect);
    void update();

private:
    void reloadThemeSounds(int themeIndex);
    void switchMusicTrack(int trackIndex);

    Settings settings_;
    sf::SoundBuffer placeBuffer_;
    sf::SoundBuffer captureBuffer_;
    sf::SoundBuffer errorBuffer_;
    sf::SoundBuffer passBuffer_;

    sf::Sound placeSound_{placeBuffer_};
    sf::Sound captureSound_{captureBuffer_};
    sf::Sound errorSound_{errorBuffer_};
    sf::Sound passSound_{passBuffer_};

    sf::Music music_;
    int currentTheme_ = -1;
    int currentTrack_ = -1;
};
