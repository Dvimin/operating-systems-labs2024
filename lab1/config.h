#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>

struct Config {
    std::vector<std::string> sourceDirs;
    std::vector<std::string> destDirs;
    std::vector<std::string> extensions;
    int interval;

    bool loadFromFile(const std::string& filename);
};

#endif
