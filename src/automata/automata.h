#ifndef AUTOMATA_LIFE_H
#define AUTOMATA_LIFE_H

#include "../common.h"
#include <string>
#include <optional>
#include <cstdint>

// Rules are in the form S/B/C where:
// S - A list of 1-digit numbers representing the number of neighbours a cell
//     must have in order to survive (stay at state 1) in the next generation
// B - A list of 1-digit numbers representing the number of neighbours a cell
//     must have in order to give birth (go from state 0 to state 1) in the
//     next generation
// C - The number of possible states a cell can have (including 0 state).
//     Any state higher than 1 is a history state (is not considered
//     to be firing).
class Generations: public CellularAutomata {
protected:
    std::vector<uint8_t> survive_numbers;
    std::vector<uint8_t> birth_numbers;
public:
    virtual std::pair<Board, bool> rewrite(const Board& board) override;

    Generations(std::string name, std::string rules);
};

class Life: public Generations {
    public:
        Life(std::string name, std::string rules);
};

// Rules are in form Rx/Tx/Cx/N/GH where:
// R = Neighbourhood range from [1..10].
// T = Threshold (minimal count of cells having next color that are necessary
//     for the current cell to advance to the next state.
// C = The number of states in the rule [0..C-1].
// N = NeighbourhoodType (NM = extended Moore, NN = extended Von Neumann).
// GH (optional) = Whether or not the rule is a Greenberg-Hastings model.
//     In a Greenberg-Hastings model all states but 0 advance to the next state
//     automatically, except for cells with state 0 which advance only if there are
//     at least 'threshold' 1s in their neighbourhood.
//
// example:
//     R1/T3/C3/NM
class Cyclic: public CellularAutomata {
protected:
    int neighbourhood_range;
    NeighbourhoodType neighbourhood_type;
    int threshold;
    bool greenberg_hastings;
public:
    virtual std::pair<Board, bool> rewrite(const Board& board) override;

    Cyclic(std::string name, std::string rules);
};

// Rules are in the form Rx,Cx,Mx,Sx..y,Bx..y,N where:
// Rx - The neighbourhood range [1...10]
// Cx - The number of states [0...25]. A value smaller than 3 means
//      that history is not active.
// Mx - Whether the center cell should be considered when counting the number of
//     firing neighbours. 0 for false, 1 for true.
// Sx..y - The range (inclusive) of firing neighbours necessary for a cell
//         to survive.
// Bx..y - The range (inclusive) of firing neighbours necessary for a cell
//         to be born.
// N - The neighbourhood type. NM for extended Moore, NN for extended
//     Von Neumann.
//
// example:
//     N5,C0,M1,S3..20,B1..4,NM
class LargerThanLife: public CellularAutomata {
protected:
    int range;
    bool count_center_cell;
    std::vector<uint8_t> survive_numbers;
    std::vector<uint8_t> birth_numbers;
    NeighbourhoodType neighbourhood_type;

public:
    virtual std::pair<Board, bool> rewrite(const Board& board) override;

    LargerThanLife(std::string name, std::string rules);
};

class NeumannBinary: public CellularAutomata {
protected:
    /* An array of integers representing what state a cell should receive in the
    * next generation, depending on it's current neighbour configuration.
    *
    * ## Index
    * Every index represents a neighbour configuration in the following form:
    *
    *    ME N E S W
    *
    * Where ME represents the current cell that is being checked; N, E, S and W
    * represent the neighbour cells using cardinal directions; and each element
    * represents one digit of a base 'numStates' number. This indexing method is
    * used to enumerate every possible neighbour configuration.
    *
    * For example with numStates = 2 and the configuration:
    *
    *     ME=0, N=1, E=1, S=0, W=1
    *
    * The index would be 13 (01101 in binary is 13 in decimal). So the current
    * cell would receive the value of transitionTable[13].
    *
    * ## Element
    * Each element of the array represents the state that should be given to a cell
    * with a neighbour configuration that matches that index.
    *
    * For example if numStates = 2 and transitionTable = [0, 1, 0, 1, 1, 0 ... ],
    * if the configuration is (ME=0, N=0, E=0, S=0, W=1) then we use 1 as the index.
    * transitionTable[1] == 1 so we set the current cell equal to 1.
    */
    std::vector<uint8_t> transition_table;

public:
    virtual std::pair<Board, bool> rewrite(const Board& board) override;

    // Rules taken as a string of 1-digit integers where the first digit
    // represents the number of states (2, 3 or 4), and the following digits
    // represent the transition table (see above description).
    NeumannBinary(std::string name, std::string rules);
};

// parses a string representing the rules and sets member variables accordingly
// the rule format is as follows:
//     (N, C, B), TABLE
//     where:
//         N: The neighbourhood type. "1" for Moore, "2" for VonNeumann
//         C: Whether the center cell is considered when counting neighbours
//            "0" for false, "1" for true
//         B: If "1", any cell state with a 1 bit in position 0 is considered
//            to be firing, else if "0" only the state with value 1 is
//            considered to be firing
//         TABLE: A comma separated list of integers representing the rule table
//                Every 10 digits in the list represents a row in the table,
//                and the last row can omit trailing zeroes. Every row in the
//                table represents a cell state, and every column represents
//                a count of firing neighbours. On every rewrite, a cell's
//                state can be determined by looking up it's new state in the
//                table with the two aforementioned parameters.
class RulesTable: public CellularAutomata {
protected:
    NeighbourhoodType neighbourhood_type;
    bool count_center_cell;
    bool first_bitplane_is_firing;
    // accessed using table[<cell state>][<number of neighbours firing>]
    std::vector<std::vector<int>> table;
public:
    virtual std::pair<Board, bool> rewrite(const Board& board) override;

    RulesTable(std::string name, std::string rules);
    RulesTable(
        std::string name, std::string rules, ColorPalette color_override
    );
};

// Rule notation consists of the following keywords specified in any order and
// followed by an integer value.
//
//     NW NN NE WW EE SW SS SE ME HI RS RB
// 
// The first 8 define weights of the 8 neighbours. ME specifies the weight of
// the center cell. HI (history) if non-zero, defines how many states cells
// can have (including 0 state). RS and RB keywords specify rules for survival
// and birth respectively. They can appear any number of times in the string.
//
// example:
//     NW3,NN2,NE3,WW2,ME0,EE2,SW3,SS2,SE3,HI0,RS3,RS5,RS8,RB4,RB6,RB8
class WeightedLife: public CellularAutomata {
protected:
    std::map<std::string, int> neighbour_weights;
    std::vector<uint8_t> birth_numbers;
    std::vector<uint8_t> survive_numbers;

public:
    virtual std::pair<Board, bool> rewrite(const Board& board) override;

    WeightedLife(std::string name, std::string rules);
};

class LangtonsAnt: public CellularAutomata {
protected:
    std::pair<int, int> ant_pos { BOARD_ROWS / 2, BOARD_COLS / 2 };
    Direction ant_direction { Direction::Up };
    // track the state of the square the ant is on here, since we need to
    // set it to a different state on the board in order to display the ant
    uint8_t ant_square_state = 0;

public:
    virtual std::pair<Board, bool> rewrite(const Board& board) override;
    virtual void handle_mouse_click(
        Board& board, int selected_state, int row, int col, bool is_right_click
    ) override;

    LangtonsAnt();
};

#endif
