#include <stdexcept>

#include "./automata.h"
#include "../common.h"

std::vector<uint8_t> parse_range(std::string range) {
    auto range_arr = split(range, '.');
    auto start = std::stoi(range_arr[0]);
    auto end = std::stoi(range_arr[range_arr.size()-1]);

    std::vector<uint8_t> nums;
    for (uint8_t i = start; i <= end; i++) {
        nums.push_back(i);
    }
    return nums;
}

LargerThanLife::LargerThanLife(std::string name, std::string rules)
    : CellularAutomata { name, rules }
{
    std::string err = "Invalid rules for LargerThanLife with name " + name;
    for (const std::string& rule : split(rules, ',')) {
        switch (rule[0]) {
            case 'R':
                range = std::stoi(rule.substr(1));
                break;
            case 'C':
                num_states = std::stoi(rule.substr(1));
                if (num_states < 2) {
                    num_states = 2;
                }
                break;
            case 'M':
                count_center_cell = std::stoi(rule.substr(1)) == 1;
                break;
            case 'S':
                survive_numbers = parse_range(rule.substr(1));
                break;
            case 'B':
                birth_numbers = parse_range(rule.substr(1));
                break;
            case 'N':
                if (rule[1] == 'M') {
                    neighbourhood_type = NeighbourhoodType::Moore;
                } else if (rule[1] == 'N') {
                    neighbourhood_type = NeighbourhoodType::VonNeumann;
                } else {
                    throw new std::runtime_error(err);
                }
                break;
            default:
                throw new std::runtime_error(err);
        }
    }
}

std::pair<Board, bool> LargerThanLife::rewrite(const Board& board) {
    bool change_made = false;
    auto board_copy = board;

    for (int row = 0; row < BOARD_ROWS; row++) {
        for (int col = 0; col < BOARD_COLS; col++) {
            uint8_t neighbour_count = get_extended_neighbour_count(
                board, neighbourhood_type, row, col, {1}, range
            );

            if (board[row][col] == 0) {
                if (contains(birth_numbers, neighbour_count)) {
                    board_copy[row][col] = 1;
                    change_made = true;
                }
            } else if (!(board[row][col] == 1 &&
                        contains(survive_numbers, neighbour_count))
            ) {
                board_copy[row][col] = (board[row][col] + 1) % num_states;
                change_made = true;
            }
        }
    }

    return {board_copy, change_made};
}
