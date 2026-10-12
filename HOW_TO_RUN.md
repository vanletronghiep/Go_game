# How to Build and Run the Go Game

This guide provides step-by-step instructions for installing dependencies, compiling the project, running the desktop game, executing the test suite, and using all in-game controls.

---

## 1. System Requirements & Prerequisites

### 1.1 Software Requirements
- **Language Standard:** C++20 or newer
- **Build System:** CMake 3.20 or newer
- **Build Generator:** Ninja (recommended) or MinGW Makefiles / Make
- **Multimedia Framework:** SFML 3.0.2 (Graphics, Audio, Window, System)

---

### 1.2 Installing Dependencies

#### Windows (Recommended via MSYS2 UCRT64)
1. Download and install **MSYS2** from [https://www.msys2.org/](https://www.msys2.org/).
2. Open the **MSYS2 UCRT64** terminal and run:
   ```bash
   pacman -Syu
   pacman -S --needed mingw-w64-ucrt-x86_64-gcc \
                      mingw-w64-ucrt-x86_64-g++ \
                      mingw-w64-ucrt-x86_64-cmake \
                      mingw-w64-ucrt-x86_64-ninja \
                      mingw-w64-ucrt-x86_64-sfml
   ```
3. Add `C:\msys64\ucrt64\bin` to your system `PATH`.

#### Linux (Ubuntu / Debian)
```bash
sudo apt update
sudo apt install -y build-essential cmake ninja-build \
                    libfreetype-dev libopenal-dev libflac-dev libvorbis-dev \
                    libgl1-mesa-dev libx11-dev libxrandr-dev libxcursor-dev libudev-dev
```
*Note: If your distribution package manager provides SFML 2.x, install SFML 3 from source using CMake.*

#### macOS (Homebrew)
```bash
brew install cmake ninja sfml
```

---

## 2. Building the Project

Open a terminal (PowerShell, Command Prompt, or Bash) in the project root directory:

### Step 1: Configure CMake
```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
```
*(If Ninja is not installed, omit `-G Ninja` to use the default generator)*

### Step 2: Compile the Binaries
```bash
cmake --build build --config Release
```

This compiles two executables:
1. `build/GoGame.exe` (or `build/GoGame` on Linux/macOS) — The desktop graphical game.
2. `build/game_tests.exe` (or `build/game_tests` on Linux/macOS) — The automated headless test suite.

---

## 3. Running the Game & Tests

### 3.1 Launching the Desktop Game
```powershell
# On Windows (PowerShell):
./build/GoGame.exe

# On Linux / macOS:
./build/GoGame
```

### 3.2 Running the Automated Test Suite
The test suite runs headlessly in the console without needing a display server or window:
```powershell
# On Windows (PowerShell):
./build/game_tests.exe

# On Linux / macOS:
./build/game_tests
```
You should see:
```text
30/30 tests passed, 410/410 checks passed
```

---

## 4. In-Game Controls & Gameplay Guide

### 4.1 Game Modes
- **Player vs Player (PvP):** Two people play locally on the same device.
  - Player 1 is White, Player 2 is Black.
  - Black makes the first move.
- **Player vs AI:** One person plays against the computer.
  - Choose between **Easy**, **Medium**, or **Hard** AI difficulty.
  - Choose to play as Black (moves first) or White.

### 4.2 Board Interactions
- **Place a Stone:** Hover over any empty intersection to see a ghost preview. Left-click to place a stone.
- **Illegal Moves:** Occupied intersections, suicide moves, and positional superko repetitions are blocked and display an explanatory banner (with an error sound).
- **Pass:** Click the **Pass** button on the right-hand panel. Passing yields your turn to your opponent.
- **End-Game Scoring Review:** When both players pass consecutively, the game transitions into **Scoring Review**:
  - The board displays territory ownership markers.
  - **Click on stones** directly on the board to mark/unmark dead groups (marked dead stones show a red cross).
  - Click **Accept Score** to finalize the match, or **Resume Game** to return to play.

### 4.3 Side Panel Actions
| Button | Action |
| :--- | :--- |
| **Pass** | Passes the current player's turn. |
| **Undo** | Reverts the last move. In Player vs AI, it undoes the AI response and human move together as one exchange. |
| **Redo** | Reapplies the last undone move. |
| **Reset** | Clears the board back to the opening position. |
| **New Game** | Opens the dialog to pick board size (19x19, 13x13, 9x9), mode, and AI difficulty. |
| **Save** | Opens the save dialog to write the full game state to `saves/<filename>.gosave`. |
| **Load** | Opens a file chooser displaying saved games with date and move count. |
| **Settings** | Opens customization settings. |
| **Exit** | Safely saves settings and closes the application. |

### 4.4 Customization & Settings Dialog
- **Board Themes:** Choose between *Kaya Wood*, *Bamboo*, *Walnut*, *Slate Blue*, or *Jade*.
- **Stone Styles:** Choose between *Classic* (slate & shell sheen), *Modern* (flat minimalist with center pips), or *Cartoon* (bold outlines with glossy reflection).
- **Sound Effects:** Toggle on/off, choose theme (*Wooden*, *Glass*, *Retro*), and adjust volume.
- **Background Music:** Toggle on/off, select music track (*Zen Garden*, *Night Rain*, *Mountain Stream*), and adjust volume.
- **Coordinates:** Toggle edge coordinate labels (A–T, 1–19).

---

## 5. File Locations

- **Save Files:** Saved under the `saves/` folder (e.g., `saves/my_game.gosave`).
- **User Settings:** Persisted in `settings.cfg` in the project root.
- **Fonts & Audio Assets:** Located under `assets/fonts/` and `assets/audio/`.

---

## 6. Troubleshooting

- **Audio does not play:** Ensure audio drivers are enabled. The game automatically handles missing or disabled audio without crashing.
- **Window scaling on high-DPI displays:** The window is fully resizable. Drag the window corners to adjust the board size dynamically to your screen.
- **Missing DLLs on Windows:** If running outside MSYS2 terminal, ensure `C:\msys64\ucrt64\bin` is in your `PATH` so `libsfml-graphics-3.dll`, `libsfml-audio-3.dll`, etc., are discoverable.
