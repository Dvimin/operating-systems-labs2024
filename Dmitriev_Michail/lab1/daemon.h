#ifndef DAEMON_HPP
#define DAEMON_HPP

#include <vector>
#include <string>
#include <filesystem>
#include <chrono>

struct Data {
    std::string folder1;
    std::string folder2;
    std::string ext;
    int interval;
};

class Daemon {
public:
    static Daemon &getInstance() {
        static Daemon instance;
        return instance;
    }

    void start(const std::string &config_file);

private:
    Daemon() : got_sighup(false), got_sigterm(false) {
    }

    Daemon(const Daemon &) = delete;

    Daemon &operator=(const Daemon &) = delete;

    void replace_folder(const Data &data);

    void open_config_file(const std::string &config_file);

    void create_pid_file();

    void daemonize();

    void run(const std::string &config_file);

    void signal_handler(int sig);

    bool got_sighup;
    bool got_sigterm;
    std::vector<Data> table;
    std::vector<std::chrono::time_point<std::chrono::steady_clock> > time_points;
    std::string current_path;
};
#endif // DAEMON_HPP
