#pragma once

#include "Game.h"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace SaveManager {

constexpr int formatVersion = 1;
constexpr const char* fileExtension = ".gosave";

struct LoadResult {
    std::optional<Game> game;
    std::string error;
};

std::string serialize(const Game& game);
LoadResult deserialize(const std::string& text);

bool saveToFile(const Game& game, const std::filesystem::path& path, std::string& error);
LoadResult loadFromFile(const std::filesystem::path& path);
std::vector<std::filesystem::path> listSaves(const std::filesystem::path& directory);
std::string sanitizeFileName(const std::string& name);

}
