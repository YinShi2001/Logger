#ifndef LOGGER_H
#define LOGGER_H

#include "block_queue.h"

#include <fmt/chrono.h>
#include <fmt/format.h>
#include <fmt/os.h>

#include <folly/io/IOBuf.h>

#include <ctime>
#include <filesystem>
#include <initializer_list>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

class Logger {
public:
    enum class LogLevel {
        INFO,
        WARN,
        DEBUG,
        ERROR,
    };

    Logger( Logger const & ) = delete;

    Logger &operator=( Logger const & ) = delete;

    static Logger &get_instance() {
        static Logger singleton;
        return singleton;
    }

    template <typename... Args>
    void write( LogLevel level, fmt::basic_string_view<char> format, Args... args ) {
        auto system_time = get_localtime();

        auto formatted_str = fmt::format( "{0:%Y-%m-%d}    {0:%H:%M:%S}", *system_time );
        formatted_str.append( level_to_str( level ) );
        formatted_str.append( format_log_message( format, fmt::make_format_args( args... ) ) );

        auto temp = folly::IOBuf::copyBuffer( formatted_str, formatted_str.size() );
        block_que_.push_back( std::move( *temp ) );
    }

private:
    int LINES_;

    int today_;

    bool is_work_;

    FILE *output;

    BlockDeque<folly::IOBuf> block_que_;

    std::mutex file_mtx_;

    std::unique_ptr<std::thread> write_thread_;

    explicit Logger()
      : LINES_( 0 )
      , is_work_( true ) {

        //        std::time_t temp_time   = std::time( nullptr );
        //        std::tm    *system_time = std::localtime( &temp_time );
//        std::unique_ptr<std::tm> system_time{ get_localtime() };
        auto system_time = get_localtime();
        today_ = system_time->tm_mday;

        std::string           ymd = fmt::format( "{:%Y-%m-%d}", *system_time );
        std::string           hms = fmt::format( "{:%H-%M-%S}", *system_time );
        std::filesystem::path log_path{ path_append( "./log/", { ymd, "/", hms, ".log" } ) };
        if ( !exists( log_path.parent_path() ) ) {
            std::filesystem::create_directories( log_path.parent_path() );
        }
        output = fopen( log_path.c_str(), "w" );

        write_thread_ = std::make_unique<std::thread>( [ &, this ] { this->write_task(); } );
    }

    ~Logger();

    void write_task();

    static std::string format_log_message( fmt::string_view format, fmt::format_args args );

    static std::string path_append( const std::string &base_path, std::initializer_list<std::string_view> suffix_list );

    static std::tm *get_localtime() {
        std::time_t temp = std::time( nullptr );
        return std::localtime( &temp );
    }

    inline static constexpr const char *level_to_str( LogLevel level ) noexcept;
};

inline constexpr const char *Logger::level_to_str( Logger::LogLevel level ) noexcept {
    switch ( level ) {
    case LogLevel::INFO:
        return "[INFO] : ";
    case LogLevel::WARN:
        return "[WARN] : ";
    case LogLevel::DEBUG:
        return "[DEBUG]: ";
    case LogLevel::ERROR:
        return "[ERROR]: ";
    }
}

#define LOG_BASE( level, format, ... )                                                    \
    do {                                                                                  \
        Logger::get_instance().write( ::Logger::LogLevel::level, format, ##__VA_ARGS__ ); \
    } while ( 0 );

#endif
