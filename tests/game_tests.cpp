#include "AI.h"
#include "Evaluator.h"
#include "Game.h"
#include "SaveManager.h"
#include "Scoring.h"
#include "Settings.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <source_location>
#include <string>
#include <vector>

namespace {

int failedChecks = 0;
int totalChecks = 0;

void expect(bool condition, const std::string& description,
    std::source_location location = std::source_location::current())
{
    ++totalChecks;
    if (!condition) {
        ++failedChecks;
        std::cout << "    FAILED: " << description << " (line " << location.line() << ")\n";
    }
}

Point at(const std::string& coordinate, int size = 9)
{
    return *parseGoCoordinate(coordinate, size);
}

MoveReport playAt(Game& game, const std::string& coordinate)
{
    if (coordinate == "pass") {
        return game.play(Move::pass());
    }
    return game.play(Move::place(at(coordinate, game.board().size())));
}

Game playSequence(const std::vector<std::string>& moves, GameConfig config = {9})
{
    Game game(config);
    for (const std::string& move : moves) {
        const MoveReport report = playAt(game, move);
        expect(report.ok(), "setup move " + move + " should be legal");
    }
    return game;
}

struct Snapshot {
    Board board;
    Stone toMove;
    int blackCaptures;
    int whiteCaptures;
    int passes;
    std::size_t moveCount;
    Phase phase;

    explicit Snapshot(const Game& game)
        : board(game.board())
        , toMove(game.toMove())
        , blackCaptures(game.position().capturesBy(Stone::Black))
        , whiteCaptures(game.position().capturesBy(Stone::White))
        , passes(game.position().consecutivePasses())
        , moveCount(game.moves().size())
        , phase(game.phase())
    {
    }

