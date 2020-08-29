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

// forward declarations
class CellularAutomata;

// typedefs
typedef std::array<std::array<uint8_t, BOARD_COLS>, BOARD_ROWS> Board;
typedef std::array<uint8_t, 3> Color;
typedef std::map<std::string, std::vector<CellularAutomata*>>
        CellularAutomataMap;
typedef std::vector<std::array<uint8_t, 3>> ColorPalette;

// functions
CellularAutomataMap load_cellular_automata();
std::vector<std::string> split(const std::string& s, char delimiter);
template<typename T>
bool contains(const std::vector<T>& vec, T val) {
    return std::find(vec.cbegin(), vec.cend(), val) != vec.cend();
}

inline int modulo(int a, int b) {
    return ((a % b) + b) % b;
}

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
