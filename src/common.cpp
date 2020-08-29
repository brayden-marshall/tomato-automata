#include <string>
#include <sstream>

#include "./common.h"
#include "./automata/automata.h"

std::vector<std::string> split(const std::string& s, char delimiter)
{
   std::vector<std::string> tokens;
   std::string token;
   std::istringstream token_stream(s);
   while (std::getline(token_stream, token, delimiter))
   {
      tokens.push_back(token);
   }
   return tokens;
}

CellularAutomata::CellularAutomata() {
}

CellularAutomata::CellularAutomata(std::string name, std::string rules)
    : name(name), rules(rules) {
}

