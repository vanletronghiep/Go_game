#pragma once

#include <filesystem>
#include <string>

std::filesystem::path dataRoot();
std::filesystem::path assetPath(const std::string& relativePath);
std::filesystem::path savesDirectory();
std::filesystem::path settingsFile();
