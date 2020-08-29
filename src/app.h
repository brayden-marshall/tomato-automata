#ifndef APP_H
#define APP_H

#include <SDL2/SDL.h>
#include <cstdint>
#include <unordered_map>
#include <array>
#include <random>

#include "../imgui/imgui.h"
#include "./common.h"

#define ANIMATION_SPEEDS_MAX 4
enum class AnimationSpeed {
    Slow,
    Medium,
    Fast,
    VeryFast,
};

#define COLORSCHEMES_MAX 2
enum class ColorScheme {
    Greyscale,
    RedGradient,
};

CellularAutomataMap load_cellular_automata();
std::vector<std::array<uint8_t, 3>> get_color_subset(
        const std::vector<std::array<uint8_t, 3>>& colors, int states
);
std::array<std::vector<std::array<uint8_t, 3>>, COLORSCHEMES_MAX>
load_colorschemes();

class App {
    private:
        // members
        Board board{};
        std::default_random_engine random_generator;
        int delay = 100;
        int timer = 0;

        uint8_t selected_state = 0;
        int brush_size = 1;

        // animation speed
        AnimationSpeed animation_speed = AnimationSpeed::Fast;
        std::array<const char*, ANIMATION_SPEEDS_MAX> animation_speed_names
            {"Slow", "Medium", "Fast", "Very Fast"};
        std::array<int, ANIMATION_SPEEDS_MAX> animation_speed_delays
            {250, 150, 100, 50};

        // color scheme
        std::array<const char*, COLORSCHEMES_MAX> color_scheme_names
            {"Greyscale", "Red Gradient"};
        std::array<ColorPalette, COLORSCHEMES_MAX> color_schemes;
        ColorScheme current_color_scheme = ColorScheme::Greyscale;
        ColorPalette colors;

        CellularAutomataMap cellular_automata;
        CellularAutomata* current_cellular_automata;
        std::string current_cellular_automata_family;

        // functions
        void randomize_board();
        void clear_board();
        void update_colors();

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
