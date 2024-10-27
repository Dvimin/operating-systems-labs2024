#ifndef DAEMON_H
#define DAEMON_H

#include <string>
#include <signal.h>
#include "config.h"

class Daemon {
public:
    static Daemon& getInstance();
    void run(const std::string& configPath);

private:
    Daemon();
    void forkDaemon();
    void readConfig(const std::string& configPath);
    void moveFiles();
    void signalHandler(int signal);
    void log(const std::string& message);

    Config config;
    pid_t pid;
    int interval;
};

#endif // DAEMON_H
