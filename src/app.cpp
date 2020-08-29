#include <iostream>
using std::cout;
using std::endl;

#include <SDL2/SDL.h>
#include <stdio.h>
#include <cmath>

#include "app.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_sdl.h"
#include "./common.h"
#include "./automata/automata.h"

// public methods
App::App(SDL_Renderer* r): renderer(r) {
    cellular_automata = load_cellular_automata();
    color_schemes = load_colorschemes();

    current_cellular_automata_family = "Life";
    current_cellular_automata =
        cellular_automata[current_cellular_automata_family][0];

    update_colors();
    randomize_board();
}

void App::render(const ImGuiIO& io) {
    auto scheme = color_schemes[static_cast<int>(current_color_scheme)];
    int current_color_scheme_i = static_cast<int>(current_color_scheme);

    // render main drawing
    SDL_SetRenderDrawColor(
        renderer,
        colors[0][0],
        colors[0][1],
        colors[0][2],
        255
    );
    SDL_RenderClear(renderer);

    auto display_width = io.DisplaySize.x;
    auto display_height = io.DisplaySize.y;
    auto cell_width = display_width / BOARD_COLS;
    auto cell_height = display_height / BOARD_ROWS;

    // fill in squares
    for (int row = 0; row < BOARD_ROWS; row++) {
        for (int col = 0; col < BOARD_COLS; col++) {
            uint8_t val = board[row][col];
            SDL_SetRenderDrawColor(
                renderer,
                colors[val][0],
                colors[val][1],
                colors[val][2],
                255
            );

            SDL_Rect rect;
            rect.x = col * cell_width;
            rect.y = row * cell_height;
            rect.w = cell_width;
            rect.h = cell_height;
            SDL_RenderFillRect(renderer, &rect);
        }
    }

    SDL_SetRenderDrawColor(renderer, 70, 70, 70, 255);
    // draw vertical grid lines
    for (int col = 1; col < BOARD_COLS; col++) {
        SDL_RenderDrawLine(renderer,
            col * cell_width, 0,
            col * cell_width, display_width
        );
    }

    // draw horizontal grid lines
    for (int row = 1; row < BOARD_ROWS; row++) {
        SDL_RenderDrawLine(renderer,
            0, row * cell_height,
            display_height, row * cell_height
        );
    }

    // render ImGui
    ImGui::NewFrame();

    int old_selected_state = selected_state;

    // paintbrush window
    ImGui::Begin("Paintbrush");
    for (int i = 0; i < current_cellular_automata->num_states; i++) {
        ImGui::PushID(i);

        if (ImGui::ColorButton("", ImVec4(
                colors[i][0]/255.0,
                colors[i][1]/255.0,
                colors[i][2]/255.0,
                255
        ))) {
            selected_state = i;
        }

        if (old_selected_state == i) {
            ImGui::SameLine();
            ImGui::Text("Active");
        }

        ImGui::PopID();
    }

    // end paintbrush window
    ImGui::End();

    // controls window
    bool question = false;
    ImGui::Begin("Controls", &question, ImGuiWindowFlags_AlwaysAutoResize);

    if (ImGui::Button("Clear")) {
        clear_board();
    }

    if (ImGui::Button("Randomize")) {
        randomize_board();
    }

    if (ImGui::Button("Advance Once")) {
        advance_one_generation();
    }

    if (ImGui::BeginCombo(
            "Color Scheme",
            color_scheme_names[current_color_scheme_i]
    )) {
        int selected = -1;

        for (int i = 0; i < COLORSCHEMES_MAX; i++) {
            if (ImGui::Selectable(
                    color_scheme_names[i], current_color_scheme_i == i)
            ) {
                selected = i;
            }
        }

        if (selected != -1) {
            current_color_scheme = static_cast<ColorScheme>(selected);
            update_colors();
        }

        ImGui::EndCombo();
    }

    int animation_speed_i = static_cast<int>(animation_speed);
    if (ImGui::BeginCombo(
            "Animation Speed",
            animation_speed_names[animation_speed_i]
    )) {
        int selected = -1;

        for (int i = 0; i < ANIMATION_SPEEDS_MAX; i++) {
            if (ImGui::Selectable(animation_speed_names[i], animation_speed_i == i)) {
                selected = i;
            }
        }

        if (selected != -1) {
            animation_speed = static_cast<AnimationSpeed>(selected);
        }

        ImGui::EndCombo();
    }

    if (ImGui::BeginCombo(
            "Automata Family",
            current_cellular_automata_family.c_str())
    ) {
        int i = 0;
        for (auto& family : cellular_automata) {
            ImGui::PushID(i);

            const char* name = family.first.c_str();
            if (ImGui::Selectable(
                    name, name == current_cellular_automata_family)
            ) {
                current_cellular_automata_family = name;
                current_cellular_automata = cellular_automata[name][0];
                clear_board();
                update_colors();
            }
            ImGui::PopID();
            i++;
        }

        ImGui::EndCombo();
    }

    if (ImGui::BeginCombo(
            "Automata Type",
            cellular_automata[current_cellular_automata_family][0]->name.c_str())
    ) {

        int i = 0;
        for (auto& automata :
                   cellular_automata[current_cellular_automata_family]
        ) {
            if (ImGui::Selectable(
                    automata->name.c_str(),
                    automata == current_cellular_automata)) {
                clear_board();
                current_cellular_automata = automata;
                update_colors();
            }

            i++;
        }

        ImGui::EndCombo();
    }

    // end controls window
    ImGui::End();

    ImGui::Render();
    ImGuiSDL::Render(ImGui::GetDrawData());

    SDL_RenderPresent(renderer);
}

