#include "logger.h"
#include <iostream>
#include <cstring>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace dune {

Logger& Logger::instance() {
    static Logger ins;
    return ins;
}

void Logger::init(const std::string& filepath, LogLevel min_level) {
    min_level_ = min_level;
    file_.open(filepath, std::ios::app);
    if (file_.is_open()) {
        use_file_ = true;
    } else {
        std::cerr << "[Logger] cannot open log file: " << filepath << std::endl;
    }
}

void Logger::init_console(LogLevel min_level) {
    min_level_ = min_level;
    use_file_ = false;
}

void Logger::log(LogLevel level, const char* file, int line,
                 const std::string& msg) {
    if (level < min_level_) return;

    std::lock_guard<std::mutex> lock(mutex_);

    // 从完整路径中提取文件名
    const char* fname = file;
    const char* p = strrchr(file, '/');
    if (p) fname = p + 1;

    std::ostringstream oss;
    oss << timestr() << " [" << level_str(level) << "] "
        << fname << ":" << line << "  " << msg;

    if (use_file_ && file_.is_open()) {
        file_ << oss.str() << std::endl;
        file_.flush();
    } else {
        std::cout << oss.str() << std::endl;
    }
}

const char* Logger::level_str(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO ";
        case LogLevel::WARN:  return "WARN ";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
    }
    return "????";
}

std::string Logger::timestr() {
    auto now = std::chrono::system_clock::now();
    auto t   = std::chrono::system_clock::to_time_t(now);
    auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(
                   now.time_since_epoch()) % 1000;

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S")
        << "." << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

} // namespace dune
