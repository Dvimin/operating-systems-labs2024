#include "config.h"
#include <fstream>
#include <sstream>

bool Config::load(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    rules.clear();
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        Rule rule;
        if (iss >> rule.folder1 >> rule.folder2 >> rule.ext) {
            rules.push_back(rule);
        }
    }
    return true;
}

const std::vector<Config::Rule> &Config::getRules() const {
    return rules;
}
