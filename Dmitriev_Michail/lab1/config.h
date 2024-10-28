#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>

class Config {
public:
    struct Rule {
        std::string folder1;
        std::string folder2;
        std::string ext;
    };

    bool load(const std::string &filename);

    const std::vector<Rule> &getRules() const;

private:
    std::vector<Rule> rules;
};

#endif // CONFIG_H
