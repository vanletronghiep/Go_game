#pragma once

#include <filesystem>
#include <string>
#include <vector>

struct Settings {
    int boardTheme = 0;
    int stoneStyle = 0;
    bool soundEnabled = true;
    int soundTheme = 0;
    float soundVolume = 0.7f;
    bool musicEnabled = true;
    int musicTrack = 0;
    float musicVolume = 0.35f;
    bool showCoordinates = true;
};

const std::vector<std::string>& boardThemeNames();
const std::vector<std::string>& stoneStyleNames();
const std::vector<std::string>& soundThemeNames();
const std::vector<std::string>& musicTrackNames();

Settings loadSettings(const std::filesystem::path& path);
bool saveSettings(const Settings& settings, const std::filesystem::path& path);
