#include "daemon.h"
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <filesystem>

Daemon& Daemon::getInstance() {
    static Daemon instance;
    return instance;
}

Daemon::Daemon() {
}

Daemon::~Daemon() {
}

void Daemon::start(const std::string& configPath) {
    if (!config.loadFromFile(configPath)) {
        syslog(LOG_ERR, "Ошибка загрузки конфигурации");
        return;
    }

    pid = fork();
    if (pid < 0) {
        syslog(LOG_ERR, "Ошибка создания процесса");
        return;
    }
    if (pid > 0) {
        exit(0);
    }


    umask(0);
    setsid();
    chdir("/");

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    openlog("DaemonExample", LOG_PID | LOG_CONS, LOG_DAEMON);

    signal(SIGHUP, [](int signal) { Daemon::getInstance().reloadConfig(); });
    signal(SIGTERM, [](int signal) { Daemon::getInstance().stop(); });

    run();
}

void Daemon::run() {
    while (true) {
        moveFiles();
        usleep(config.interval * 1000);
    }
}

void Daemon::moveFiles() {
    for (size_t i = 0; i < config.sourceDirs.size(); ++i) {
        const auto& sourceDir = config.sourceDirs[i];
        const auto& destDir = config.destDirs[i];
        const auto& ext = config.extensions[i];

        for (const auto& entry : std::filesystem::directory_iterator(sourceDir)) {
            if (entry.is_regular_file()) {
                auto filename = entry.path().filename().string();
                if (filename.substr(filename.find_last_of('.') + 1) != ext) {
                    std::filesystem::rename(entry.path(), destDir + "/" + filename);
                    syslog(LOG_INFO, "Перемещён файл: %s -> %s", entry.path().c_str(), destDir.c_str());
                }
            }
        }
    }
}

void Daemon::stop() {
    syslog(LOG_INFO, "Остановка демона");
    closelog();
    exit(0);
}

void Daemon::reloadConfig() {
    syslog(LOG_INFO, "Перезагрузка конфигурации");
}
