# Go Game - AI Agent Build Instructions

> **Primary instruction for the coding agent:** Build a complete, playable Go game in C++ that satisfies this document and the project brief in `Project - Go game.pdf`. Work in the repository, create and edit the actual files, compile the application, run the tests, and fix problems before reporting completion. Do not stop after producing a plan, pseudocode, or a list of suggested changes.

## 1. Project goal

Create a desktop Go game with a graphical interface. The game must support:

- **Player vs Player:** two people play on the same device.
- **Player vs AI:** one person plays against an AI with Easy, Medium, and Hard difficulty levels.
- The main rules of Go, including legal moves, liberties, captures, ko, passing, and end-game scoring.
- Undo, redo, reset, save, load, settings, and exit controls.
- Clear build and run instructions, a project report, and a work-division document for submission.

The course PDF is the primary source for required features. Optional improvements must not replace or delay the required features.

## 2. How the coding agent must work

1. Inspect the existing repository before editing. Keep working code and assets that are useful. If the repository is empty, create the structure in Section 8.
2. Implement the game in small, testable parts. Keep the rules and AI independent of the graphical interface.
3. Build a working end-to-end game rather than a UI mockup. Buttons must perform their advertised actions; do not leave placeholder handlers, empty methods, fake AI moves, or unimplemented TODOs.
4. After each major part, compile and test it. Fix build errors, runtime errors, invalid moves, and broken flows before continuing.
5. If a detail is not specified in the PDF, use the explicit defaults in this README and document the decision. Do not stop for clarification unless a genuine blocker prevents progress.
6. At the end, provide a concise summary of what was built, how to build and run it, the tests performed, and anything that remains incomplete. Do not claim that a test or build succeeded unless it actually ran successfully.

## 3. Code style requirements

- Use **idiomatic, readable C++** with meaningful names, focused functions, consistent formatting, and a straightforward design suitable for an introductory programming project.
- **Do not add comments to code.** Source files, headers, tests, and build configuration files must contain no `//`, `/* ... */`, or explanatory code comments. Make the code understandable through names and structure instead.
- Avoid artificial complexity, unnecessarily generic frameworks, huge functions, duplicated logic, unused abstractions, dead code, and boilerplate that does not serve the game.
- Keep responsibilities separated. The entry point should initialize the app and run the main loop, not contain all the rules, AI, rendering, and file handling.
- Handle invalid input and file errors gracefully. Do not crash when a save file is missing, malformed, or incompatible.
- Do not claim that the code is human-written. The requirement is for natural, maintainable code style, not for misrepresenting authorship.

## 4. Technology choices

Use the following defaults unless the repository already has a working, suitable setup:

- **Language:** C++17 or newer.
- **GUI and audio:** SFML.
- **Build system:** CMake.
- **Persistence:** a documented, versioned text save format or JSON. Avoid adding a large dependency for a simple save file.
- **Testing:** a repeatable test target for game rules, state management, and AI move legality.

Choose one SFML major version and use its API consistently. Record the required compiler, SFML version, and build commands in this README. Keep core rule tests runnable without opening a game window where practical.

## 5. Required game behavior

### 5.1 Modes, players, and turns

- Provide a Player vs Player mode and a Player vs AI mode.
- In Player vs Player, retain the course brief's player mapping: **Player 1 is White and Player 2 is Black**.
- Follow the normal Go turn order: **Black moves first**. Always show the active color and, where relevant, the player or AI name.
- In Player vs AI, default to the human playing Black and the AI playing White so the human makes the opening move and the AI responds immediately afterward. Do not run an AI move during a human turn.
- Let the user select Easy, Medium, or Hard before starting a Player vs AI game. Show the selected difficulty during the game.
- Provide New Game, Reset, Undo, Redo, Save, Load, Settings, and Exit actions.

### 5.2 Board and interaction

- Render a Go board with a clear grid, visible intersections, and distinct black and white stones.
- Use a 19 x 19 board as the default. If additional board sizes can be implemented cleanly, 9 x 9 and 13 x 13 may be offered as optional settings; they must not compromise the default board.
- Place a stone on the nearest valid intersection when the user clicks close enough to it. Clicking outside the board or on an occupied intersection must not change the board.
- Display whose turn it is, captured-stone information, game status, and useful move feedback.
- Show visible notifications for successful captures, rejected illegal moves, ko/superko restrictions, passes, and final scoring.
- Make the interface adapt reasonably to window resizing and avoid requiring pixel-perfect clicks.
- Include a Pass button. Passing is part of Go and must count toward the end-game rule.

