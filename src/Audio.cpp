#include "Audio.h"

#include "Paths.h"

#include <algorithm>
#include <filesystem>

namespace {

std::string themeDirectoryName(int themeIndex)
{
    switch (themeIndex) {
    case 1:
        return "glass";
    case 2:
        return "retro";
    case 0:
    default:
        return "wooden";
    }
}

std::string musicFileName(int trackIndex)
{
    switch (trackIndex) {
    case 1:
        return "night_rain.wav";
    case 2:
        return "mountain_stream.wav";
    case 0:
    default:
        return "zen_garden.wav";
    }
}

}

AudioManager::AudioManager()
{
    reloadThemeSounds(0);
    switchMusicTrack(0);
}

void AudioManager::applySettings(const Settings& settings)
{
    const bool themeChanged = (settings.soundTheme != currentTheme_);
    const bool trackChanged = (settings.musicTrack != currentTrack_);
    const bool musicToggled = (settings.musicEnabled != settings_.musicEnabled);

    settings_ = settings;

    const float sVol = std::clamp(settings_.soundVolume * 100.0f, 0.0f, 100.0f);
    placeSound_.setVolume(sVol);
    captureSound_.setVolume(sVol);
    errorSound_.setVolume(sVol);
    passSound_.setVolume(sVol);

    const float mVol = std::clamp(settings_.musicVolume * 100.0f, 0.0f, 100.0f);
    music_.setVolume(mVol);

    if (themeChanged) {
        reloadThemeSounds(settings_.soundTheme);
    }

    if (trackChanged) {
        switchMusicTrack(settings_.musicTrack);
    } else if (musicToggled) {
        if (settings_.musicEnabled) {
            if (music_.getStatus() != sf::SoundStream::Status::Playing) {
                music_.play();
            }
        } else {
            music_.pause();
        }
    }
}

void AudioManager::reloadThemeSounds(int themeIndex)
{
    currentTheme_ = themeIndex;
    const std::string dir = themeDirectoryName(themeIndex);
    const auto basePath = assetPath("audio/themes/" + dir);

    const auto pPlace = basePath / "place.wav";
    if (std::filesystem::exists(pPlace)) {
        if (placeBuffer_.loadFromFile(pPlace)) {
            placeSound_ = sf::Sound(placeBuffer_);
        }
    }

    const auto pCapture = basePath / "capture.wav";
    if (std::filesystem::exists(pCapture)) {
        if (captureBuffer_.loadFromFile(pCapture)) {
            captureSound_ = sf::Sound(captureBuffer_);
        }
    }

    const auto pError = basePath / "error.wav";
    if (std::filesystem::exists(pError)) {
        if (errorBuffer_.loadFromFile(pError)) {
            errorSound_ = sf::Sound(errorBuffer_);
        }
    }

    const auto pPass = basePath / "pass.wav";
    if (std::filesystem::exists(pPass)) {
        if (passBuffer_.loadFromFile(pPass)) {
            passSound_ = sf::Sound(passBuffer_);
        }
    }

    const float sVol = std::clamp(settings_.soundVolume * 100.0f, 0.0f, 100.0f);
    placeSound_.setVolume(sVol);
    captureSound_.setVolume(sVol);
    errorSound_.setVolume(sVol);
    passSound_.setVolume(sVol);
}

void AudioManager::switchMusicTrack(int trackIndex)
{
    currentTrack_ = trackIndex;
    const std::string fname = musicFileName(trackIndex);
    const auto pMusic = assetPath("audio/music/" + fname);

    music_.stop();
    if (std::filesystem::exists(pMusic)) {
        if (music_.openFromFile(pMusic)) {
            music_.setLooping(true);
            music_.setVolume(std::clamp(settings_.musicVolume * 100.0f, 0.0f, 100.0f));
            if (settings_.musicEnabled) {
                music_.play();
            }
        }
    }
}

void AudioManager::playEffect(SoundEffect effect)
{
    if (!settings_.soundEnabled) {
        return;
    }
    switch (effect) {
    case SoundEffect::Place:
        placeSound_.play();
        break;
    case SoundEffect::Capture:
        captureSound_.play();
        break;
    case SoundEffect::Error:
        errorSound_.play();
        break;
    case SoundEffect::Pass:
        passSound_.play();
        break;
    }
}

void AudioManager::update()
{
}
