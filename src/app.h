#ifndef APP_H
#define APP_H

#include <SDL2/SDL.h>
#include <cstdint>
#include <array>
#include <random>

#include "../imgui/imgui.h"

#define BOARD_ROWS 100
#define BOARD_COLS 100

class App {
    private:
        // members
        std::array<std::array<uint8_t, BOARD_COLS>, BOARD_ROWS> board = {};
        std::default_random_engine random_generator;
        int delay = 100;
        int timer = 0;

        uint8_t selected_state = 0;
        int brush_size = 1;

        // functions
        void randomize_board();

    public:
        // members
        SDL_Renderer* renderer;
        bool paused = true;

        // constructor
        App(SDL_Renderer* r);

        // functions
        void render(const ImGuiIO& io);
        void update(const ImGuiIO& io, int dt);
        void advance_one_generation();
};

#endif