### 5.3 Rules and legality

Implement and test the rules in the game engine rather than only in the UI:

- A move places one stone of the current color on an empty intersection.
- A group consists of orthogonally connected stones of the same color. Empty orthogonally adjacent intersections are its liberties.
- After placing a stone, capture every adjacent opponent group that has no liberties.
- Reject a move that leaves the played stone's group with no liberties after captures, unless the selected documented ruleset explicitly allows suicide. Use the default rule of **forbidding suicide**.
- Enforce ko. Use positional superko if feasible: reject a move that recreates a previous board position. At minimum, enforce simple ko correctly. Explain the implemented rule in the report.
- Prevent a move from changing the board or turn when it is illegal. Show a message explaining the rejection.
- Two consecutive passes enter end-game review and scoring. A normal stone placement resets the consecutive-pass count.
- Do not allow ordinary moves after the game has ended, except for the actions required by the scoring/review screen.
- Keep a move list using recognizable Go coordinates as a useful optional feature if time permits.

When there are choices not fixed in the PDF, use a consistent ruleset and record it. A practical default is area scoring with a 7.5-point komi for White. Make the scoring convention visible in the report and UI. In scoring review, allow the players to mark dead groups if this can be implemented reliably; do not silently remove stones from a final score without showing what was counted.

### 5.4 End-game scoring

- Enter scoring/review after two consecutive passes.
- Calculate and display both players' scores, the komi, and the winner or a draw.
- Under the default area-scoring choice, count living stones plus empty regions bordered exclusively by one color as that color's area; treat neutral regions as neutral.
- Present the calculation clearly enough that the result can be checked. Do not label an approximate score as official tournament scoring.
- Provide a way to return to a new game after the result is shown.

## 6. AI opponent

Implement three distinct difficulty levels. All AI moves must pass through the same legality checks as human moves.

### Easy

- Choose quickly from legal moves, with little strategic evaluation.
- Randomness is acceptable, but do not choose occupied points, suicide moves, ko-forbidden moves, or otherwise illegal moves.
- When there is no useful legal move, allow the AI to pass.

### Medium

- Use a shallow **Minimax** search as required by the course brief.
- Use a practical board evaluation that considers factors such as captures, immediate threats, liberties, groups in atari, connections, and influence or area.
- Make reasonable moves without attempting exhaustive search over every intersection.

### Hard

- Use **Minimax with Alpha-Beta pruning**, searching deeper than Medium within a clear computation budget.
- Reuse the same evaluator and legal-move engine where appropriate.
- Keep move generation practical on a Go board: rank plausible candidates and consider strategically relevant intersections, such as points near existing groups and immediate capture or rescue moves. Include a sensible opening move on an empty board.
- Put limits on depth, candidate count, and/or time so the game stays responsive. Avoid freezing the UI during search; use a safe worker-thread approach or a strictly bounded search.
- Make Hard meaningfully stronger than Medium, but do not describe it as professional-strength Go AI. The brief requires a student-project Minimax approach, not a claim of world-class play.

Document the evaluation features, search-depth strategy, candidate restrictions, and performance limits. Ensure a pass is handled sensibly when the game is near its end.

## 7. Game state, undo/redo, save/load, and settings

### 7.1 State management

Track all data necessary to continue a game correctly, including:

- Board positions and side to move.
- Move history and consecutive passes.
- Captured-stone counts and prior board positions needed for the selected ko rule.
- Game phase, scoring state, and any other data that affects legal moves or the visible result.
- Current mode and AI difficulty where relevant.

### 7.2 Undo and redo

- Keep a history of moves or state snapshots and a separate redo history.
- Undo must restore the exact prior board and all related rule state, not merely erase the last visible stone.
- Redo must restore the undone position correctly and must not ask the AI to generate a different move from the original one.
- A new move after undo must clear the redo history.
- Support pass moves in history as well as stone placements.
- In Player vs AI mode, make Undo useful for a human turn: preferably undo the latest AI response and the human move that triggered it as one exchange. Redo should restore that same exchange. If a different behavior is chosen, make it consistent and explain it in the UI or documentation.

### 7.3 Save and load

- Allow the user to save a game and load a prior save through a simple file chooser or an in-app save/load dialog.
- Store the full state needed to resume correctly, not only the visible stone coordinates.
- Validate the file version, board dimensions, stone values, turn, history, and other fields before applying loaded data.
- Load transactionally: if the file is invalid, keep the current game unchanged and show a useful error.
- Provide a small `saves/` directory or an equivalent clear save location, and document where files are stored.

