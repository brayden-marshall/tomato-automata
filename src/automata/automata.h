#ifndef AUTOMATA_LIFE_H
#define AUTOMATA_LIFE_H

#include "../common.h"
#include <string>
#include <optional>
#include <cstdint>

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

    NeumannBinary(std::string name, std::string rules);
};

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

class WeightedLife: public CellularAutomata {
};

#endif
