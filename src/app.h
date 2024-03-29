#ifndef APP_H
#define APP_H

#include <SDL2/SDL.h>
#include <cstdint>
#include <unordered_map>
#include <array>
#include <random>

#include "../imgui/imgui.h"
#include "./common.h"

#define ANIMATION_SPEEDS_MAX 5
enum class AnimationSpeed {
    Slow,
    Medium,
    Fast,
    VeryFast,
    Lightning,
};

#define COLORSCHEMES_MAX 2
enum class ColorScheme {
    Greyscale,
    RedGradient,
};

CellularAutomataMap load_cellular_automata();
std::vector<std::array<uint8_t, 3>> get_color_subset(
        const std::vector<std::array<uint8_t, 3>>& colors, size_t states
);
std::array<std::vector<std::array<uint8_t, 3>>, COLORSCHEMES_MAX>
load_colorschemes();

class App {
    private:
        // members
        Board board{};
        std::default_random_engine random_generator;
        int timer = 0;
        bool grid_enabled = true;

        uint8_t selected_state = 0;
        int brush_size = 1;

        std::array<const char*, ANIMATION_SPEEDS_MAX> animation_speed_names
            {"Slow", "Medium", "Fast", "Very Fast", "Lightning"};
        std::array<int, ANIMATION_SPEEDS_MAX> animation_speed_delays
            {250, 150, 100, 50, 0};

        // color scheme
        std::array<const char*, COLORSCHEMES_MAX> color_scheme_names
            {"Greyscale", "Red Gradient"};
        std::array<ColorPalette, COLORSCHEMES_MAX> color_schemes;
        ColorScheme current_color_scheme = ColorScheme::Greyscale;
        ColorPalette colors;

        // cellular automata and automata family
        CellularAutomataMap cellular_automata;
        CellularAutomata* current_cellular_automata;
        std::string current_cellular_automata_family;

        // functions
        void randomize_board();
        void clear_board();
        void render_gui();
        void update_colors();

    public:
        // members
        SDL_Renderer* renderer;
        AnimationSpeed animation_speed = AnimationSpeed::Fast;
        bool paused = true;
        bool show_gui = true;
        bool show_help_menu = false;

        // constructor
        App(SDL_Renderer* r);

        // functions
        void render(const ImGuiIO& io);
        void update(const ImGuiIO& io, int dt);
        void advance_one_generation();
};

#endif
