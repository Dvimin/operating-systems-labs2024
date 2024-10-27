#include "daemon.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <config file>\n", argv[0]);
        return EXIT_FAILURE;
    }
    std::string configPath = argv[1];
    Daemon::getInstance().run(configPath);
    return EXIT_SUCCESS;
}
