#ifndef DUNE_LOG_LOGGER_H_
#define DUNE_LOG_LOGGER_H_

#include <string>
#include <fstream>
#include <mutex>

namespace dune {

enum class LogLevel {
    DEBUG = 0,
    INFO  = 1,
    WARN  = 2,
    ERROR = 3,
    FATAL = 4
};

class Logger {
public:
    static Logger& instance();

    void init(const std::string& filepath, LogLevel min_level = LogLevel::INFO);
    void init_console(LogLevel min_level = LogLevel::DEBUG);

    void log(LogLevel level, const char* file, int line,
             const std::string& msg);

    LogLevel level() const { return min_level_; }

private:
    Logger() = default;

    static const char* level_str(LogLevel level);
    std::string timestr();

    std::ofstream file_;
    bool use_file_ = false;
    LogLevel min_level_ = LogLevel::DEBUG;
    std::mutex mutex_;
};

// 便捷宏
#define LOG_DEBUG(msg) dune::Logger::instance().log(dune::LogLevel::DEBUG, __FILE__, __LINE__, msg)
#define LOG_INFO(msg)  dune::Logger::instance().log(dune::LogLevel::INFO,  __FILE__, __LINE__, msg)
#define LOG_WARN(msg)  dune::Logger::instance().log(dune::LogLevel::WARN,  __FILE__, __LINE__, msg)
#define LOG_ERROR(msg) dune::Logger::instance().log(dune::LogLevel::ERROR, __FILE__, __LINE__, msg)
#define LOG_FATAL(msg) dune::Logger::instance().log(dune::LogLevel::FATAL, __FILE__, __LINE__, msg)

} // namespace dune

#endif // DUNE_LOG_LOGGER_H_
