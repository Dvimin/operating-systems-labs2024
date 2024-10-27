#include <iostream>
#include "daemon.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Использование: " << argv[0] << " <путь к конфигурационному файлу>" << std::endl;
        return 1;
    }
    std::cout << "Запуск демона с конфигурационным файлом: " << argv[1] << std::endl;
    std::string configPath = argv[1];
    Daemon::getInstance().start(configPath);
    std::cout << "Демон успешно запущен." << std::endl;

    return 0;
}
