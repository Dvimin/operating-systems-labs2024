#include "config.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <syslog.h>

bool Config::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        syslog(LOG_ERR, "Ошибка открытия файла конфигурации: %s", filename.c_str());
        return false;
    }

    sourceDirs.clear();
    destDirs.clear();
    extensions.clear();

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string folder1, folder2, ext;
        if (!(iss >> folder1 >> folder2 >> ext)) {
            syslog(LOG_ERR, "Ошибка разбора строки: %s", line.c_str());
            continue;
        }
        sourceDirs.push_back(folder1);
        destDirs.push_back(folder2);
        extensions.push_back(ext);
    }

    return true;
}