    bool operator==(const Snapshot& other) const
    {
        return board == other.board && toMove == other.toMove && blackCaptures == other.blackCaptures
            && whiteCaptures == other.whiteCaptures && passes == other.passes && moveCount == other.moveCount
            && phase == other.phase;
    }
};

Game koPosition()
{
    return playSequence({"D6", "E6", "C5", "D5", "D4", "E4", "J9", "F5"});
}

void testCoordinates()
{
    expect(toGoCoordinate({0, 0}, 19) == "A19", "top-left of 19x19 is A19");
    expect(toGoCoordinate({18, 8}, 19) == "J1", "column after H is J (I is skipped)");
    expect(parseGoCoordinate("T1", 19) == Point{18, 18}, "T1 is the bottom-right corner");
    expect(!parseGoCoordinate("I5", 19).has_value(), "column I does not exist");
    expect(!parseGoCoordinate("K10", 9).has_value(), "K is outside a 9x9 board");
    expect(parseMoveText("pass", 9) == Move::pass(), "pass parses as a pass move");
}

void testPlaceOnEmpty()
{
    Game game(GameConfig{9});
    expect(game.toMove() == Stone::Black, "black moves first");
    const MoveReport report = playAt(game, "E5");
    expect(report.ok(), "placing on an empty point succeeds");
    expect(game.board().at(at("E5")) == Stone::Black, "stone appears on the board");
    expect(game.toMove() == Stone::White, "turn passes to white");
    expect(game.moves().size() == 1, "move is recorded");
}

void testOccupiedRejected()
{
    Game game = playSequence({"E5"});
    const Snapshot before(game);
    const MoveReport report = playAt(game, "E5");
    expect(report.error == MoveError::Occupied, "occupied point is rejected");
    expect(Snapshot(game) == before, "rejected move leaves state untouched");
}

void testSingleCapture()
{
    Game game = playSequence({"D5", "E5", "F5", "A1", "E4", "A2"});
    const MoveReport report = playAt(game, "E6");
    expect(report.ok() && report.captured.size() == 1, "removing the last liberty captures the stone");
    expect(game.board().at(at("E5")) == Stone::Empty, "captured stone is removed");
    expect(game.position().capturesBy(Stone::Black) == 1, "black capture count increases");
}

void testMultipleGroupCapture()
{
    Game game = playSequence({"A3", "A2", "B2", "B1", "C1", "J9"});
    const MoveReport report = playAt(game, "A1");
    expect(report.ok() && report.captured.size() == 2, "one move captures two separate groups");
    expect(game.board().at(at("A2")) == Stone::Empty && game.board().at(at("B1")) == Stone::Empty,
        "both groups are removed");
    expect(game.board().at(at("A1")) == Stone::Black, "capturing stone stays on the board");
}

void testLargeGroupCapture()
{
    Game game = playSequence({"D5", "E5", "D6", "E6", "F6", "J1", "F5", "J2", "E4", "J3"});
    const MoveReport report = playAt(game, "E7");
    expect(report.ok() && report.captured.size() == 2, "a connected two-stone group is captured together");
}

void testSuicideRejected()
{
    Game game = playSequence({"A2", "J9", "B1"});
    const Snapshot before(game);
    const MoveReport report = playAt(game, "A1");
    expect(report.error == MoveError::Suicide, "suicide is rejected");
    expect(Snapshot(game) == before, "suicide attempt leaves state untouched");
}

void testCaptureIsNotSuicide()
{
    Game game = playSequence({"B1", "A2", "J9", "B2", "J8", "C1"});
    expect(game.board().at(at("B1")) == Stone::Black, "setup keeps B1 black");
    const MoveReport report = playAt(game, "J7");
    expect(report.ok(), "filler move");
    const MoveReport capture = playAt(game, "A1");
    expect(capture.ok() && capture.captured.size() == 1, "move without liberties is legal when it captures");
}

void testKoRejected()
{
    Game game = koPosition();
    const MoveReport take = playAt(game, "E5");
    expect(take.ok() && take.captured.size() == 1, "black takes the ko");
    const Snapshot before(game);
    const MoveReport retake = playAt(game, "D5");
    expect(retake.error == MoveError::Superko, "immediate recapture is rejected by the ko rule");
    expect(Snapshot(game) == before, "rejected ko leaves state untouched");

    expect(playAt(game, "A1").ok() && playAt(game, "J1").ok(), "ko threats elsewhere are legal");
    expect(playAt(game, "D5").ok(), "recapture is legal once the whole-board position is new");
}

void testPassAndScoringEntry()
{
    Game game(GameConfig{9});
    const MoveReport first = playAt(game, "pass");
    expect(first.ok() && game.toMove() == Stone::White, "a pass changes the turn");
    expect(game.position().consecutivePasses() == 1 && game.phase() == Phase::Playing, "one pass keeps playing");
    const MoveReport second = playAt(game, "pass");
    expect(second.enteredScoring && game.phase() == Phase::Scoring, "two passes enter scoring");
    expect(playAt(game, "E5").error == MoveError::GameNotInProgress, "stones cannot be placed during scoring");
}

void testMoveResetsPassCount()
{
    Game game = playSequence({"pass", "E5"});
    expect(game.position().consecutivePasses() == 0, "a stone move resets the pass count");
    playAt(game, "pass");
    expect(game.phase() == Phase::Playing, "a single pass after a move does not end the game");
}

void testUndoRedo()
{
    Game game = playSequence({"D5", "E5", "F5", "A1", "E4", "A2"});
    const Snapshot beforeCapture(game);
    playAt(game, "E6");
    const Snapshot afterCapture(game);

    expect(game.undo(), "undo succeeds");
    expect(Snapshot(game) == beforeCapture, "undo restores board, captures, turn and history");
    expect(game.redo(), "redo succeeds");
    expect(Snapshot(game) == afterCapture, "redo restores the undone capture exactly");

    game.undo();
    playAt(game, "J9");
    expect(!game.canRedo(), "a new move clears the redo history");

    playAt(game, "pass");
    const Snapshot afterPass(game);
    game.undo();
    expect(game.position().consecutivePasses() == 0, "undoing a pass restores the pass count");
    game.redo();
    expect(Snapshot(game) == afterPass, "redoing a pass restores it");
}

void testUndoRestoresKoState()
{
    Game game = koPosition();
    playAt(game, "E5");
    game.undo();
    game.redo();
    expect(playAt(game, "D5").error == MoveError::Superko, "ko restriction survives undo/redo");
}

void testUndoLeavesScoring()
{
    Game game = playSequence({"E5", "pass", "pass"});
    expect(game.phase() == Phase::Scoring, "in scoring after two passes");
    game.undo();
    expect(game.phase() == Phase::Playing && game.position().consecutivePasses() == 1,
        "undo returns from scoring to play");
}

void testAIUndoExchange()
{
    Game game(GameConfig{9, GameMode::PlayerVsAI, Difficulty::Easy, Stone::Black});
    playAt(game, "E5");
    playAt(game, "C3");
    playAt(game, "G7");
    playAt(game, "C7");
    expect(game.undoTurn() == 2, "undo removes the AI reply and the human move together");
    expect(game.moves().size() == 2 && game.toMove() == Stone::Black, "human is to move after undo");
    expect(game.redoTurn() == 2, "redo restores the same exchange");
    expect(game.moves().size() == 4 && game.moves().back() == Move::place(at("C7")), "redo keeps the AI's move");

    Game aiFirst(GameConfig{9, GameMode::PlayerVsAI, Difficulty::Easy, Stone::White});
    playAt(aiFirst, "E5");
    expect(aiFirst.undoTurn() == 0 && aiFirst.moves().size() == 1, "AI opening move alone is not undone");
}

void testResumeAndAccept()
{
    Game game = playSequence({"E5", "pass", "pass"});
    expect(game.resumePlay() && game.phase() == Phase::Playing, "players can resume play from scoring");
    expect(game.position().consecutivePasses() == 0, "resuming clears the pass count");
    playAt(game, "pass");
    playAt(game, "pass");
    expect(game.acceptScore() && game.phase() == Phase::Finished, "accepting the score finishes the game");
    expect(!game.canUndo() && playAt(game, "D4").error == MoveError::GameNotInProgress,
        "no moves after the game is finished");
}

Board columnBoard()
{
    Board board(9);
    for (int row = 0; row < 9; ++row) {
        board.set({row, 3}, Stone::Black);
        board.set({row, 5}, Stone::White);
    }
    return board;
}

void testAreaScoring()
{
    Board board = columnBoard();
    std::vector<bool> noDead(81, false);
    const ScoreBreakdown score = scoreByArea(board, noDead, 7.5);
    expect(score.blackStones == 9 && score.blackTerritory == 27, "black has 9 stones and 27 points of territory");
    expect(score.whiteStones == 9 && score.whiteTerritory == 27, "white has 9 stones and 27 points of territory");
    expect(score.neutralPoints == 9, "the shared column is neutral");
    expect(score.blackTotal == 36.0 && score.whiteTotal == 43.5, "totals include komi for white");
    expect(score.winner == Stone::White, "white wins by komi");

    board.set(at("B5"), Stone::White);
    const ScoreBreakdown invaded = scoreByArea(board, noDead, 7.5);
    expect(invaded.blackTerritory == 0, "a living invader makes the region neutral");

    std::vector<bool> dead(81, false);
    dead[at("B5").row * 9 + at("B5").col] = true;
    const ScoreBreakdown marked = scoreByArea(board, dead, 7.5);
    expect(marked.blackTerritory == 27 && marked.deadWhiteStones == 1, "dead stones count as territory");
}

void testDeadMarkingInGame()
{
    Game game = playSequence({"E5", "D4", "pass", "pass"});
    expect(game.toggleDeadGroup(at("D4")), "a group can be marked dead in scoring");
    expect(game.isDead(at("D4")), "group is reported dead");
    expect(game.score().deadWhiteStones == 1, "dead stone is counted in the score");
    game.toggleDeadGroup(at("D4"));
    expect(!game.isDead(at("D4")), "toggling again restores the group");
}

void testSaveLoadRoundTrip()
{
    GameConfig config{9, GameMode::PlayerVsAI, Difficulty::Hard, Stone::White};
    Game game = playSequence({"D5", "E5", "F5", "A1", "E4", "A2", "E6", "pass", "J9", "B2"}, config);
    game.undo();
    game.undo();

    const SaveManager::LoadResult loaded = SaveManager::deserialize(SaveManager::serialize(game));
    expect(loaded.game.has_value(), "a saved game loads back: " + loaded.error);
    if (!loaded.game) {
        return;
    }
    const Game& copy = *loaded.game;
    expect(Snapshot(copy) == Snapshot(game), "board, turn, captures and passes survive a round trip");
    expect(copy.moves() == game.moves(), "move history survives a round trip");
    expect(copy.redoMoves() == game.redoMoves(), "redo history survives a round trip");
    expect(copy.config().mode == GameMode::PlayerVsAI && copy.config().difficulty == Difficulty::Hard
            && copy.config().humanColor == Stone::White,
        "mode, difficulty and player color survive a round trip");
}

void testSaveLoadScoringState()
{
    Game game = playSequence({"E5", "D4", "pass", "pass"});
    game.toggleDeadGroup(at("D4"));
    const SaveManager::LoadResult loaded = SaveManager::deserialize(SaveManager::serialize(game));
    expect(loaded.game && loaded.game->phase() == Phase::Scoring, "scoring phase survives a round trip");
    expect(loaded.game && loaded.game->isDead(at("D4")), "dead stone marks survive a round trip");

    Game resumed = playSequence({"E5", "pass", "pass"});
    resumed.resumePlay();
    playAt(resumed, "C3");
    const SaveManager::LoadResult again = SaveManager::deserialize(SaveManager::serialize(resumed));
    expect(again.game && Snapshot(*again.game) == Snapshot(resumed), "resumed games survive a round trip");
}

void testMalformedSaves()
{
    Game current = playSequence({"E5", "D4"});
    const Snapshot before(current);
    const std::string valid = SaveManager::serialize(current);

    auto replace = [&](const std::string& from, const std::string& to) {
        std::string text = valid;
        text.replace(text.find(from), from.size(), to);
        return text;
    };

    const std::vector<std::pair<std::string, std::string>> broken{
        {"", "empty file"},
        {"hello world", "wrong header"},
        {replace("GOGAME_SAVE 1", "GOGAME_SAVE 99"), "unsupported version"},
        {replace("size 9", "size 7"), "invalid board size"},
        {replace("moves 2 E5 D4", "moves 2 E5 E5"), "illegal move in history"},
        {replace("moves 2 E5 D4", "moves 3 E5 D4"), "wrong move count"},
        {replace("to_move black", "to_move white"), "wrong side to move"},
        {replace("captures 0 0", "captures 4 0"), "wrong capture count"},
        {replace("mode pvp", "mode chess"), "invalid mode"},
        {valid.substr(0, valid.size() / 2), "truncated file"},
    };
    for (const auto& [text, label] : broken) {
        const SaveManager::LoadResult result = SaveManager::deserialize(text);
        expect(!result.game && !result.error.empty(), "rejects save with " + label);
        if (result.game) {
            current = *result.game;
        }
    }

    std::string badStone = valid;
    badStone[badStone.find("board\n") + 6] = 'Z';
    expect(!SaveManager::deserialize(badStone).game, "rejects invalid stone characters");

    const SaveManager::LoadResult missing = SaveManager::loadFromFile("definitely_missing_file.gosave");
    expect(!missing.game && !missing.error.empty(), "missing file fails safely");
    expect(Snapshot(current) == before, "current game is unchanged after failed loads");
}

void testSaveFiles()
{
    const auto directory = std::filesystem::temp_directory_path() / "gogame_tests";
    std::filesystem::remove_all(directory);
    Game game = playSequence({"E5", "D4", "C3"});
    std::string error;
    const auto path = directory / ("round_trip" + std::string(SaveManager::fileExtension));
    expect(SaveManager::saveToFile(game, path, error), "save file is written: " + error);
    const SaveManager::LoadResult loaded = SaveManager::loadFromFile(path);
    expect(loaded.game && Snapshot(*loaded.game) == Snapshot(game), "save file loads back");
    expect(SaveManager::listSaves(directory).size() == 1, "save directory listing finds the file");
    expect(SaveManager::sanitizeFileName("my game/../x?") == "my_gamex", "file names are sanitized");
    std::filesystem::remove_all(directory);
}

void testSettingsPersistence()
{
    const auto path = std::filesystem::temp_directory_path() / "gogame_settings_test.cfg";
    Settings settings;
    settings.boardTheme = 3;
    settings.stoneStyle = 2;
    settings.soundEnabled = false;
    settings.soundTheme = 1;
    settings.soundVolume = 0.25f;
    settings.musicEnabled = false;
    settings.musicTrack = 2;
    settings.musicVolume = 0.5f;
    expect(saveSettings(settings, path), "settings file is written");
    const Settings loaded = loadSettings(path);
    expect(loaded.boardTheme == 3 && loaded.stoneStyle == 2 && !loaded.soundEnabled && loaded.soundTheme == 1
            && loaded.musicTrack == 2 && !loaded.musicEnabled,
        "settings survive a round trip");
    expect(loaded.soundVolume == 0.25f && loaded.musicVolume == 0.5f, "volumes survive a round trip");

    std::ofstream(path) << "board_theme=99\nsound_volume=7\nstone_style=-4\ngarbage line\n";
    const Settings clamped = loadSettings(path);
    expect(clamped.boardTheme == static_cast<int>(boardThemeNames().size()) - 1, "board theme index is clamped");
    expect(clamped.soundVolume == 1.0f && clamped.stoneStyle == 0, "out-of-range values are clamped");
    std::filesystem::remove(path);
    const Settings defaults = loadSettings(path);
    expect(defaults.boardTheme == 0 && defaults.soundEnabled, "missing settings file gives defaults");
}

void expectLegalAIMove(const Game& game, Difficulty difficulty, const std::string& label, double maxSeconds)
{
    std::atomic<bool> stop{false};
    const auto start = std::chrono::steady_clock::now();
    const AIDecision decision = chooseAIMove(game.position(), difficulty, game.config().komi, 7, stop);
    const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    expect(game.position().isLegal(decision.move), difficultyName(difficulty) + " AI plays legally: " + label);
    expect(seconds < maxSeconds, difficultyName(difficulty) + " AI answers in time: " + label);
}

void testAILegality()
{
    const std::vector<Difficulty> levels{Difficulty::Easy, Difficulty::Medium, Difficulty::Hard};
    for (Difficulty level : levels) {
        expectLegalAIMove(Game(GameConfig{9}), level, "empty 9x9", 6.0);
        expectLegalAIMove(Game(GameConfig{19}), level, "empty 19x19", 8.0);
        expectLegalAIMove(playSequence({"D5", "E5", "F5", "A1", "E4", "A2", "C3", "G7"}), level, "midgame", 6.0);
        Game ko = koPosition();
        playAt(ko, "E5");
        expectLegalAIMove(ko, level, "ko must not be retaken", 6.0);
        AIDecision decision = chooseAIMove(ko.position(), level, 7.5, 3, std::atomic<bool>{false});
        expect(decision.move != Move::place(at("D5")), difficultyName(level) + " AI respects ko");
    }
}

void testAIFindsCapture()
{
    Game game = playSequence(
        {"D5", "E5", "D6", "E6", "D7", "E7", "F5", "A1", "F6", "A3", "F7", "J9", "E4", "J1"});
    for (Difficulty level : {Difficulty::Medium, Difficulty::Hard}) {
        std::atomic<bool> stop{false};
        const AIDecision decision = chooseAIMove(game.position(), level, 7.5, 11, stop);
        expect(decision.move == Move::place(at("E8")), difficultyName(level) + " AI captures a group in atari");
    }

    Game rescue = playSequence(
        {"D5", "E5", "D6", "E6", "D7", "E7", "F5", "A1", "F6", "A2", "F7", "A3", "E4"});
    std::atomic<bool> stop{false};
    const AIDecision decision = chooseAIMove(rescue.position(), Difficulty::Hard, 7.5, 13, stop);
    expect(decision.move == Move::place(at("E8")), "Hard AI rescues its stone group from atari");
}

void testAIStopsWhenCancelled()
{
    std::atomic<bool> stop{true};
    const auto start = std::chrono::steady_clock::now();
    const AIDecision decision = chooseAIMove(Game(GameConfig{19}).position(), Difficulty::Hard, 7.5, 5, stop);
    const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    expect(seconds < 1.0 && Game(GameConfig{19}).position().isLegal(decision.move), "cancelled search returns quickly");
}

void testAISelfPlayFinishes()
{
    Game game(GameConfig{9, GameMode::PlayerVsPlayer});
    const Difficulty levels[] = {Difficulty::Easy, Difficulty::Medium};
    std::atomic<bool> stop{false};
    int turn = 0;
    while (game.phase() == Phase::Playing && turn < 400) {
        const Difficulty level = levels[turn % 2];
        const AIDecision decision = chooseAIMove(game.position(), level, 7.5, 100 + turn, stop);
        const MoveReport report = game.play(decision.move);
        expect(report.ok(), "self-play move " + std::to_string(turn + 1) + " is legal");
        if (!report.ok()) {
            break;
        }
        ++turn;
    }
    expect(game.phase() == Phase::Scoring, "an AI game reaches scoring by passing");
}

void testAsyncPlayer()
{
    AIPlayer player;
    Game game(GameConfig{9});
    player.start(game.position(), Difficulty::Medium, 7.5);
    expect(player.isThinking(), "AI player reports it is thinking");
    std::optional<AIDecision> decision;
    const auto start = std::chrono::steady_clock::now();
    while (!decision && std::chrono::steady_clock::now() - start < std::chrono::seconds(10)) {
        decision = player.takeDecision();
    }
    expect(decision && game.position().isLegal(decision->move), "background AI delivers a legal move");
    player.start(game.position(), Difficulty::Hard, 7.5);
    player.cancel();
    expect(!player.isThinking(), "cancelling stops the background AI");
}

void testEvaluatorBasics()
{
    Board board(9);
    board.set(at("E5"), Stone::Black);
    expect(estimateScoreMargin(board, Stone::Black, 0.0) > 0, "a lone stone gives its owner influence");
    Board eye(9);
    eye.set(at("A2"), Stone::Black);
    eye.set(at("B1"), Stone::Black);
    expect(isOwnEye(eye, at("A1"), Stone::Black), "a corner point surrounded by own stones is an eye");
    expect(!isOwnEye(eye, at("A1"), Stone::White), "it is not an eye for the opponent");
}

void testFullFlow()
{
    Game game(GameConfig{19, GameMode::PlayerVsAI, Difficulty::Medium});
    for (const char* move : {"D4", "Q16", "Q4", "D16"}) {
        expect(playAt(game, move).ok(), std::string("flow move ") + move);
    }
    const std::string saved = SaveManager::serialize(game);
    game.reset();
    expect(game.moves().empty() && game.board().isEmpty() && game.toMove() == Stone::Black, "reset clears the game");
    SaveManager::LoadResult loaded = SaveManager::deserialize(saved);
    expect(loaded.game && loaded.game->moves().size() == 4, "the game loads after a reset");
}

}

