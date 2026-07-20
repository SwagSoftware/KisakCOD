#pragma once

#include <string>

struct KisakDataProbe {
    bool rootCreated = false;
    bool rootExists = false;
    bool mainExists = false;
    bool zoneExists = false;
    int iwdCount = 0;
    int localizedIwdCount = 0;
    int fastFileCount = 0;
    std::string rootPath;
    std::string message;
};

KisakDataProbe ProbeCod4DataDirectory(const std::string& rootPath);
