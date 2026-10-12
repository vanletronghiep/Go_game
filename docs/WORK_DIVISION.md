# Work Division Document

## Project Information

- **Course:** Introduction to Programming / CS160
- **Project:** Final Project - Go Game
- **Institution:** Department of Software Engineering - FIT - HCMUS
- **Academic Year:** 2025 - 2026

---

## 1. Team Members & Contributions

*Note: Please update the member placeholders below with the actual student names and student IDs.*

| Member Name | Student ID | Primary Responsibilities | Module Focus | Percentage Contribution |
| :--- | :--- | :--- | :--- | :--- |
| `[Member 1 Name - e.g., Le Van Trong Hiep]` | `[Student ID 1]` | Game Engine & Core Rules, Data Types | `Board.cpp`, `Position.cpp`, `Scoring.cpp`, `Types.cpp` | `[25% / Placeholder]` |
| `[Member 2 Name - Placeholder]` | `[Student ID 2]` | AI Opponent Engine & Minimax / Alpha-Beta | `AI.cpp`, `Evaluator.cpp` | `[25% / Placeholder]` |
| `[Member 3 Name - Placeholder]` | `[Student ID 3]` | Graphical User Interface & Audio Integration | `UI.cpp`, `BoardView.cpp`, `Gui.cpp`, `Screens.cpp`, `Audio.cpp`, `Theme.cpp` | `[25% / Placeholder]` |
| `[Member 4 Name - Placeholder]` | `[Student ID 4]` | Persistence, Testing Suite & Documentation | `SaveManager.cpp`, `Settings.cpp`, `game_tests.cpp`, Report & Tutorial | `[25% / Placeholder]` |

---

## 2. Detailed Task Breakdown

### Member 1: Game Engine & Core Rules
- Implementation of discrete grid and board representation (`Board.h`, `Board.cpp`).
- Group tracking, flood-fill liberty calculation, and capture detection.
- Rules enforcement: move legality, suicide move prohibition, and positional superko hashing (`Position.h`, `Position.cpp`).
- Area-based end-game scoring and territory flood fill with dead stone handling (`Scoring.h`, `Scoring.cpp`).

### Member 2: AI Opponent Development
- Heuristic board evaluator: material count, eye shapes, tactical liberties, and territorial influence (`Evaluator.h`, `Evaluator.cpp`).
- Easy AI: legal move generation and randomized candidate selection.
- Medium AI: shallow Minimax depth search with board evaluation.
- Hard AI: iterative-deepening Minimax with Alpha-Beta pruning, candidate ranking, and asynchronous background worker execution (`AI.h`, `AI.cpp`).

### Member 3: User Interface & Audio
- Desktop graphical application window setup with SFML 3 (`UI.h`, `UI.cpp`).
- Interactive board rendering with wood themes, grid lines, star points, and coordinate labels (`BoardView.h`, `BoardView.cpp`).
- Stone rendering engine featuring Classic, Modern, and Cartoon design styles (`Theme.h`, `Theme.cpp`).
- Modal dialogs for New Game configuration, Save/Load file management, Settings, and Scoring review (`Screens.h`, `Screens.cpp`).
- Reusable UI widgets: buttons, sliders, segmented controls, text inputs, and notifications (`Gui.h`, `Gui.cpp`).
- Procedural audio asset generation and audio manager integration (`Audio.h`, `Audio.cpp`).

### Member 4: Persistence, Testing & Documentation
- Save and load serialization with versioned plain-text schema and strict validation (`SaveManager.h`, `SaveManager.cpp`).
- User settings model with persistent configuration files (`Settings.h`, `Settings.cpp`).
- Comprehensive headless automated test suite covering all rules, AI constraints, and round-trips (`tests/game_tests.cpp`).
- CMake build configuration setup (`CMakeLists.txt`).
- Project documentation: comprehensive report, architecture diagrams, work division, and C++ game development tutorial (`docs/PROJECT_REPORT.md`, `README.md`).

---

## 3. Collaboration Tools & Workflow

- **Version Control:** Git & GitHub repository for source code versioning, branch management, and pull request reviews.
- **Build & Compilation:** CMake 3.20+ with Ninja build generator and GCC / MinGW-w64 (UCRT64).
- **GUI & Multimedia Framework:** SFML 3 (Simple and Fast Multimedia Library).
- **Communication:** `[e.g., Discord / Microsoft Teams / Zalo - Placeholder]`.
- **Project Tracking:** `[e.g., GitHub Projects / Trello - Placeholder]`.
- **Documentation Preparation:** Markdown and pandoc / PDF export tools.
