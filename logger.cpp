#include "logger.h"    

void Logger::init(const std::string& program, const std::string& path) {
    this->program = program;
    this->file.open(path, std::ios::app);

    if (!this->file.is_open()) {
        std::cerr << "[WARN] Cannot open log file: " << path << std::endl;
    }
}
 
void Logger::log(const std::string& event, int32_t seq, const std::string& extra = "") {
    std::string line = now_ts() + " | " + program + " | " + event + " | "
                     + (seq == -1 ? "" : std::to_string(seq) + " | ") + extra;

    std::cerr << line << std::endl;

    if (this->file.is_open()) {
        this->file << line << std::endl;
        this->file.flush();
    }
}
 
void Logger::log(const std::string& event, const std::string& extra = "") {
    log(event, -1, extra);
}

std::string Logger::now_ts() {
    using namespace std::chrono;
    auto now   = system_clock::now();
    auto now_t = system_clock::to_time_t(now);
    auto ms    = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    struct tm tm_buf {};
    localtime_r(&now_t, &tm_buf);

    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%dT%H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3) << ms.count();

    return oss.str();
}

