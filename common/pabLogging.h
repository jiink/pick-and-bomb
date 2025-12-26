#pragma once

#include <iostream>
#include <string>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <format>

enum class LogLevel {
    INFO,
    WARNING,
    ERROR
};

class Logger {
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    void log(LogLevel level, const std::string& message) {
        std::lock_guard<std::mutex> lock(logMutex);
        std::cout << "[" << levelToString(level) << "] "
                  << message << std::endl;
    }

    template<typename... Args>
    void info(const std::format_string<Args...>& fmt, Args&&... args) {
        log(LogLevel::INFO, std::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void warning(const std::format_string<Args...>& fmt, Args&&... args) {
        log(LogLevel::WARNING, std::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void error(const std::format_string<Args...>& fmt, Args&&... args) {
        log(LogLevel::ERROR, std::format(fmt, std::forward<Args>(args)...));
    }

private:
    Logger() = default;
    std::mutex logMutex;

    std::string levelToString(LogLevel level) {
        switch (level) {
            case LogLevel::INFO:    return "INFO";
            case LogLevel::WARNING: return "WARNING";
            case LogLevel::ERROR:   return "ERROR";
            default:                return "UNKNOWN";
        }
    }
};

#define LOG_INFO(...) Logger::getInstance().info(__VA_ARGS__)
#define LOG_WARN(...) Logger::getInstance().warning(__VA_ARGS__)
#define LOG_ERR(...)  Logger::getInstance().error(__VA_ARGS__)
