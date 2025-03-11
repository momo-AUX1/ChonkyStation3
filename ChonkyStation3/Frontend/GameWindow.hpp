#pragma once

#include <common.hpp>

#include <string>
#include <format>

#include <SDL.h>
#include <PlayStation3.hpp>


class GameWindow {
public:
    #ifdef __XBOX_BUILD
    GameWindow(SDL_Window* externalWindow);
    #else
    GameWindow();
    #endif
    void run(PlayStation3* ps3);
    void flipHandler();

    PlayStation3* ps3;
    std::string title_game;
    SDL_Window* window;
    SDL_GameController* controller;

    bool quit = false;
    bool fullscreen = false;
    bool vsync_enabled = false;
    int frame_count = 0;
    double last_time = 0;
    double curr_time = 0;
    float ppu_usage = 0;
    u64 last_timestamp = 0;
};