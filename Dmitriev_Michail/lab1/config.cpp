#include "config.h"
#include <fstream>
#include <sstream>
#include <iostream>

bool Config::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Ошибка открытия файла конфигурации: " << filename << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string folder1, folder2, ext;
        if (!(iss >> folder1 >> folder2 >> ext)) {
            std::cerr << "Ошибка разбора строки: " << line << std::endl;
            continue;
        }
        sourceDirs.push_back(folder1);
        destDirs.push_back(folder2);
        extensions.push_back(ext);
    }

    return true;
}
