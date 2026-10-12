#include "SaveManager.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <map>
#include <sstream>
#include <system_error>

namespace SaveManager {

namespace {

const char* const header = "GOGAME_SAVE";

struct SaveData {
    std::map<std::string, std::string> fields;
    std::vector<std::string> boardRows;
};

std::string colorToken(Stone stone)
{
    return stone == Stone::White ? "white" : "black";
}

std::optional<Stone> parseColor(const std::string& token)
{
    if (token == "black") {
        return Stone::Black;
    }
    if (token == "white") {
        return Stone::White;
    }
    return std::nullopt;
}

std::string phaseToken(Phase phase)
{
    switch (phase) {
    case Phase::Playing:
        return "playing";
    case Phase::Scoring:
        return "scoring";
    case Phase::Finished:
        return "finished";
    }
    return "playing";
}

std::optional<Phase> parsePhase(const std::string& token)
{
    if (token == "playing") {
        return Phase::Playing;
    }
    if (token == "scoring") {
        return Phase::Scoring;
    }
    if (token == "finished") {
        return Phase::Finished;
    }
    return std::nullopt;
}

std::optional<Difficulty> parseDifficulty(const std::string& token)
{
    if (token == "easy") {
        return Difficulty::Easy;
    }
    if (token == "medium") {
        return Difficulty::Medium;
    }
    if (token == "hard") {
        return Difficulty::Hard;
    }
    return std::nullopt;
}

std::string lowercase(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return text;
}

char stoneSymbol(Stone stone)
{
    switch (stone) {
    case Stone::Black:
        return 'X';
    case Stone::White:
        return 'O';
    default:
        return '.';
    }
}

void writeMoveList(std::ostream& out, const std::string& key, const std::vector<Move>& moves, int size)
{
    out << key << ' ' << moves.size();
    for (const Move& move : moves) {
        out << ' ' << moveToText(move, size);
    }
    out << '\n';
}

std::optional<int> parseInt(const std::string& text)
{
    std::istringstream stream(text);
    int value = 0;
    if (!(stream >> value) || !(stream >> std::ws).eof()) {
        return std::nullopt;
    }
    return value;
}

std::optional<std::vector<std::string>> parseCountedList(const std::string& text)
{
    std::istringstream stream(text);
    int count = 0;
    if (!(stream >> count) || count < 0) {
        return std::nullopt;
    }
    std::vector<std::string> tokens;
    std::string token;
    while (stream >> token) {
        tokens.push_back(token);
    }
    if (static_cast<int>(tokens.size()) != count) {
        return std::nullopt;
    }
    return tokens;
}

std::optional<std::vector<Move>> parseMoves(const std::string& text, int size)
{
    const auto tokens = parseCountedList(text);
    if (!tokens) {
        return std::nullopt;
    }
    std::vector<Move> moves;
    for (const std::string& token : *tokens) {
        const auto move = parseMoveText(token, size);
        if (!move) {
            return std::nullopt;
        }
        moves.push_back(*move);
    }
    return moves;
}

bool readSaveData(const std::string& text, SaveData& data, std::string& error)
{
    std::istringstream stream(text);
    std::string line;
    if (!std::getline(stream, line)) {
        error = "The file is empty";
        return false;
    }
    std::istringstream headerLine(line);
    std::string magic;
    int version = 0;
    if (!(headerLine >> magic >> version) || magic != header) {
        error = "Not a Go save file";
        return false;
    }
    if (version != formatVersion) {
        error = "Unsupported save version " + std::to_string(version);
        return false;
    }

    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty()) {
            continue;
        }
        if (line == "board") {
            while (std::getline(stream, line)) {
                if (!line.empty() && line.back() == '\r') {
                    line.pop_back();
                }
                if (line == "end") {
                    break;
                }
                data.boardRows.push_back(line);
            }
            continue;
        }
        const auto split = line.find(' ');
        const std::string key = line.substr(0, split);
        data.fields[key] = split == std::string::npos ? "" : line.substr(split + 1);
    }

    for (const char* required : {"size", "komi", "mode", "difficulty", "human", "phase", "to_move", "passes",
             "captures", "moves", "redo", "dead"}) {
        if (!data.fields.contains(required)) {
            error = std::string("Missing field '") + required + "'";
            return false;
        }
    }
    return true;
}

