#include <iostream>
using std::cout;
using std::endl;

#include <SDL2/SDL.h>
#include <stdio.h>

#include "app.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_sdl.h"

// public functions
App::App(SDL_Renderer* r): renderer(r) {
    randomize_board();
    //board[0][0] = 1;
}

void App::render(const ImGuiIO& io) {
    // render main drawing
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    auto display_width = io.DisplaySize.x;
    auto display_height = io.DisplaySize.y;
    auto cell_width = display_width / BOARD_COLS;
    auto cell_height = display_height / BOARD_ROWS;

    // fill in squares
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    for (int row = 0; row < BOARD_ROWS; row++) {
        for (int col = 0; col < BOARD_COLS; col++) {
            if (board[row][col] != 0) {
                SDL_Rect rect;
                rect.x = col * cell_width;
                rect.y = row * cell_height;
                rect.w = cell_width;
                rect.h = cell_height;
                SDL_RenderFillRect(renderer, &rect);
            }
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

    ImGui::Begin("Paintbrush");
    ImGui::SliderInt("Brush Size", &brush_size, 1, 10);
    ImGui::Separator();

    if (ImGui::ColorButton("White", ImVec4(255, 255, 255, 255))) {
        selected_state = 0;
    }

    if (old_selected_state == 0) {
        ImGui::SameLine();
        ImGui::Text("Active");
    }

    if (ImGui::ColorButton("Black", ImVec4(0, 0, 0, 255))) {
        selected_state = 1;
    }

    if (old_selected_state == 1) {
        ImGui::SameLine();
        ImGui::Text("Active");
    }

    ImGui::End();

    ImGui::Render();
    ImGuiSDL::Render(ImGui::GetDrawData());

    SDL_RenderPresent(renderer);
}

// dt is the delta time in milliseconds
void App::update(const ImGuiIO& io, int dt) {
    if (!paused) {
        timer += dt;
        //cout << "timer is: " << timer << endl;
        if (timer >= delay) {
            timer = 0;
            advance_one_generation();
        }
    }

    // if left mouse was pressed
    if (io.MouseDown[0]) {
        // get the row/col from mouse position
        double cell_width = io.DisplaySize.x / BOARD_COLS;
        double cell_height = io.DisplaySize.y / BOARD_ROWS;

        int clicked_col = io.MousePos.x / cell_width;
        int clicked_row = io.MousePos.y / cell_height;

        board[clicked_row][clicked_col] = selected_state;
    }
}

inline int modulo(int a, int b) {
    return ((a % b) + b) % b;
}

void App::advance_one_generation() {
    auto board_copy = board;

    for (int row = 0; row < BOARD_ROWS; row++) {
        for (int col = 0; col < BOARD_COLS; col++) {

            // count neighbours
            uint8_t neighbour_count = 0;
            for (int row_offset = -1; row_offset <= 1; row_offset++) {
                for (int col_offset = -1; col_offset <= 1; col_offset++) {
                    if (row_offset == 0 && col_offset == 0) {
                        continue;
                    }

                    auto neighbour_row = modulo(row + row_offset, BOARD_ROWS);
                    auto neighbour_col = modulo(col + col_offset, BOARD_COLS);

                    neighbour_count += board[neighbour_row][neighbour_col];
                }
            }

            if (board[row][col] == 0) {
                if (neighbour_count == 3) {
                    board_copy[row][col] = 1;
                }
            } else {
                if (neighbour_count < 2 || neighbour_count > 3) {
                    board_copy[row][col] = 0;
                }
            }
        }
    }

    board = board_copy;
}

// private functions
void App::randomize_board() {
    std::uniform_int_distribution<uint8_t> distribution(0, 1);
    for (int row = 0; row < BOARD_ROWS; row++) {
        for (int col = 0; col < BOARD_COLS; col++) {
            board[row][col] = distribution(random_generator);
        }
    }
}
