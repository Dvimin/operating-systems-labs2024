#include <iostream>
#include "daemon.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Использование: " << argv[0] << " <путь к конфигурационному файлу>" << std::endl;
        return 1;
    }

    Daemon::getInstance().start(argv[1]);

    return 0;
}
