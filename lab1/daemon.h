#ifndef DAEMON_H
#define DAEMON_H

#include <string>
#include <signal.h>
#include <syslog.h>
#include "config.h"

class Daemon {
public:
    static Daemon& getInstance();

    void start(const std::string& configPath);

    bool checkAndHandleExistingDaemon();

    void savePidToFile();

    void stop();
    void reloadConfig();

private:
    Daemon();
    ~Daemon();

    void run();
    void signalHandler(int signal);
    void moveFiles();

    Config config;
    pid_t pid;
};

#endif
