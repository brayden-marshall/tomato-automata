#include <stdexcept>
#include <string>
#include <iostream>
#include <algorithm>

#include "./automata.h"
#include "../common.h"

Generations::Generations(std::string name, std::string rules)
    : CellularAutomata {name, rules}
{
    std::vector<std::string> rules_arr = split(rules, '/');

    if (rules_arr.size() != 3) {
        throw new std::runtime_error(
            "Generations notation must contain 3 fields"
        );
    }

    // parse survive numbers
    for (int i = 0; i < rules_arr[0].size(); i++) {
        int n = rules_arr[0][i] - '0';
        if (n < 0 || n > 9) {
            throw new std::runtime_error(
                "Generations first field is incorrect."
            );
        }

        survive_numbers.push_back(n);
    }

    // parse birth numbers
    for (int i = 0; i < rules_arr[1].size(); i++) {
        int n = rules_arr[1][i] - '0';
        if (n < 0 || n > 9) {
            throw new std::runtime_error(
                "Generations second field is incorrect."
            );
        }

        birth_numbers.push_back(n);
    }

    // parse num states
    {
        int n = rules_arr[2][0] - '0';
        if (n < 0 || n > 9) {
            throw new std::runtime_error(
                "Generations third field is incorrect."
            );
        }
        num_states = n;
    }
}

std::pair<Board, bool> Generations::rewrite(const Board& board) {
    bool change_made = false;
    Board board_copy = board;

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

                    if (board[neighbour_row][neighbour_col] == 1) {
                        neighbour_count++;
                    }
                }
            }

            if (board[row][col] == 0) {
                if (contains(birth_numbers, neighbour_count)) {
                    board_copy[row][col] = 1;
                    change_made = true;
                }
            } else if (board[row][col] != 1 ||
                       !contains(survive_numbers, neighbour_count)
            ) {
                board_copy[row][col] = (board[row][col] + 1) % num_states;
                change_made = true;
            }
        }
    }

    return std::pair<Board, bool> { board_copy, change_made };
}
