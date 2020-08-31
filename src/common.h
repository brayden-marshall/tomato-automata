#ifndef CELLULAR_AUTOMATA_H
#define CELLULAR_AUTOMATA_H

#include <string>
#include <vector>
#include <array>
#include <map>
#include <algorithm>
#include <cstdint>

#define BOARD_ROWS 100
#define BOARD_COLS 100

//
// forward declarations
//
class CellularAutomata;
enum class NeighbourhoodType;

//
// variables
//

// stored neighbourhood offsets
extern std::vector<std::vector<std::vector<int8_t>>> moore_offsets;
extern std::vector<std::vector<std::vector<int8_t>>> von_neumann_offsets;
extern std::map<std::string, std::vector<int>> direction_to_offset;

//
// typedefs
//
typedef std::array<std::array<uint8_t, BOARD_COLS>, BOARD_ROWS> Board;
typedef std::array<uint8_t, 3> Color;
typedef std::map<std::string, std::vector<CellularAutomata*>>
        CellularAutomataMap;
typedef std::vector<std::array<uint8_t, 3>> ColorPalette;

//
// functions
//
CellularAutomataMap load_cellular_automata();
std::vector<std::string> split(const std::string& s, char delimiter);
template<typename T>
bool contains(const std::vector<T>& vec, T val) {
    return std::find(vec.cbegin(), vec.cend(), val) != vec.cend();
}

// neighbourhood count functions
void init_neighbourhood_offsets();
std::vector<std::vector<int8_t>> generate_neighbourhood_offsets(
    NeighbourhoodType neighbourhood_type, int range
);
std::vector<std::vector<std::vector<int8_t>>> generate_all_neighbourhood_offsets(
    NeighbourhoodType neighbourhood_type
);
std::vector<std::vector<int8_t>> get_neighbourhood_offsets(
        NeighbourhoodType neighbourhood_type, int range
);
int get_weighted_neighbour_count(
    const Board& board, int row, int col,
    std::map<std::string, std::vector<int>> weights
);
int get_neighbour_count(
    const Board& board, NeighbourhoodType neighbourhood_type,
    int row, int col, int bitmask = 0
);
int get_extended_neighbour_count(
    const Board& board, NeighbourhoodType neighbourhood_type,
    int row, int col, std::vector<uint8_t> firing_states, int range
);
std::vector<uint8_t> get_neighbour_configuration(
    const Board& board, int row, int col
);

// inline functions
inline int in_bounds(int row, int col) {
    return row >= 9 && row < BOARD_ROWS && col >= 0 && col < BOARD_COLS;
}

inline int modulo(int a, int b) {
    return ((a % b) + b) % b;
}

// enums
enum class NeighbourhoodType {
    VonNeumann,
    Moore,
};

// classes
class CellularAutomata {
    protected:
        CellularAutomata();
        CellularAutomata(std::string name, std::string rules);
    public:
        // members
        std::string name;
        uint8_t num_states;
        std::string rules;
        Color* color_override = nullptr;

        // methods
        virtual ~CellularAutomata() {}
        virtual std::pair<Board, bool> rewrite(const Board& board) = 0;
};

#endif
