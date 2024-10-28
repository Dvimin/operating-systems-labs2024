#include "daemon.h"

#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <fstream>
#include <signal.h>
#include <sstream>
#include <sys/syslog.h>

bool got_sighup = false;
bool got_sigterm = false;

void Daemon::signal_handler(int sig) {
    switch (sig) {
        case SIGHUP:
            got_sighup = true;
            syslog(LOG_INFO, "Получен сигнал SIGHUP для повторного чтения конфигурации");
            break;
        case SIGTERM:
            got_sigterm = true;
            syslog(LOG_INFO, "Получен сигнал SIGTERM, завершение работы");
            break;
        default:
            syslog(LOG_WARNING, "Получен неизвестный сигнал: %d", sig);
            break;
    }
}

void Daemon::start(const std::string &config_file) {
    syslog(LOG_INFO, "Запуск демона с конфигурационным файлом: %s", config_file.c_str());
    current_path = std::filesystem::absolute(config_file).parent_path().string();
    daemonize();
    open_config_file(config_file);
    run(config_file);
}

void Daemon::create_pid_file() {
    std::string pid_file = "/var/run/daemon_example.pid";
    syslog(LOG_INFO, "Создание PID файла: %s", pid_file.c_str());
    int pid_file_handle = open(pid_file.c_str(), O_RDWR | O_CREAT, 0600);

    if (pid_file_handle == -1) {
        syslog(LOG_ERR, "Не удалось открыть PID файл %s", pid_file.c_str());
        exit(EXIT_FAILURE);
    }

    if (lockf(pid_file_handle, F_TLOCK, 0) == -1) {
        syslog(LOG_ERR, "Демон уже запущен (PID файл заблокирован)");
        close(pid_file_handle);
        exit(EXIT_FAILURE);
    }

    char old_pid_str[10];
    if (read(pid_file_handle, old_pid_str, sizeof(old_pid_str) - 1) > 0) {
        int old_pid = atoi(old_pid_str);
        if (old_pid > 0 && kill(old_pid, 0) == 0) {
            syslog(LOG_INFO, "Процесс с PID %d уже запущен, отправка SIGTERM", old_pid);
            kill(old_pid, SIGTERM);
            sleep(1);
        } else {
            syslog(LOG_INFO, "Не найден процесс с PID %d, продолжаем...", old_pid);
        }
    }

    ftruncate(pid_file_handle, 0);
    lseek(pid_file_handle, 0, SEEK_SET);
    snprintf(old_pid_str, sizeof(old_pid_str), "%d\n", getpid());
    write(pid_file_handle, old_pid_str, strlen(old_pid_str));
    syslog(LOG_INFO, "PID файл %s успешно создан с PID %d", pid_file.c_str(), getpid());
    close(pid_file_handle);
}

void Daemon::daemonize() {
    syslog(LOG_INFO, "Демонизация процесса");
    if (fork() > 0) exit(EXIT_SUCCESS);

    umask(0);
    setsid();
    chdir("/");
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    create_pid_file();
}

void Daemon::open_config_file(const std::string &config_file) {
    syslog(LOG_INFO, "Открытие конфигурационного файла: %s", config_file.c_str());
    std::ifstream infile(config_file);
    if (!infile.is_open()) {
        syslog(LOG_ERR, "Не удалось открыть конфигурационный файл: %s", config_file.c_str());
        return;
    }

    std::filesystem::path config_dir = std::filesystem::absolute(config_file).parent_path();
    table.clear();
    time_points.clear();

    std::string line;
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        Data data;
        if (iss >> data.folder1 >> data.folder2 >> data.ext) {
            if (std::filesystem::path(data.folder1).is_relative()) {
                data.folder1 = (config_dir / data.folder1).string();
            }
            if (std::filesystem::path(data.folder2).is_relative()) {
                data.folder2 = (config_dir / data.folder2).string();
            }
            data.interval = 20;
            table.push_back(data);
            time_points.emplace_back(std::chrono::steady_clock::now());
            syslog(LOG_INFO, "Добавлена задача: folder1=%s, folder2=%s, ext=%s",
                   data.folder1.c_str(), data.folder2.c_str(), data.ext.c_str());
        } else {
            syslog(LOG_WARNING, "Ошибка при чтении строки конфигурации: %s", line.c_str());
        }
    }
}

void Daemon::replace_folder(const Data &data) {
    syslog(LOG_INFO, "Начало замены файлов из %s в %s", data.folder1.c_str(), data.folder2.c_str());

    auto cur_path1 = std::filesystem::path(data.folder1);
    auto cur_path2 = std::filesystem::path(data.folder2);
    std::filesystem::path current_path_fs(current_path);

    if (cur_path1.is_relative()) cur_path1 = current_path_fs / data.folder1;
    if (cur_path2.is_relative()) cur_path2 = current_path_fs / data.folder2;

    if (!std::filesystem::exists(cur_path1)) {
        syslog(LOG_ERR, "Исходная директория не найдена: %s", cur_path1.c_str());
        return;
    }

    if (!std::filesystem::exists(cur_path2)) {
        std::filesystem::create_directories(cur_path2);
        syslog(LOG_INFO, "Создана директория: %s", cur_path2.c_str());
    }

    for (const auto &entry: std::filesystem::directory_iterator(cur_path1)) {
        if (entry.is_regular_file() && (data.ext.empty() || entry.path().extension() != "." + data.ext)) {
            try {
                syslog(LOG_INFO, "Перемещение файла: %s в %s", entry.path().c_str(), cur_path2.c_str());
                std::filesystem::rename(entry.path(), cur_path2 / entry.path().filename());
                syslog(LOG_INFO, "Перемещен файл: %s в %s", entry.path().c_str(), cur_path2.c_str());
            } catch (const std::filesystem::filesystem_error &e) {
                syslog(LOG_ERR, "Ошибка перемещения файла %s: %s", entry.path().c_str(), e.what());
            }
        } else {
            syslog(LOG_INFO, "Пропущен файл: %s (не является обычным файлом или имеет запрещенное расширение)",
                   entry.path().c_str());
        }
    }

    syslog(LOG_INFO, "Завершено перемещение файлов из %s в %s", cur_path1.c_str(), cur_path2.c_str());
}

void Daemon::run(const std::string &config_file) {
    syslog(LOG_INFO, "Запуск основного цикла демона");

    while (true) {
        if (got_sighup) {
            got_sighup = false;
            syslog(LOG_INFO, "Получен SIGHUP, открытие конфигурационного файла: %s", config_file.c_str());
            open_config_file(config_file);
        }

        if (got_sigterm) {
            syslog(LOG_INFO, "Получен SIGTERM, завершение работы демона.");
            closelog();
            exit(EXIT_SUCCESS);
        }

        for (size_t i = 0; i < table.size(); ++i) {
            auto now_time = std::chrono::steady_clock::now();
            int diff = std::chrono::duration_cast<std::chrono::seconds>(now_time - time_points[i]).count();

            syslog(LOG_INFO, "Проверка задачи %zu: интервал = %d, прошедшее время = %d секунд", i, table[i].interval,
                   diff);

            if (diff >= table[i].interval) {
                syslog(LOG_INFO, "Запуск замены файлов для задачи %zu", i);
                replace_folder(table[i]);
                time_points[i] = now_time;
            }
        }

        sleep(1);
    }
}