// dt is the delta time in milliseconds
void App::update(const ImGuiIO& io, int dt) {
    if (!paused) {
        timer += dt;
        if (timer >= animation_speed_delays[static_cast<int>(animation_speed)]) {
            timer = 0;
            advance_one_generation();
        }
    }

    // if mouse was pressed
    if (io.MouseDown[0] || io.MouseDown[1]) {
        // get the row/col from mouse position
        double cell_width = io.DisplaySize.x / BOARD_COLS;
        double cell_height = io.DisplaySize.y / BOARD_ROWS;

        int clicked_col = io.MousePos.x / cell_width;
        int clicked_row = io.MousePos.y / cell_height;

        board[clicked_row][clicked_col] =
            io.MouseDown[0] ? selected_state : 0;
    }
}

void App::advance_one_generation() {
    auto result = current_cellular_automata->rewrite(board);
    board = result.first;

    bool change_made = result.second;
    if (!change_made) {
        paused = true;
    }
}

// private methods
void App::randomize_board() {
    std::uniform_int_distribution<uint8_t> distribution(
        0, current_cellular_automata->num_states-1);
    for (int row = 0; row < BOARD_ROWS; row++) {
        for (int col = 0; col < BOARD_COLS; col++) {
            board[row][col] = distribution(random_generator);
        }
    }
}

void App::clear_board() {
    for (int row = 0; row < BOARD_ROWS; row++) {
        board[row].fill(0);
    }
}

void App::update_colors() {
    colors = get_color_subset(
        color_schemes[static_cast<int>(current_color_scheme)],
        current_cellular_automata->num_states
    );
}

// functions
ColorPalette get_color_subset(
        const ColorPalette& colors, int states
) {
    auto _colors = colors;
    while (_colors.size() < states) {
        _colors.insert(_colors.end(), colors.begin(), colors.end());
    }

    std::vector<std::array<uint8_t, 3>> color_subset;
    color_subset.push_back(_colors[0]);

    int next_color_index = 1;
    for (int i = 1; i < states; i++) {
        color_subset.push_back(_colors[next_color_index]);
        next_color_index = floor((_colors.size()-1)/states);
    }

    return color_subset;
}

std::array<ColorPalette, COLORSCHEMES_MAX>
load_colorschemes() {
    return {{
        {{0xff, 0xff, 0xff}, {0x00, 0x00, 0x00}, {0x08, 0x08, 0x08},
         {0x0f, 0x0f, 0x0f}, {0x17, 0x17, 0x17}, {0x1f, 0x1f, 0x1f},
         {0x27, 0x27, 0x27}, {0x2e, 0x2e, 0x2e}, {0x36, 0x36, 0x36},
         {0x3e, 0x3e, 0x3e}, {0x46, 0x46, 0x46}, {0x4d, 0x4d, 0x4d},
         {0x55, 0x55, 0x55}, {0x5d, 0x5d, 0x5d}, {0x64, 0x64, 0x64},
         {0x6c, 0x6c, 0x6c}, {0x74, 0x74, 0x74}, {0x7c, 0x7c, 0x7c},
         {0x83, 0x83, 0x83}, {0x8b, 0x8b, 0x8b}, {0x93, 0x93, 0x93},
         {0x9b, 0x9b, 0x9b}, {0xa2, 0xa2, 0xa2}, {0xaa, 0xaa, 0xaa},
         {0xb2, 0xb2, 0xb2}, {0xb9, 0xb9, 0xb9}, {0xc1, 0xc1, 0xc1},
         {0xc9, 0xc9, 0xc9}, {0xd1, 0xd1, 0xd1}, {0xd8, 0xd8, 0xd8},
         {0xe0, 0xe0, 0xe0}, {0xe8, 0xe8, 0xe8}, {0xf0, 0xf0, 0xf0},
         {0xf7, 0xf7, 0xf7}
        },
        {{0x00, 0x00, 0x00}, {0xE5, 0x00, 0x35}, {0xE3, 0x00, 0x50},
         {0xE2, 0x00, 0x6B}, {0xE1, 0x00, 0x85}, {0xE0, 0x00, 0xA0},
         {0xDE, 0x00, 0xBA}, {0xDD, 0x00, 0xD4}, {0xCB, 0x00, 0xDC},
         {0xAF, 0x00, 0xDB}, {0x94, 0x00, 0xD9}, {0x78, 0x00, 0xD8},
         {0x5E, 0x00, 0xD7}, {0x43, 0x00, 0xD6}, {0x29, 0x00, 0xD5},
         {0x0F, 0x00, 0xD3}, {0x00, 0x09, 0xD2}, {0x00, 0x23, 0xD1},
         {0x00, 0x3C, 0xD0}, {0x00, 0x54, 0xCE}, {0x00, 0x6D, 0xCD},
         {0x00, 0x85, 0xCC}, {0x00, 0x9D, 0xCB}, {0x00, 0xB4, 0xCA},
         {0x00, 0xC8, 0xC5}, {0x00, 0xC7, 0xAC}, {0x00, 0xC6, 0x93},
         {0x00, 0xC5, 0x7A}, {0x00, 0xC3, 0x61}, {0x00, 0xC2, 0x49},
         {0x00, 0xC1, 0x31}, {0x00, 0xC0, 0x1A}, {0x00, 0xBF, 0x03}
        }
    }};
}

CellularAutomataMap load_cellular_automata() {
    return {
        {
            "Life", {
                new Life("Conway's Life", "23/3")
            },
        },
        {
            "Generations", {
                new Generations("Brian's Brain", "/2/3")
            },
        }
    };
}
