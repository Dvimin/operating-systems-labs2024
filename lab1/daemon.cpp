#include "daemon.h"
#include <unistd.h>
#include <syslog.h>
#include <sys/stat.h>
#include <dirent.h>
#include <cstring>
#include <cstdlib>

Daemon::Daemon() : interval(20) {
    openlog("Daemon", LOG_PID | LOG_CONS, LOG_USER);
}

Daemon& Daemon::getInstance() {
    static Daemon instance;
    return instance;
}

void Daemon::forkDaemon() {
    pid = fork();
    if (pid < 0) {
        log("Fork failed");
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }
    if (setsid() < 0) {
        log("Failed to create new session");
        exit(EXIT_FAILURE);
    }
    signal(SIGHUP, [](int signum) { getInstance().signalHandler(signum); });
    signal(SIGTERM, [](int signum) { getInstance().signalHandler(signum); });
}

void Daemon::run(const std::string& configPath) {
    forkDaemon();
    while (true) {
        readConfig(configPath);
        moveFiles();
        sleep(interval);
    }
}

void Daemon::readConfig(const std::string& configPath) {
    if (!config.load(configPath)) {
        log("Failed to load configuration");
    }
}

void Daemon::moveFiles() {
    for (const auto& rule : config.getRules()) {
        DIR* dir = opendir(rule.folder1.c_str());
        if (dir == nullptr) {
            log("Failed to open directory: " + rule.folder1);
            continue;
        }

        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string filename = entry->d_name;
            if (filename == "." || filename == "..") continue;

            if (filename.substr(filename.find_last_of(".") + 1) != rule.ext) {
                std::string srcPath = rule.folder1 + "/" + filename;
                std::string dstPath = rule.folder2 + "/" + filename;
                rename(srcPath.c_str(), dstPath.c_str());
                log("Moved: " + srcPath + " -> " + dstPath);
            }
        }
        closedir(dir);
    }
}

void Daemon::signalHandler(int signal) {
    if (signal == SIGHUP) {
        log("Received SIGHUP, reloading config");
    } else if (signal == SIGTERM) {
        log("Received SIGTERM, exiting");
        closelog();
        exit(EXIT_SUCCESS);
    }
}

void Daemon::log(const std::string& message) {
    syslog(LOG_NOTICE, "%s", message.c_str());
}
