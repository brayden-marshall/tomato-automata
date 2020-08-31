#include <string>
#include <sstream>
#include <iostream>

#include "./common.h"
#include "./automata/automata.h"

// set the range limit for generating offsets
// currently, no rulesets require greater than 10 range
#define MAX_NEIGHBOURHOOD_RANGE 10

std::vector<std::vector<std::vector<int8_t>>> moore_offsets;
std::vector<std::vector<std::vector<int8_t>>> von_neumann_offsets;
std::map<std::string, std::vector<int>> direction_to_offset;

void init_neighbourhood_offsets() {
    moore_offsets =
        generate_all_neighbourhood_offsets(NeighbourhoodType::Moore);
    von_neumann_offsets =
        generate_all_neighbourhood_offsets(NeighbourhoodType::VonNeumann);
    direction_to_offset = {
        {"NW", { -1, -1 }},
        {"NW", {-1, -1}},
        {"NN", {-1, 0}},
        {"NE", {-1, 1}},
        {"WW", {0, -1}},
        {"EE", {0, 1}},
        {"SW", {1, -1}},
        {"SS", {1, 0}},
        {"SE", {1, 1}},
        {"ME", {0, 0}},
    };
}

std::vector<std::string> split(const std::string& s, char delimiter)
{
   std::vector<std::string> tokens;
   std::string token;
   std::istringstream token_stream(s);
   while (std::getline(token_stream, token, delimiter))
   {
      tokens.push_back(token);
   }
   return tokens;
}

CellularAutomata::CellularAutomata() {
}

CellularAutomata::CellularAutomata(std::string name, std::string rules)
    : name(name), rules(rules) {
}

std::vector<std::vector<int8_t>> generate_neighbourhood_offsets(
    NeighbourhoodType neighbourhood_type, int range
) {
    std::vector<std::vector<int8_t>> offsets;

    for (int row = -range; row <= range; row++) {
        for (int col = -range; col <= range; col++) {
            if (row == 0 && col == 0) {
                continue;
            }

            switch (neighbourhood_type) {
                case NeighbourhoodType::Moore:
                    offsets.push_back({
                        static_cast<int8_t>(row), static_cast<int8_t>(col)
                    });
                    break;
                case NeighbourhoodType::VonNeumann:
                    if (abs(row) + abs(col) <= range) {
                        offsets.push_back({
                            static_cast<int8_t>(row), static_cast<int8_t>(col)
                        });
                    }
                    break;
            }
        }
    }

    return offsets;
}

std::vector<std::vector<std::vector<int8_t>>>
generate_all_neighbourhood_offsets(
    NeighbourhoodType neighbourhood_type
) {
    // initalize offsets to {} because there is no valid offsets[0]
    std::vector<std::vector<std::vector<int8_t>>> offsets {{}};

    for (int range = 1; range < MAX_NEIGHBOURHOOD_RANGE; range++) {
        offsets.push_back(generate_neighbourhood_offsets(
            neighbourhood_type, range
        ));
    }

    return offsets;
}

std::vector<std::vector<int8_t>> get_neighbourhood_offsets(
    NeighbourhoodType neighbourhood_type, int range
) {
    switch (neighbourhood_type) {
        case NeighbourhoodType::Moore:
            return moore_offsets[range];
        case NeighbourhoodType::VonNeumann:
            return von_neumann_offsets[range];
    }

    return {};
}

int _get_neighbour_count(
    const Board& board, std::vector<std::vector<int8_t>> offsets,
    int row, int col, std::vector<uint8_t> firing_states, int bitmask = -1
) {
    int neighbour_count = 0;
    for (auto& offset : offsets) {
        auto row_offset = offset[0];
        auto col_offset = offset[1];

        int neighbour_row = modulo(row + row_offset, BOARD_ROWS);
        int neighbour_col = modulo(col + col_offset, BOARD_COLS);

        if ((bitmask != -1 && (bitmask & board[neighbour_row][neighbour_col]) == 1)
            || contains(firing_states, board[neighbour_row][neighbour_col])
        ) {
            neighbour_count++;
        }
    }

    return neighbour_count;
}

int get_weighted_neighbour_count(
    const Board& board, int row, int col,
    std::map<std::string, int> weights
) {
    int neighbour_count = 0;
    for (auto& pair : weights) {
        auto direction = pair.first;

        auto offsets = direction_to_offset[direction];
        int row_offset = offsets[0];
        int col_offset = offsets[1];
        int neighbour_row = modulo(row + row_offset, BOARD_ROWS);
        int neighbour_col = modulo(col + col_offset, BOARD_COLS);
        if (board[neighbour_row][neighbour_col] == 1) {
            neighbour_count += weights[direction];
        }
    }

    return neighbour_count;
}

int get_neighbour_count(
    const Board& board, NeighbourhoodType neighbourhood_type,
    int row, int col, int bitmask
) {
    auto offsets = get_neighbourhood_offsets(neighbourhood_type, 1);
    return _get_neighbour_count(board, offsets, row, col, {1}, bitmask);
}

int get_extended_neighbour_count(
    const Board& board, NeighbourhoodType neighbourhood_type,
    int row, int col, std::vector<uint8_t> firing_states, int range
) {
    auto offsets = get_neighbourhood_offsets(neighbourhood_type, range);
    return _get_neighbour_count(board, offsets, row, col, firing_states);
}

// gets the values of all neighbours in VonNeumann neighbourhood with range 1
// in the order [SELF, NORTH, EAST, SOUTH, WEST]
std::vector<uint8_t> get_neighbour_configuration(
    const Board& board, int row, int col
) {
    // relative coordinates of ME, N, E, S, W respectively
    // we don't use getNeighbourOffsets here because we need a specific order
    std::vector<std::vector<int8_t>> offsets {
        {{0, 0}, {-1, 0}, {0, -1}, {1, 0}, {0, 1}}
    };

    std::vector<uint8_t> neighbour_values;

    for (auto& offset : offsets) {
        int row_offset = offset[0];
        int col_offset = offset[1];
        int neighbour_row = modulo(row + row_offset, BOARD_ROWS);
        int neighbour_col = modulo(col + col_offset, BOARD_COLS);
        neighbour_values.push_back(board[neighbour_row][neighbour_col]);
    }

    return neighbour_values;
}
