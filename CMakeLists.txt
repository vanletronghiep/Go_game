cmake_minimum_required(VERSION 3.20)

project(GoGame VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(GOGAME_BUILD_APP "Build the SFML desktop application" ON)
option(GOGAME_BUILD_TESTS "Build the rules and AI test suite" ON)

find_package(Threads REQUIRED)

add_library(gocore STATIC
    src/Types.cpp
    src/Board.cpp
    src/Position.cpp
    src/Scoring.cpp
    src/Game.cpp
    src/Evaluator.cpp
    src/AI.cpp
    src/SaveManager.cpp
    src/Settings.cpp
    src/Paths.cpp
)
target_include_directories(gocore PUBLIC include)
target_compile_definitions(gocore PRIVATE GO_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
target_link_libraries(gocore PUBLIC Threads::Threads)

if(MSVC)
    target_compile_options(gocore PUBLIC /W4 /permissive-)
else()
    target_compile_options(gocore PUBLIC -Wall -Wextra -Wpedantic)
endif()

if(GOGAME_BUILD_APP)
    find_package(SFML 3 REQUIRED COMPONENTS Graphics Audio)
    add_executable(GoGame
        src/main.cpp
        src/Audio.cpp
        src/Gui.cpp
        src/Theme.cpp
        src/BoardView.cpp
        src/UI.cpp
        src/Screens.cpp
    )
    target_link_libraries(GoGame PRIVATE gocore SFML::Graphics SFML::Audio)
    if(WIN32)
        find_package(SFML 3 COMPONENTS Main)
        if(TARGET SFML::Main)
            target_link_libraries(GoGame PRIVATE SFML::Main)
            set_target_properties(GoGame PROPERTIES WIN32_EXECUTABLE ON)
        endif()
    endif()
endif()

if(GOGAME_BUILD_TESTS)
    enable_testing()
    add_executable(game_tests tests/game_tests.cpp)
    target_link_libraries(game_tests PRIVATE gocore)
    add_test(NAME game_tests COMMAND game_tests)
endif()
