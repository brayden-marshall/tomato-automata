#include "./automata.h"

Life::Life(std::string name, std::string rules)
    : Generations { name, rules + "/2" } 
{}