int main()
{
    const std::vector<std::pair<std::string, std::function<void()>>> tests{
        {"coordinates", testCoordinates},
        {"place on empty intersection", testPlaceOnEmpty},
        {"occupied intersection rejected", testOccupiedRejected},
        {"single stone capture", testSingleCapture},
        {"multiple group capture", testMultipleGroupCapture},
        {"connected group capture", testLargeGroupCapture},
        {"suicide rejected", testSuicideRejected},
        {"capture is not suicide", testCaptureIsNotSuicide},
        {"ko rejected", testKoRejected},
        {"pass and scoring entry", testPassAndScoringEntry},
        {"move resets pass count", testMoveResetsPassCount},
        {"undo and redo", testUndoRedo},
        {"undo restores ko state", testUndoRestoresKoState},
        {"undo leaves scoring", testUndoLeavesScoring},
        {"AI exchange undo", testAIUndoExchange},
        {"resume and accept score", testResumeAndAccept},
        {"area scoring", testAreaScoring},
        {"dead stone marking", testDeadMarkingInGame},
        {"save/load round trip", testSaveLoadRoundTrip},
        {"save/load scoring state", testSaveLoadScoringState},
        {"malformed saves", testMalformedSaves},
        {"save files", testSaveFiles},
        {"settings persistence", testSettingsPersistence},
        {"evaluator basics", testEvaluatorBasics},
        {"AI legality", testAILegality},
        {"AI finds capture", testAIFindsCapture},
        {"AI cancellation", testAIStopsWhenCancelled},
        {"AI self-play finishes", testAISelfPlayFinishes},
        {"background AI player", testAsyncPlayer},
        {"start, play, reset, save, load", testFullFlow},
    };

    int failedTests = 0;
    for (const auto& [name, run] : tests) {
        const int failuresBefore = failedChecks;
        const auto start = std::chrono::steady_clock::now();
        run();
        const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
        const bool passed = failedChecks == failuresBefore;
        failedTests += passed ? 0 : 1;
        std::cout << (passed ? "[PASS] " : "[FAIL] ") << name << " (" << seconds << " s)\n";
    }
    std::cout << "\n" << tests.size() - failedTests << "/" << tests.size() << " tests passed, " << totalChecks - failedChecks
              << "/" << totalChecks << " checks passed\n";
    return failedTests == 0 ? 0 : 1;
}