### 7.4 Settings

Provide a settings screen or dialog with working controls for:

- Board color/theme.
- Stone design/style, with options such as Classic, Modern, and Cartoon.
- Sound effects on/off and a selectable sound theme.
- Background music on/off and a choice of available music tracks or themes.
- Separate volume controls for sound effects and background music.

Include the assets needed for the offered sound/music choices. Use original or appropriately licensed assets, and record any third-party asset licenses. Missing optional audio must never crash the game. Settings should update the UI/audio when changed and should persist between launches if practical.

## 8. Suggested repository structure

Adapt this structure if the existing repository has a sensible equivalent, but keep components clearly separated.

```text
GoGame/
├── CMakeLists.txt
├── README.md
├── include/
│   ├── Board.h
│   ├── Game.h
│   ├── AI.h
│   ├── UI.h
│   ├── SaveManager.h
│   └── Settings.h
├── src/
│   ├── main.cpp
│   ├── Board.cpp
│   ├── Game.cpp
│   ├── AI.cpp
│   ├── UI.cpp
│   ├── SaveManager.cpp
│   └── Settings.cpp
├── tests/
│   └── game_tests.cpp
├── assets/
│   ├── fonts/
│   └── audio/
├── saves/
└── docs/
    ├── PROJECT_REPORT.md
    └── WORK_DIVISION.md
```

Names may change if the result remains coherent. Do not put all application logic in `main.cpp` or in one oversized source file.

## 9. Testing and acceptance checklist

Before declaring the project complete, build and run a repeatable test suite. At minimum, test the following cases:

- A stone can be placed on an empty intersection.
- A move on an occupied intersection is rejected without corrupting state.
- A group is captured when its last liberty is removed.
- Multiple adjacent groups are captured when appropriate.
- An illegal suicide move is rejected under the chosen rules.
- A ko/superko repetition is rejected.
- A pass changes the turn, and two consecutive passes enter scoring review.
- A stone move between passes resets the consecutive-pass count.
- Undo and redo restore the board, captures, player turn, passes, and other rule state.
- Save/load round-trips the game state; malformed saves fail safely and do not damage the current game.
- Every AI difficulty returns a legal move or a legal pass and does not block indefinitely.
- Settings controls actually change the board/stone/audio behavior.
- The game can start, play several moves, reset, save, load, and exit without crashing.

Also run the application manually and test the primary buttons and both game modes. Fix warnings and obvious runtime issues where possible. Do not mark unrun tests as passed. Record test commands and known limitations in the report.

## 10. Documentation and submission files

The course brief also asks for documentation beyond the source code. Create both files below with content that matches the implementation.

### `docs/PROJECT_REPORT.md`

Include:

1. Title page information: project title, team members, and submission date. Use clearly marked placeholders for unknown team information; do not invent names.
2. Project overview, objectives, and implemented requirements.
3. Requirements specification showing which required and optional features are implemented.
4. High-level architecture and design decisions. Add a simple class/module diagram if useful.
5. A **C++ game-development tutorial of at least five pages of substantive material**, covering environment setup, selected libraries, project setup, a small starter example with detailed explanation, build instructions, and how the concepts are applied in this game. The tutorial must be consistent with the actual code and dependencies.
6. Implementation details for board rules, state management, AI, UI, save/load, and settings.
7. Testing strategy, commands, actual results, and known limitations.
8. Future improvements.

Do not pad the tutorial with repeated text to reach the length requirement. Do not claim features or test results that are not present.

### `docs/WORK_DIVISION.md`

Include team members, each member's actual responsibilities, and collaboration tools used. Leave editable placeholders where the team has not supplied details. Do not fabricate the team's names, workload, or tools.

### Final package

- Ensure the root README documents prerequisites, build steps, run steps, controls, save-file location, and known limitations while retaining any task-critical instructions needed by the team.
- Prepare a submission ZIP such as `GoGame_Submission.zip` containing source, headers, assets, tests, README, and documentation. Exclude build output, temporary files, and machine-specific cache directories.
- Verify that the ZIP contains the required files and that a clean build can be followed using the written instructions.

## 11. Completion response

When done, report:

- The main features implemented.
- The compiler/build command and run command.
- The tests actually executed and their results.
- The location of the ZIP and documentation files.
- Any known gaps that remain.

**Completion means a buildable, playable game with working core rules, three functioning AI levels, state management, settings, save/load, and submission documentation—not just a design proposal.**
