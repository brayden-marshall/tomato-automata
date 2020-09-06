#include <stdexcept>
#include <optional>

#include "./automata.h"
#include "../common.h"

#define RULE_TABLE_ROW_LENGTH 10

RulesTable::RulesTable(std::string name, std::string rules)
    : CellularAutomata { name, rules }
{
    auto rules_arr = split(rules, ',');
    if (rules_arr.size() < 3) {
        throw new std::runtime_error("RulesTable format is incorrect.");
    }

    if (rules_arr[0] == "1") {
        neighbourhood_type = NeighbourhoodType::Moore;
    } else if (rules_arr[0] == "2") {
        neighbourhood_type = NeighbourhoodType::VonNeumann;
    } else {
        throw new std::runtime_error(
            "Rules table first field is incorrect: " + rules_arr[0]
        );
    }

    if (rules_arr[1] == "0") {
        count_center_cell = false;
    } else if (rules_arr[1] == "1") {
        count_center_cell = true;
    } else {
        throw new std::runtime_error(
            "Rules table second field is incorrect: " + rules_arr[1]
        );
    }

    if (rules_arr[2] == "0") {
        first_bitplane_is_firing = false;
    } else if (rules_arr[2] == "1") {
        first_bitplane_is_firing = true;
    } else {
        throw new std::runtime_error(
            "Rules table third field is incorrect: " + rules_arr[2]
        );
    }

    // start index of the rules table in the string being parsed
    size_t start_index = 3;

    // parse the remainder of the string into this->table
    while (start_index < rules_arr.size()) {
        size_t end_index = start_index + RULE_TABLE_ROW_LENGTH;
        if (rules_arr.size()-start_index <= RULE_TABLE_ROW_LENGTH) {
            end_index = rules_arr.size();
        }

        // convert the current row to integers
        std::vector<int> row;
        for (size_t i = start_index; i < end_index; i++) {
            int temp = rules_arr[i][0] - '0';
            if (temp < 0 || temp > 9) {
                throw new std::runtime_error(
                    std::to_string(temp) + " is not a valid integer."
                );
            }
            row.push_back(temp);
        }

        while (row.size() < RULE_TABLE_ROW_LENGTH) {
            row.push_back(0);
        }

        table.push_back(row);
        start_index = end_index;
    }

    num_states = table.size();
}

RulesTable::RulesTable(
    std::string name, std::string rules, ColorPalette color_override
): RulesTable { name, rules }
{
    this->color_override = std::optional(color_override);
}

std::pair<Board, bool> RulesTable::rewrite(const Board& board) {
    bool change_made = false;
    Board board_copy = board;

    for (size_t row = 0; row < BOARD_ROWS; row++) {
        for (size_t col = 0; col < BOARD_COLS; col++) {
            int neighbour_count = first_bitplane_is_firing ?
                get_neighbour_count(board, neighbourhood_type, row, col, 1) :
                get_neighbour_count(board, neighbourhood_type, row, col);

            // add center cell to neighbour_count if applicable
            if (count_center_cell && board[row][col] == 1) {
                neighbour_count++;
            }

            // set the current cell to its new value given by the rule table
            board_copy[row][col] = table[board[row][col]][neighbour_count];

            // check if the current cell has changed so that we can
            // identify stagnant states
            if (board_copy[row][col] != board[row][col]) {
                change_made = true;
            }
        }
    }

    return { board_copy, change_made };
}
