#include "Logger.h"

#include <fmt/os.h>

#include <ctime>


namespace {
enum : size_t {
    MAX_LINES = 10,
};

}  // namespace

namespace std_fs = std::filesystem;


Logger::~Logger() {
    is_work_ = false;
    write_thread_->join();
    fclose( output );
}

void Logger::write_task() {
    while ( is_work_ || !block_que_.empty() ) {
        //        std::time_t temp_time   = std::time( nullptr );
        //        std::unique_ptr<std::tm> system_time{std::localtime(&temp_time)};
        auto system_time = get_localtime();
        // check date and max line
        if ( today_ != system_time->tm_mday || LINES_ >= MAX_LINES ) {
            today_           = system_time->tm_mday;
            LINES_           = 0;
            std::string  ymd = fmt::format( "{:%Y-%m-%d}", *system_time );
            std::string  hms = fmt::format( "{:%H-%M-%S}", *system_time );
            std_fs::path log_path{ path_append( "./log/", { ymd, "/", hms, ".log" } ) };
            if ( !std_fs::exists( log_path.parent_path() ) ) {
                std_fs::create_directories( log_path.parent_path() );
            }
            fclose( output );
            output = fopen( log_path.c_str(), "w" );
        }
        folly::IOBuf buf;
        block_que_.pop( buf );
        {
            std::unique_lock locker( file_mtx_ );
            fprintf( output, "%.*s\n", buf.length(), buf.data() );
            ++LINES_;
        }
    }
}

std::string Logger::format_log_message( fmt::string_view format, fmt::format_args args ) {
    return fmt::vformat( format, args );
}

std::string Logger::path_append( const std::string &base_path, const std::initializer_list<std::string_view> suffix_list ) {
    std::string result{ base_path };
    for ( const auto &item : suffix_list ) {
        result.append( item );
    }
    return result;
}
//inline constexpr const char *Logger::level_to_str( Logger::LogLevel level ) noexcept {
//    switch ( level ) {
//    case LogLevel::INFO:
//        return "[INFO] : ";
//    case LogLevel::WARN:
//        return "[WARN] : ";
//    case LogLevel::DEBUG:
//        return "[DEBUG]: ";
//    case LogLevel::ERROR:
//        return "[ERROR]: ";
//    }
//}
