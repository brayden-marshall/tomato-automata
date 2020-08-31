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
    init_neighbourhood_offsets();

    current_cellular_automata_family = "Life";
    current_cellular_automata =
        cellular_automata[current_cellular_automata_family][0];

    update_colors();
    randomize_board();
}

void App::render(const ImGuiIO& io) {
    //
    // render main drawing
    //
    
    // clear the window with the background color
    // (colors[0] is always the background color)
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

    if (grid_enabled) {
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
    }

    //
    // render ImGui
    //

    ImGui::NewFrame();

    // paintbrush window
    {
        int old_selected_state = selected_state;
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

        ImGui::End();
    } 


    // controls window
    ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    if (ImGui::Button("Clear")) {
        clear_board();
    }

    ImGui::SameLine();
    if (ImGui::Button("Randomize")) {
        randomize_board();
    }

    ImGui::SameLine();
    if (ImGui::Button("Advance Once")) {
        advance_one_generation();
    }

    ImGui::SameLine();
    ImGui::Checkbox("Grid", &grid_enabled);

    int current_color_scheme_i = static_cast<int>(current_color_scheme);
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
            current_cellular_automata->name.c_str())
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
        next_color_index += floor((_colors.size()-1)/states);
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
                new Life("2x2", "125/36"),
                new Life("34 Life", "34/34"),
                new Life("Amoeba", "1358/357"),
                new Life("Assimilation", "4567/345"),
                new Life("Coagulations", "235678/378"),
                new Life("Conway's Life", "23/3"),
                new Life("Coral", "45678/3"),
                new Life("Day & Night", "34678/3678"),
                new Life("Diamoeba", "5678/35678"),
                new Life("Flakes", "012345678/3"),
                new Life("Gnarl", "1/1"),
                new Life("High Life", "23/36"),
                new Life("Inverse Life", "34678/0123478"),
                new Life("Long Life", "5/345"),
                new Life("Maze", "12345/3"),
                new Life("Mazectric", "1234/3"),
                new Life("Move", "245/368"),
                new Life("Pseudo Life", "238/357"),
                new Life("Replicator", "1357/1357"),
                new Life("Seeds (2)", "/2"),
                new Life("Serviettes", "/234"),
                new Life("Stains", "235678/3678"),
                new Life("Walled Cities", "2345/45678"),
            },
        },
        {
            "Generations", {
                new Generations("Banners", "2367/3457/5"),
                new Generations("BelZhab", "23/23/8"),
                new Generations("BelZhab Sediment", "145678/23/8"),
                new Generations("Bloomerang", "234/34678/24"),
                new Generations("Bombers", "345/24/25"),
                new Generations("Brain 6", "6/246/3"),
                new Generations("Brian's Brain", "/2/3"),
                new Generations("Burst", "0235678/3468/9"),
                new Generations("Burst II", "235678/3468/9"),
                new Generations("Caterpillars", "124567/378/4"),
                new Generations("Chenille", "05678/24567/6"),
                new Generations("Circuit Genesis", "2345/1234/8"),
                new Generations("Cooties", "23/2/8"),
                new Generations("Ebb & Flow", "012478/36/18"),
                new Generations("Ebb & Flow II", "012468/37/18"),
                new Generations("Faders", "2/2/25"),
                new Generations("Fireworks", "2/13/21"),
                new Generations("Flaming Starbows", "347/23/8"),
                new Generations("Frogs", "12/34/3"),
                new Generations("Frozen Spirals", "356/23/6"),
                new Generations("Glisserati", "035678/245678/7"),
                new Generations("Glissergy", "035678/245678/5"),
                new Generations("Lava", "12345/45678/8"),
                new Generations("Lines", "012345/458/3"),
                new Generations("LivingOnTheEdge", "345/3/6"),
                new Generations("Meteor Guns", "01245678/3/8"),
                new Generations("Nova", "45678/2478/25"),
                new Generations("OrthoGo", "3/2/4"),
                new Generations("Prairie Fire", "345/34/6"),
                new Generations("RainZha", "2/23/8"),
                new Generations("Rake", "3467/2678/6"),
                new Generations("SediMental", "45678/25678/4"),
                new Generations("Snake", "03467/25/6"),
                new Generations("SoftFreeze", "13458/38/6"),
                new Generations("Spirals", "2/234/5"),
                new Generations("Star Wars", "345/2/4"),
                new Generations("Sticks", "3456/2/6"),
                new Generations("Swirl", "23/34/8"),
                new Generations("ThrillGrill", "1234/34/48"),
                new Generations("Transers", "345/26/5"),
                new Generations("Transers II", "0345/26/6"),
                new Generations("Wanderers", "345/34678/5"),
                new Generations("Worms", "3467/25/6"),
                new Generations("Xtasy", "1456/2356/16"),
            },
        },
        {
            "Cyclic", {
                new Cyclic("313", "R1/T3/C3/NM"),
                new Cyclic("3-Color bootstrap", "R2/T11/C3/NM"),
                new Cyclic("Amoeba (cyclic)", "R3/T10/C2/NN"),
                new Cyclic("Black vs White", "R5/T23/C2/NN"),
                new Cyclic("CCA", "R1/T1/C14/NN"),
                new Cyclic("Cubism", "R2/T5/C3/NN"),
                new Cyclic("Cyclic Spirals", "R3/T5/C8/NM"),
                new Cyclic("Fossil Debris", "R2/T9/C4/NM"),
                new Cyclic("GH Macaroni", "R2/T4/C5/NM/GH"),
                new Cyclic("GH Multistrands", "R5/T15/C6/NM/GH"),
                new Cyclic("GH Percolation Mix", "R5/T10/C8/NM/GH"),
                new Cyclic("GH Weak Spirals", "R4/T9/C7/NM/GH"),
                new Cyclic("GH", "R3/T5/C8/NM/GH"),
                new Cyclic("Imperfect", "R1/T2/C4/NM"),
                new Cyclic("Lava Lamp", "R2/T10/C3/NM"),
                new Cyclic("Maps", "R2/T3/C5/NN"),
                new Cyclic("Perfect", "R1/T3/C4/NM"),
                new Cyclic("Squarish Spirals", "R2/T2/C6/NN"),
                new Cyclic("Stripes", "R3/T4/C5/NN"),
                new Cyclic("Turbulent Phase", "R2/T5/C8/NM"),
            }
        },
        {
            "LargerThanLife", {
                new LargerThanLife("Bugs", "R5,C0,M1,S34..58,B34..45,NM"),
                new LargerThanLife("BugsMovie", "R10,C0,M1,S123..212,B123..170,NM"),
                new LargerThanLife("Globe", "R8,C0,M0,S163..223,B74..252,NM"),
                new LargerThanLife("Gnarl", "R1,C0,M1,S1..1,B1..1,NN"),
                new LargerThanLife("Majority", "R4,C0,M1,S41..81,B41..81,NM"),
                new LargerThanLife("Majorly", "R7,C0,M1,S113..225,B113..225,NM"),
                new LargerThanLife("ModernArt", "R10,C255,M1,S2..3,B3..3,NM"),
                new LargerThanLife("Waffle", "R7,C0,M1,S100..200,B75..170,NM"),
            }
        },
        {
            "NeumannBinary", {
                new NeumannBinary("Aggregation", "3002000202000000000202000202000000000000000000000000000202000202000000000202000202001001111001001111111111111001001111001001111111111111111111111111111111111111111212021222020201010222221222012012010122011211000111111202122212121111111202111211"),

                new NeumannBinary("Birds", "3010112020112112222020222020112112222112102220222220202020222020222220202020202020112102222102000200222200202102000200000012020200020000222200202200020000202000200020220000220222020000020000220222020222222222020222020000020000020222020000020000"),

                new NeumannBinary("Colony", "3010102020102011210020210000102011210011102120210120000020210000210120000000000000112111212111110100212100200111110100110102021100021012212100200100021012200012020020222020222222222020222020222222222222222222222222222020222020222222222020222020"),

                new NeumannBinary("Crystal2", "201101101101101101111101011001000"),

                new NeumannBinary("Crystal3a", "3012101220100010100210200002102010000010121010100011002210000002200010012002002221111101111101000100111100100101000100000020000100000001111100100100000001100001011222222220222200200222200000222200200200022020200020000220200200200020000000000000"),

                new NeumannBinary("Crystal3b", "3012100200100011002200012020100020021021221021012020122200021010002212102020112021111111111111100100111100100111100100100010001100001011111100100100001011100011011222222222222200200222200200222200200200022022200020020222200200200022000200020000"),

                new NeumannBinary("Fredkin2", "201101001100101101001011001101001"),

                new NeumannBinary("Fredkin3", "3012120201120201012201012120120201012201012120012120201201012120012120201120201012120201012201012120012120201201012120012120201120201012012120201120201012201012120201012120012120201120201012012120201120201012201012120120201012201012120012120201"),

                new NeumannBinary("Galaxy", "3010112020112112220020220000112112220112110200220200000020220000220200000000000000002002222002000200222200200002000200000000002200002020222200200200002020200020002020220000220220000000000000220220000220220000000000000000000000000000000000000000"),

                new NeumannBinary("Greenberg", "3010110000110110000000000000110110000110110000000000000000000000000000000000000000222222222222222222222222222222222222222222222222222222222222222222222222222222222000000000000000000000000000000000000000000000000000000000000000000000000000000000"),

                new NeumannBinary("Honeycomb", "3010102020102002222020222020102002222002002220222220202020222020222220202020202020110112020112100202020202020112100202100002020202020200020202020202020200020200000020222020222222222020222020222222222222220202222202222020222020222202222020222020"),

                new NeumannBinary("Knitting", "3010112020112110202020202020112110202110110002202002221020202020202002221020221010102010202010102021202021211010102021102000200021200102202021211021200102211102120020222020222222222020222020222222222222222222222222222020222020222222222020222020"),

                new NeumannBinary("Lake", "3010112020112100202020202020112100202100010002202002220020202020202002220020220000012110202110100000202000200110100000100002022000022020202000200000022020200020000020222020222222222020222020222222222222222222222222222020222020222222222020222020"),

                new NeumannBinary("Plankton", "3010112020112112222020222020112112222112102220222220202020222020222220202020202020100002020002010202020202020002010202010102020202020200020202020202020200020200000020220000220222020000020000220222020222222222020222020000020000020222020000020000"),

                new NeumannBinary("Pond", "3010112020112112222020222020112112222112102220222220202020222020222220202020202020110102020102010202020202020102010202010112020202020200020202020202020200020200000020220000220222020000020000220222020222222222020222020000020000020222020000020000"),

                new NeumannBinary("Strata", "3000000200120200000000000000000000200020100000000000000000000100000000000000000000110000000110000010000000000000000000000000000000000000000000000000000000000000000000000000020000000000000000000000000000000000000000000000000000000000000000000000"),

                new NeumannBinary("Tanks", "3010112020112112222020222020112112222112102220222220202020222020222220202020202020110102020102010202020202020102010202010102020202020202020202020202020202020202020020222020222220200020200000222220200220222020200020000020200000200020000000000000"),

                new NeumannBinary("Typhoon", "3010112020112112220020220000112112220112110200220200000020220000220200000000000000002002222002000200222200200002000200000000002200002020222200200200002020200020002020222020222222222020222020222222222222222222222222222020222020222222222020222020"),

                new NeumannBinary("Wave", "3010112020112110202020202020112110202110102020202020202020202020202020202020202020112102222102000200222200202102000200000002020200020000222200202200020000202000200020220000220222020000020000220222020222222222020222020000020000020222020000020000"),

            }
        }
    };
}