bool parseConfig(const SaveData& data, GameConfig& config, std::string& error)
{
    const auto size = parseInt(data.fields.at("size"));
    if (!size || !isValidBoardSize(*size)) {
        error = "Invalid board size";
        return false;
    }
    config.boardSize = *size;

    std::istringstream komiStream(data.fields.at("komi"));
    if (!(komiStream >> config.komi) || config.komi < 0.0 || config.komi > 50.0) {
        error = "Invalid komi";
        return false;
    }

    const std::string mode = data.fields.at("mode");
    if (mode != "pvp" && mode != "pvai") {
        error = "Invalid game mode";
        return false;
    }
    config.mode = mode == "pvai" ? GameMode::PlayerVsAI : GameMode::PlayerVsPlayer;

    const auto difficulty = parseDifficulty(data.fields.at("difficulty"));
    const auto human = parseColor(data.fields.at("human"));
    if (!difficulty || !human) {
        error = "Invalid AI difficulty or player color";
        return false;
    }
    config.difficulty = *difficulty;
    config.humanColor = *human;
    return true;
}

bool playMoves(Game& game, const std::vector<Move>& moves, std::string& error)
{
    for (std::size_t i = 0; i < moves.size(); ++i) {
        if (game.phase() == Phase::Scoring) {
            game.resumePlay();
        }
        const MoveReport report = game.play(moves[i]);
        if (!report.ok()) {
            error = "Move " + std::to_string(i + 1) + " (" + moveToText(moves[i], game.board().size())
                + ") is illegal: " + describeMoveError(report.error);
            return false;
        }
    }
    return true;
}

bool boardMatches(const Board& board, const std::vector<std::string>& rows, std::string& error)
{
    if (static_cast<int>(rows.size()) != board.size()) {
        error = "Board has the wrong number of rows";
        return false;
    }
    for (int row = 0; row < board.size(); ++row) {
        if (static_cast<int>(rows[row].size()) != board.size()) {
            error = "Board row " + std::to_string(row + 1) + " has the wrong length";
            return false;
        }
        for (int col = 0; col < board.size(); ++col) {
            const char symbol = rows[row][col];
            if (symbol != 'X' && symbol != 'O' && symbol != '.') {
                error = "Board contains an invalid stone value";
                return false;
            }
            if (symbol != stoneSymbol(board.at({row, col}))) {
                error = "Board does not match the move history";
                return false;
            }
        }
    }
    return true;
}

bool applyDeadStones(Game& game, const std::string& text, std::string& error)
{
    const auto tokens = parseCountedList(text);
    if (!tokens) {
        error = "Invalid dead stone list";
        return false;
    }
    if (!tokens->empty() && game.phase() != Phase::Scoring) {
        error = "Dead stones are only valid during scoring";
        return false;
    }
    std::vector<bool> expected(game.deadStones().size(), false);
    for (const std::string& token : *tokens) {
        const auto point = parseGoCoordinate(token, game.board().size());
        if (!point || game.board().at(*point) == Stone::Empty) {
            error = "Dead stone list refers to an empty point";
            return false;
        }
        expected[point->row * game.board().size() + point->col] = true;
        if (!game.isDead(*point)) {
            game.toggleDeadGroup(*point);
        }
    }
    if (game.deadStones() != expected) {
        error = "Dead stone list does not cover whole groups";
        return false;
    }
    return true;
}

bool verifyState(const Game& game, const SaveData& data, std::string& error)
{
    const auto toMove = parseColor(data.fields.at("to_move"));
    const auto passes = parseInt(data.fields.at("passes"));
    std::istringstream captureStream(data.fields.at("captures"));
    int blackCaptures = -1;
    int whiteCaptures = -1;
    captureStream >> blackCaptures >> whiteCaptures;

    if (!toMove || *toMove != game.toMove()) {
        error = "Side to move does not match the move history";
        return false;
    }
    if (!passes || *passes != game.position().consecutivePasses()) {
        error = "Pass count does not match the move history";
        return false;
    }
    if (blackCaptures != game.position().capturesBy(Stone::Black)
        || whiteCaptures != game.position().capturesBy(Stone::White)) {
        error = "Capture counts do not match the move history";
        return false;
    }
    return boardMatches(game.board(), data.boardRows, error);
}

}

