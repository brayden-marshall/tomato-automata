#include <stdexcept>
#include <iostream>
using std::cout;
using std::endl;

#include "automata.h"
#include "../common.h"

Cyclic::Cyclic(std::string name, std::string rules)
    : CellularAutomata { name, rules }
{
    auto rules_arr = split(rules, '/');

    if (rules_arr.size() != 4 && rules_arr.size() != 5) {
        throw new std::runtime_error("Cyclic rule notation is incorrect");
    }

    neighbourhood_range = std::stoi(rules_arr[0].substr(1));

    threshold = std::stoi(rules_arr[1].substr(1));

    num_states = std::stoi(rules_arr[2].substr(1));

    if (rules_arr[3] == "NM") {
        neighbourhood_type = NeighbourhoodType::Moore;
    } else if (rules_arr[3] == "NN") {
        neighbourhood_type = NeighbourhoodType::VonNeumann;
    } else {
        throw new std::runtime_error(
            "Cyclic neighbourhood field is invalid " + rules_arr[3]
        );
    }

    greenberg_hastings = rules_arr.size() == 5 && rules_arr[4] == "GH";
}

std::pair<Board, bool> Cyclic::rewrite(const Board& board) {
    bool change_made = false;
    Board board_copy = board;

    for (int row = 0; row < BOARD_ROWS; row++) {
        for (int col = 0; col < BOARD_COLS; col++) {
            int next_state = (board[row][col] + 1) % num_states;

            int neighbour_count = get_extended_neighbour_count(
                board, neighbourhood_type, row, col,
                {greenberg_hastings ?
                    static_cast<uint8_t>(1) :
                    static_cast<uint8_t>(next_state)},
                neighbourhood_range
            );

            if (neighbour_count >= threshold
                || (greenberg_hastings && board[row][col] != 0)
            ) {
                board_copy[row][col] = next_state;
                change_made = true;
            }
        }
    }

    return std::pair<Board, bool> {board_copy, change_made};
}
