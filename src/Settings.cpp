#include "Settings.h"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace {

int clampIndex(int value, const std::vector<std::string>& options)
{
    return std::clamp(value, 0, static_cast<int>(options.size()) - 1);
}

template <typename T>
void readValue(const std::string& text, T& target)
{
    std::istringstream stream(text);
    T value{};
    if (stream >> value) {
        target = value;
    }
}

}

const std::vector<std::string>& boardThemeNames()
{
    static const std::vector<std::string> names{"Kaya Wood", "Bamboo", "Walnut", "Slate Blue", "Jade"};
    return names;
}

const std::vector<std::string>& stoneStyleNames()
{
    static const std::vector<std::string> names{"Classic", "Modern", "Cartoon"};
    return names;
}

const std::vector<std::string>& soundThemeNames()
{
    static const std::vector<std::string> names{"Wooden", "Glass", "Retro"};
    return names;
}

const std::vector<std::string>& musicTrackNames()
{
    static const std::vector<std::string> names{"Zen Garden", "Night Rain", "Mountain Stream"};
    return names;
}

Settings loadSettings(const std::filesystem::path& path)
{
    Settings settings;
    std::ifstream file(path);
    std::string line;
    while (std::getline(file, line)) {
        const auto separator = line.find('=');
        if (separator == std::string::npos) {
            continue;
        }
        const std::string key = line.substr(0, separator);
        const std::string value = line.substr(separator + 1);
        if (key == "board_theme") {
            readValue(value, settings.boardTheme);
        } else if (key == "stone_style") {
            readValue(value, settings.stoneStyle);
        } else if (key == "sound_enabled") {
            readValue(value, settings.soundEnabled);
        } else if (key == "sound_theme") {
            readValue(value, settings.soundTheme);
        } else if (key == "sound_volume") {
            readValue(value, settings.soundVolume);
        } else if (key == "music_enabled") {
            readValue(value, settings.musicEnabled);
        } else if (key == "music_track") {
            readValue(value, settings.musicTrack);
        } else if (key == "music_volume") {
            readValue(value, settings.musicVolume);
        } else if (key == "show_coordinates") {
            readValue(value, settings.showCoordinates);
        }
    }
    settings.boardTheme = clampIndex(settings.boardTheme, boardThemeNames());
    settings.stoneStyle = clampIndex(settings.stoneStyle, stoneStyleNames());
    settings.soundTheme = clampIndex(settings.soundTheme, soundThemeNames());
    settings.musicTrack = clampIndex(settings.musicTrack, musicTrackNames());
    settings.soundVolume = std::clamp(settings.soundVolume, 0.0f, 1.0f);
    settings.musicVolume = std::clamp(settings.musicVolume, 0.0f, 1.0f);
    return settings;
}

bool saveSettings(const Settings& settings, const std::filesystem::path& path)
{
    std::ofstream file(path, std::ios::trunc);
    if (!file) {
        return false;
    }
    file << "board_theme=" << settings.boardTheme << '\n'
         << "stone_style=" << settings.stoneStyle << '\n'
         << "sound_enabled=" << settings.soundEnabled << '\n'
         << "sound_theme=" << settings.soundTheme << '\n'
         << "sound_volume=" << settings.soundVolume << '\n'
         << "music_enabled=" << settings.musicEnabled << '\n'
         << "music_track=" << settings.musicTrack << '\n'
         << "music_volume=" << settings.musicVolume << '\n'
         << "show_coordinates=" << settings.showCoordinates << '\n';
    return static_cast<bool>(file);
}
