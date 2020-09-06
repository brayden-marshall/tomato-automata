#include <stdexcept>

#include "./automata.h"
#include "../common.h"

WeightedLife::WeightedLife(std::string name, std::string rules)
    : CellularAutomata { name, rules }
{
    auto rules_arr = split(rules, ',');
    for (auto& rule : rules_arr) {
        std::string prefix = rule.substr(0, 2);
        int value = std::stoi(rule.substr(2));

        if (contains({"NW", "NN", "NE", "WW", "EE", "SW", "SS", "SE", "NE"},
                    prefix)
        ) {
            if (abs(value) <= 256) {
                neighbour_weights[prefix] = value;
            } else {
                throw new std::runtime_error(
                    "Neighbour weights must be in range [-256..256]"
                );
            }
        } else if (prefix == "HI") {
            if (value > 0) {
                num_states = value;
            } else {
                // if no history is specified, there are 2 states
                num_states = 2;
            }
        } else if (prefix == "RB") {
            birth_numbers.push_back(value);
        } else if (prefix == "RS") {
            survive_numbers.push_back(value);
        }
    }
}

std::pair<Board, bool> WeightedLife::rewrite(const Board& board) {
    bool change_made = false;
    Board board_copy = board;

    for (size_t row = 0; row < BOARD_ROWS; row++) {
        for (size_t col = 0; col < BOARD_COLS; col++) {
            int neighbour_count = get_weighted_neighbour_count(
                board, row, col, neighbour_weights
            );

            if (board[row][col] == 0) {
                if (contains(birth_numbers,
                            static_cast<uint8_t>(neighbour_count))
                ) {
                    board_copy[row][col] = 1;
                    change_made = true;
                }
            } else if (board[row][col] != 1 ||
                       !contains(survive_numbers,
                                 static_cast<uint8_t>(neighbour_count))
            ) {
                board_copy[row][col] = (board[row][col] + 1) % num_states;
                change_made = true;
            }
        }
    }

    return { board_copy, change_made };
}
