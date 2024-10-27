#include "daemon.h"
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <signal.h>
#include <string>

Daemon& Daemon::getInstance() {
    static Daemon instance;
    return instance;
}

Daemon::Daemon() {}

Daemon::~Daemon() {}

void Daemon::start(const std::string& configPath) {
    if (!checkAndHandleExistingDaemon()) {
        syslog(LOG_ERR, "Демон уже запущен. Завершение работы.");
        return;
    }

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
        syslog(LOG_ERR, "PID больше нуля, процесс уже запущен");
        exit(0);
    }

    umask(0);
    setsid();
    chdir("/");
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    std::cout << "Не упал " << std::endl;

    openlog("DaemonExample", LOG_PID | LOG_CONS, LOG_DAEMON);


    signal(SIGHUP, [](int signal) { Daemon::getInstance().reloadConfig(); });
    signal(SIGTERM, [](int signal) { Daemon::getInstance().stop(); });
    savePidToFile();

    run();
}

bool Daemon::checkAndHandleExistingDaemon() {
    std::ifstream pidFile("daemon.pid");
    std::string line;
    if (std::getline(pidFile, line)) {
        int existingPid = std::stoi(line);
        if (kill(existingPid, 0) == 0) {
            return false;
        }
    }
    return true;
}

void Daemon::savePidToFile() {
    std::ofstream pidFile("daemon.pid");
    pidFile << getpid();
}

void Daemon::run() {
    while (true) {
        moveFiles();
        usleep(config.interval * 1000000);
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

    if (!config.loadFromFile("/home/adminmisha/Documents/Polytech/Operating_systems/lab1/config.txt")) {
        syslog(LOG_ERR, "Ошибка загрузки конфигурации при перезагрузке");
    } else {
        syslog(LOG_INFO, "Конфигурация успешно перезагружена");
    }
}
