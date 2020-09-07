#include "../common.h"
#include "automata.h"

#define OFF_STATE 0
#define ON_STATE 1
#define ANT_STATE 2

LangtonsAnt::LangtonsAnt()
    : CellularAutomata { "Langton's Ant", "" }
{
      num_states = 3;
      color_override = {{255, 255, 255}, {0, 0, 0}, {255, 0, 0} };
}


std::pair<Board, bool> LangtonsAnt::rewrite(const Board& board) {
    Board board_copy = board;

    // if there is no ant, place one and exit
    if (board[ant_pos.first][ant_pos.second] != ANT_STATE) {
        ant_square_state = board[ant_pos.first][ant_pos.second];
        board_copy[ant_pos.first][ant_pos.second] = ANT_STATE;
        return { board_copy, true };
    }

    // flip the state of the square the ant is leaving
    board_copy[ant_pos.first][ant_pos.second] =
        ant_square_state == ON_STATE ? OFF_STATE : ON_STATE;

    // change direction based on state of current square
    if (ant_square_state == ON_STATE) {
        ant_direction = static_cast<Direction>(
            modulo(static_cast<int>(ant_direction)-1, DIRECTIONS_MAX)
        );
    } else {
        ant_direction = static_cast<Direction>(
            (static_cast<int>(ant_direction)+1) % DIRECTIONS_MAX
        );
    }

    // move forward one square
    std::pair<int, int> move_offset;
    switch (ant_direction) {
        case Direction::Up:
            move_offset = {-1, 0};
            break;
        case Direction::Right:
            move_offset = {0, 1};
            break;
        case Direction::Down:
            move_offset = {1, 0};
            break;
        case Direction::Left:
            move_offset = {0, -1};
            break;
    }

    // update the ant position
    ant_pos.first = modulo(ant_pos.first + move_offset.first, BOARD_ROWS);
    ant_pos.second = modulo(ant_pos.second + move_offset.second, BOARD_COLS);

    // update ant_square_state
    ant_square_state = board[ant_pos.first][ant_pos.second];

    // move the ant
    board_copy[ant_pos.first][ant_pos.second] = ANT_STATE;

    return { board_copy, true };
}


void LangtonsAnt::handle_mouse_click(
    Board& board, int selected_state, int row, int col, bool is_right_click
) {
}
