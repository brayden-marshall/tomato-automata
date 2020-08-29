#ifndef AUTOMATA_LIFE_H
#define AUTOMATA_LIFE_H

#include "../common.h"
#include <string>
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

#endif
