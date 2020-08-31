#include <stdexcept>
#include <cmath>

#include "./automata.h"
#include "../common.h"

/* Interpret 'neighbourConfiguration' as digits of a number in base 'this.numStates'
 * and convert it to it's decimal representation.
 *
 * For example when numStates = 3:
 *     A neighbourConfiguration of [0, 0, 1, 2, 1] will be interpreted as 00121 base 3.
 *     00121 (base 3) is 16 (base 10), so 16 will be returned
*/
int neighbour_config_to_index(
    std::vector<uint8_t> neighbour_config, uint8_t num_states
) {
    int index = 0;

    for (int i = 0; i < neighbour_config.size(); i++) {
        // get values in reverse because 'neighbourConfiguration' is being intepreted
        // as digits of a number (base this.numStates) parsed from right to left
        auto neighbour_val = neighbour_config[(neighbour_config.size()-1)-i];
        index += neighbour_val * pow(num_states, i);
    }

    return index;
}

NeumannBinary::NeumannBinary(std::string name, std::string rules)
    : CellularAutomata { name, rules }
{
    int _num_states = rules[0] - '0';
    if (_num_states < 2 || _num_states > 4) {
        throw new std::runtime_error(
            "NeumannBinary rules field one must be either 2, 3, or 4"
        );
    }

    num_states = _num_states;

    int n = rules.size();
    for (int i = 1; i < n; i++) {
        int _state = rules[i] - '0';
        if (_state < 0 || _state > 9) {
            throw new std::runtime_error(
                "NeumannBinary rules can only contain digits"
            );
        }
        transition_table.push_back(_state);
    }
}

std::pair<Board, bool> NeumannBinary::rewrite(const Board& board) {
    bool change_made = false;
    auto board_copy = board;

    for (int row = 0; row < BOARD_ROWS; row++) {
        for (int col = 0; col < BOARD_COLS; col++) {
            // get values of all neighbours
            std::vector<uint8_t> neighbour_config =
                get_neighbour_configuration(board, row, col);

            int index = neighbour_config_to_index(neighbour_config, num_states);

            // check before changing here so we can properly set the change_made
            if (board[row][col] != transition_table[index]) {
                board_copy[row][col] = transition_table[index];
                change_made = true;
            }
        }
    }

    return { board_copy, change_made };
}
