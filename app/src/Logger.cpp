#include "Logger.h"

Logger *Logger::instance = NULL;

Logger *Logger::getInstance() {
    if (instance == NULL)
        instance = new Logger;
    return instance;
}

void Logger::log(Type type, std::string msg) {
    switch (type) {
        case ERROR:   log("ERROR",   RED,     msg); break;
        case INFO:    log("INFO",    GREEN,   msg); break;
        case WARNING: log("WARNING", YELLOW,  msg); break;
    }
}

void Logger::log(std::string type, std::string color, std::string msg) {
    std::stringstream ss;
    // if the user has a unix system
    // use the color
#ifdef __unix__
    ss << color;
#endif
    ss << "[" << type << "] " << msg;
#ifdef __unix__
    ss << RESET;
#endif
    std::cout << ss.str() << std::endl;
}

void Logger::testAllTypes() {
    LOG_ERR("This is an error message");
    LOG_INFO("This is an info message");
    LOG_WARNING("This is a warning message");
}