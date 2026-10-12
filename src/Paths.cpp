#include "Paths.h"

#include <system_error>

namespace {

std::filesystem::path findDataRoot()
{
    std::error_code error;
    std::filesystem::path candidate = std::filesystem::current_path(error);
    for (int level = 0; level < 4 && !candidate.empty(); ++level) {
        if (std::filesystem::is_directory(candidate / "assets", error)) {
            return candidate;
        }
        const std::filesystem::path parent = candidate.parent_path();
        if (parent == candidate) {
            break;
        }
        candidate = parent;
    }
#ifdef GO_SOURCE_DIR
    if (std::filesystem::is_directory(std::filesystem::path(GO_SOURCE_DIR) / "assets", error)) {
        return std::filesystem::path(GO_SOURCE_DIR);
    }
#endif
    return std::filesystem::current_path(error);
}

}

std::filesystem::path dataRoot()
{
    static const std::filesystem::path root = findDataRoot();
    return root;
}

std::filesystem::path assetPath(const std::string& relativePath)
{
    return dataRoot() / "assets" / relativePath;
}

std::filesystem::path savesDirectory()
{
    return dataRoot() / "saves";
}

std::filesystem::path settingsFile()
{
    return dataRoot() / "settings.cfg";
}
