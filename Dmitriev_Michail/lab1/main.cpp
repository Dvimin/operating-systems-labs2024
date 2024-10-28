#include <sys/syslog.h>

#include "daemon.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        syslog(LOG_ERR, "Usage: %s <config_file>", argv[0]);
        return EXIT_FAILURE;
    }

    Daemon::getInstance().start(argv[1]);
    return EXIT_SUCCESS;
}
