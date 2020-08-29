#include <iostream>
using std::cout;
using std::endl;

#include <SDL2/SDL.h>
#include <stdio.h>

#include "app.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_sdl.h"
#include "./common.h"
#include "./automata/automata.h"

// public methods
App::App(SDL_Renderer* r): renderer(r) {
    randomize_board();
    cellular_automata = load_cellular_automata();
    color_schemes = load_colorschemes();

    current_cellular_automata_family = "Life";
    current_cellular_automata =
        cellular_automata[current_cellular_automata_family][0];
}

void App::render(const ImGuiIO& io) {
    // render main drawing
    auto scheme = color_schemes[static_cast<int>(current_color_scheme)];
    int current_color_scheme_i = static_cast<int>(current_color_scheme);

    SDL_SetRenderDrawColor(
        renderer,
        scheme[0][0],
        scheme[0][1],
        scheme[0][2],
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
                scheme[val][0],
                scheme[val][1],
                scheme[val][2],
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

    // draw grid
    SDL_SetRenderDrawColor(renderer, 70, 70, 70, 255);
    // draw vertical lines
    for (int col = 1; col < BOARD_COLS; col++) {
        SDL_RenderDrawLine(renderer,
            col * cell_width, 0,
            col * cell_width, display_width
        );
    }

    // draw horizontal lines
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
    for (int i = 0; i < scheme.size(); i++) {
        ImGui::PushID(i);

        if (ImGui::ColorButton(
                "",
                ImVec4(scheme[i][0], scheme[i][1], scheme[i][2], 255))
        ) {
            std::cout << "selected_state was: " << i << std::endl;
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

    if (ImGui::BeginCombo("Automata Family", "Life")) {

        ImGui::Selectable("Life", true);

        ImGui::EndCombo();
    }

    if (ImGui::BeginCombo("Automata Type", "Conway's Life")) {

        ImGui::Selectable("Conway's Life", true);

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
    std::uniform_int_distribution<uint8_t> distribution(0, 1);
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

std::array<std::vector<std::array<uint8_t, 3>>, COLORSCHEMES_MAX>
load_colorschemes() {
    return {{
        { {255, 255, 255}, {0, 0, 0}},
        { {0, 0, 0}, {255, 0, 0} }
    }};
}

CellularAutomataMap load_cellular_automata() {
    return {
        {
            "Life", {
                new Life("Conway's Life", "23/3")
            }
        }
    };
}