std::string serialize(const Game& game)
{
    const GameConfig& config = game.config();
    const Board& board = game.board();
    const int size = board.size();
    std::ostringstream out;
    out << header << ' ' << formatVersion << '\n';
    out << "size " << size << '\n';
    out << "komi " << config.komi << '\n';
    out << "mode " << (config.mode == GameMode::PlayerVsAI ? "pvai" : "pvp") << '\n';
    out << "difficulty " << lowercase(difficultyName(config.difficulty)) << '\n';
    out << "human " << colorToken(config.humanColor) << '\n';
    out << "phase " << phaseToken(game.phase()) << '\n';
    out << "to_move " << colorToken(game.toMove()) << '\n';
    out << "passes " << game.position().consecutivePasses() << '\n';
    out << "captures " << game.position().capturesBy(Stone::Black) << ' '
        << game.position().capturesBy(Stone::White) << '\n';
    writeMoveList(out, "moves", game.moves(), size);
    writeMoveList(out, "redo", game.redoMoves(), size);

    std::vector<std::string> dead;
    for (int row = 0; row < size; ++row) {
        for (int col = 0; col < size; ++col) {
            if (game.isDead({row, col})) {
                dead.push_back(toGoCoordinate({row, col}, size));
            }
        }
    }
    out << "dead " << dead.size();
    for (const std::string& point : dead) {
        out << ' ' << point;
    }
    out << "\nboard\n";
    for (int row = 0; row < size; ++row) {
        for (int col = 0; col < size; ++col) {
            out << stoneSymbol(board.at({row, col}));
        }
        out << '\n';
    }
    out << "end\n";
    return out.str();
}

LoadResult deserialize(const std::string& text)
{
    LoadResult result;
    SaveData data;
    GameConfig config;
    if (!readSaveData(text, data, result.error) || !parseConfig(data, config, result.error)) {
        return result;
    }

    const auto phase = parsePhase(data.fields.at("phase"));
    const auto moves = parseMoves(data.fields.at("moves"), config.boardSize);
    const auto redoMoves = parseMoves(data.fields.at("redo"), config.boardSize);
    if (!phase || !moves || !redoMoves) {
        result.error = "Invalid phase or move list";
        return result;
    }
    if (*phase != Phase::Playing && !redoMoves->empty()) {
        result.error = "Redo history is only valid while playing";
        return result;
    }

    Game game(config);
    if (!playMoves(game, *moves, result.error)) {
        return result;
    }
    if (*phase == Phase::Playing && game.phase() == Phase::Scoring) {
        game.resumePlay();
    }
    if ((*phase == Phase::Playing) != (game.phase() == Phase::Playing)) {
        result.error = "Game phase does not match the move history";
        return result;
    }
    if (!verifyState(game, data, result.error) || !applyDeadStones(game, data.fields.at("dead"), result.error)) {
        return result;
    }
    if (*phase == Phase::Finished) {
        game.acceptScore();
    }

    if (!playMoves(game, *redoMoves, result.error)) {
        result.error = "Redo history: " + result.error;
        return result;
    }
    for (std::size_t i = 0; i < redoMoves->size(); ++i) {
        game.undo();
    }

    result.game = std::move(game);
    return result;
}

bool saveToFile(const Game& game, const std::filesystem::path& path, std::string& error)
{
    std::error_code ignored;
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path(), ignored);
    }
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file) {
        error = "Cannot open " + path.filename().string() + " for writing";
        return false;
    }
    file << serialize(game);
    if (!file) {
        error = "Failed while writing " + path.filename().string();
        return false;
    }
    return true;
}

LoadResult loadFromFile(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return {std::nullopt, "Cannot open " + path.filename().string()};
    }
    std::ostringstream content;
    content << file.rdbuf();
    return deserialize(content.str());
}

std::vector<std::filesystem::path> listSaves(const std::filesystem::path& directory)
{
    std::vector<std::filesystem::path> saves;
    std::error_code error;
    for (std::filesystem::directory_iterator it(directory, error), end; !error && it != end; it.increment(error)) {
        if (it->is_regular_file(error) && it->path().extension() == fileExtension) {
            saves.push_back(it->path());
        }
    }
    std::sort(saves.begin(), saves.end(), [](const auto& left, const auto& right) {
        std::error_code ignored;
        return std::filesystem::last_write_time(left, ignored) > std::filesystem::last_write_time(right, ignored);
    });
    return saves;
}

std::string sanitizeFileName(const std::string& name)
{
    std::string result;
    for (const char c : name) {
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_') {
            result += c;
        } else if (c == ' ') {
            result += '_';
        }
    }
    return result.substr(0, 48);
}

}
