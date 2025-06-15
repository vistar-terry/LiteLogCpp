//   _      _  _          _                     ____
//  | |    (_)| |_  ___  | |     ___    __ _   / ___| _ __   _ __
//  | |    | || __|/ _ \ | |    / _ \  / _` | | |    | '_ \ | '_ \ 
//  | |___ | || |_|  __/ | |___| (_) || (_| | | |___ | |_) || |_) |
//  |_____||_| \__|\___| |_____|\___/  \__, |  \____|| .__/ | .__/
//                                     |___/         |_|    |_|
// https://github.com/vistar-terry/LiteLogCpp
// Lite Log Cpp - version 0.1.0
//
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// SPDX-License-Identifier: MIT
// Copyright (c) 2025 <vistar-terry>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef _LITELOG_HPP_
#define _LITELOG_HPP_

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdarg>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <mutex>
#include <vector>
#include <cstring>
#include <unordered_map>
#include <memory>
#include <stdexcept>
#include <cctype>
#include <cstdio>

// 添加必要的系统头文件
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <cerrno>
#endif

// ======================
// 日志级别定义
// ======================
enum class LogLevel
{
    Trace = 0, // 最详细的调试信息
    Debug,     // 调试信息
    Info,      // 常规信息 (默认)
    Warn,      // 警告信息
    Error,     // 错误信息
    Fatal,     // 严重错误
    OFF        // 关闭所有日志
};

// ======================
// ANSI 颜色代码定义
// ======================
namespace ansi
{
    constexpr const char *reset = "\033[0m";
    constexpr const char *bold = "\033[1m";

    // 前景色
    constexpr const char *black = "\033[30m";
    constexpr const char *red = "\033[31m";
    constexpr const char *green = "\033[32m";
    constexpr const char *yellow = "\033[33m";
    constexpr const char *blue = "\033[34m";
    constexpr const char *magenta = "\033[35m";
    constexpr const char *cyan = "\033[36m";
    constexpr const char *white = "\033[37m";

    // 背景色
    constexpr const char *bg_red = "\033[41m";
    constexpr const char *bg_green = "\033[42m";
    constexpr const char *bg_yellow = "\033[43m";
    constexpr const char *bg_blue = "\033[44m";
    constexpr const char *bg_magenta = "\033[45m";
    constexpr const char *bg_cyan = "\033[46m";
    constexpr const char *bg_white = "\033[47m";
}

// ======================
// 时间戳精度设置
// ======================
enum class TimestampPrecision
{
    SECONDS = 0,  // 秒级精度
    MILLISECONDS, // 毫秒级精度 (默认)
    MICROSECONDS  // 微秒级精度
};

// ======================
// 位置信息显示模式
// ======================
enum class LocationDisplayMode
{
    FULL_PATH = 0, // 显示完整路径
    FILENAME_ONLY, // 只显示文件名 (默认)
    RELATIVE_PATH, // 显示相对路径
    NONE           // 不显示位置信息
};

// ======================
// 彩色输出模式
// ======================
enum class ColorMode
{
    OFF = 0, // 关闭
    TAG,     // 标签和等级彩色输出 (默认)
    LINE     // 整行彩色输出
};

// ======================
// 标签颜色配置
// ======================
struct TagConfig
{
    const char *color = ansi::cyan; // 标签文本颜色
    const char *style = "";         // 标签文本样式
    bool enabled = true;            // 是否启用该标签的日志

    TagConfig() {}

    TagConfig(const char *_color, const char *_style, bool _enabled)
        : color(_color), style(_style), enabled(_enabled)
    {
    }
};

// ======================
// 日志系统核心类
// ======================
class Logger
{
public:
    // 获取单例实例
    static Logger &instance()
    {
        static Logger instance;
        return instance;
    }

    // 设置全局日志级别
    void setLevel(LogLevel level)
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        current_level_ = level;
    }

    // 设置标签日志级别
    void setTagLevel(const std::string &tag, LogLevel level)
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        tag_levels_[tag] = level;
    }

    // 开启/禁用控制台输出
    void consoleOutput(const bool& console_output)
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        console_output_ = console_output;
    }

    // 设置日志文件路径（自动管理文件）
    bool setLogFile(const std::string &file_path, bool append = true)
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);

        try
        {
            // 关闭当前文件（如果有）
            file_output_.reset();

            // 打开新文件
            auto mode = std::ios::out | std::ios::ate;
            if (append)
            {
                mode |= std::ios::app;
            }

            auto new_file = std::unique_ptr<std::ofstream>(new std::ofstream(file_path, mode));
            if (!new_file->is_open())
            {
                return false;
            }

            file_output_ = std::move(new_file);
            file_path_ = file_path;
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    // 设置日志目录（自动创建目录）
    bool setLogDirectory(const std::string &dir_path,
                         const std::string &file_prefix = "app",
                         bool append = true,
                         bool daily_rotation = false)
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);

        try
        {
            // 创建目录（如果不存在）
            if (!directoryExists(dir_path))
            {
                if (!createDirectoryRecursive(dir_path))
                {
                    return false;
                }
            }

            // 生成文件名
            std::string filename;
            if (daily_rotation)
            {
                // 按日期生成文件名
                auto now = std::chrono::system_clock::now();
                auto now_time_t = std::chrono::system_clock::to_time_t(now);
                std::tm tm;
#ifdef _WIN32
                localtime_s(&tm, &now_time_t);
#else
                localtime_r(&now_time_t, &tm);
#endif

                char date_buffer[16];
                std::strftime(date_buffer, sizeof(date_buffer), "%Y%m%d", &tm);
                filename = file_prefix + "_" + date_buffer + ".log";
            }
            else
            {
                // 固定文件名
                filename = file_prefix + ".log";
            }

            // 完整路径（使用平台无关的路径分隔符）
            std::string full_path = dir_path;
            if (dir_path.back() != '/' && dir_path.back() != '\\')
            {
                full_path += '/';
            }
            full_path += filename;

            // 设置日志文件
            return setLogFile(full_path, append);
        }
        catch (...)
        {
            return false;
        }
    }

    // 关闭日志文件
    void closeLogFile()
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        file_output_.reset();
        file_path_.clear();
    }

    // 配置标签显示
    void configureTag(const std::string &tag, const char *color, const char *style = "", bool enabled = true)
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        tag_configs_[tag] = TagConfig(color, style, enabled);
    }

    // 启用/禁用特定标签
    void enableTag(const std::string &tag, bool enabled)
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        if (tag_configs_.find(tag) != tag_configs_.end())
        {
            tag_configs_[tag].enabled = enabled;
        }
        else
        {
            tag_configs_[tag] = TagConfig(ansi::cyan, "", enabled);
        }
    }

    // 设置颜色模式
    void setColorMode(ColorMode color_mode)
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        color_mode_ = color_mode;
    }

    // 启用/禁用时间戳
    void enableTimestamp(bool enabled)
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        show_timestamp_ = enabled;
    }

    // 设置时间戳精度
    void setTimestampPrecision(TimestampPrecision precision)
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        timestamp_precision_ = precision;
    }

    // 设置位置信息显示模式
    void setLocationMode(LocationDisplayMode mode, const std::string &base_path = "")
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        location_mode_ = mode;
        base_path_ = base_path;
    }

    // 启用/禁用标签显示
    void enableTags(bool enabled)
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        show_tags_ = enabled;
    }

    // 获取当前日志文件路径
    std::string getLogFilePath() const
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        return file_path_;
    }

    // 日志记录函数 (printf 风格)
    void log(LogLevel level, const char *tag, const char *file, int line, const char *function,
             const char *format, ...)
    {
        if (level == LogLevel::OFF)
            return;

        // 检查标签是否启用
        if (tag)
        {
            std::lock_guard<std::recursive_mutex> lock(mutex_);
            auto tag_config_it = tag_configs_.find(tag);
            if (tag_config_it != tag_configs_.end() && !tag_config_it->second.enabled)
            {
                return; // 标签被禁用
            }
        }

        // 检查日志级别
        LogLevel effective_level = LogLevel::Info;
        {
            std::lock_guard<std::recursive_mutex> lock(mutex_);
            effective_level = getEffectiveLevel(tag);
        }
        if (level < effective_level)
            return;

        // 格式化消息 - 使用动态缓冲区防止截断
        va_list args;
        va_start(args, format);
        // 获取所需缓冲区大小
        int needed_size = vsnprintf(nullptr, 0, format, args);
        va_end(args);

        if (needed_size < 0)
            return; // 格式化错误

        std::vector<char> message_buffer(needed_size + 1); // +1 for null terminator
        va_start(args, format);
        vsnprintf(message_buffer.data(), message_buffer.size(), format, args);
        va_end(args);

        std::string log_entry;
        {
            std::lock_guard<std::recursive_mutex> lock(mutex_);

            // 获取高精度时间戳
            std::string time_str = show_timestamp_ ? getHighPrecisionTimestamp() : "";

            // 处理位置信息
            std::string location_info;
            if (file && function && location_mode_ != LocationDisplayMode::NONE)
            {
                location_info = getLocationInfo(file, function, line);
            }

            // 创建日志流
            std::ostringstream log_stream;

            // 整行颜色控制
            if (color_mode_ == ColorMode::LINE)
            {
                log_stream << getLevelColor(level) << getLevelStyle(level);
            }

            // 添加时间戳
            log_stream << time_str;

            // 添加日志级别
            if (color_mode_ == ColorMode::TAG)
            {
                log_stream << getLevelColor(level) << getLevelStyle(level)
                           << "[" << levelToString(level) << "]" << ansi::reset;
            }
            else
            {
                log_stream << "[" << levelToString(level) << "]";
            }

            // 添加标签
            if (show_tags_ && tag && tag[0] != '\0')
            {
                if (color_mode_ == ColorMode::TAG)
                {
                    TagConfig config = getTagConfig(tag);
                    log_stream << config.style << config.color
                               << "[" << tag << "]" << ansi::reset;
                }
                else
                {
                    log_stream << "[" << tag << "]";
                }
            }

            // 添加位置信息
            log_stream << location_info;

            // 添加消息
            log_stream << " " << message_buffer.data();

            // 整行颜色结束
            if (color_mode_ == ColorMode::LINE)
            {
                log_stream << ansi::reset;
            }

            // 输出日志
            log_entry = log_stream.str();
        }

        {
            std::lock_guard<std::recursive_mutex> lock(mutex_);

            // 输出到控制台
            if (console_output_)
            {
                std::cerr << log_entry << std::endl;
                if (level >= LogLevel::Error)
                {
                    std::cerr.flush();
                }
            }

            // 输出到文件
            if (file_output_ && file_output_->is_open())
            {
                *file_output_ << log_entry << std::endl;
                if (level >= LogLevel::Error)
                {
                    file_output_->flush();
                }
            }
        }
    }

private:
    Logger()
        : current_level_(LogLevel::Info),
          console_output_(true),
          color_mode_(ColorMode::TAG),
          show_timestamp_(true),
          timestamp_precision_(TimestampPrecision::MILLISECONDS),
          location_mode_(LocationDisplayMode::FILENAME_ONLY),
          show_tags_(true)
    {
        // 预配置一些常用标签
        configureTag("NETWORK", ansi::blue);
        configureTag("DATABASE", ansi::magenta);
        configureTag("UI", ansi::green);
        configureTag("SYSTEM", ansi::yellow);
        configureTag("SECURITY", ansi::red);
    }

    // 禁止复制
    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;

    ~Logger()
    {
        // 自动关闭文件
        file_output_.reset();
    }

    // 检查目录是否存在
    bool directoryExists(const std::string &path)
    {
#ifdef _WIN32
        DWORD attrib = GetFileAttributesA(path.c_str());
        return (attrib != INVALID_FILE_ATTRIBUTES) &&
               (attrib & FILE_ATTRIBUTE_DIRECTORY);
#else
        struct stat st;
        if (stat(path.c_str(), &st) != 0)
            return false;
        return S_ISDIR(st.st_mode);
#endif
    }

    // 递归创建目录
    bool createDirectoryRecursive(const std::string &path)
    {
        // 空路径直接返回
        if (path.empty())
            return true;

        // 检查目录是否已存在
        if (directoryExists(path))
            return true;

        // 提取父目录
        size_t sep_pos = path.find_last_of("/\\");
        if (sep_pos != std::string::npos)
        {
            std::string parent = path.substr(0, sep_pos);
            // 递归创建父目录
            if (!parent.empty() && !createDirectoryRecursive(parent))
            {
                return false;
            }
        }

        // 创建当前目录
#ifdef _WIN32
        int result = _mkdir(path.c_str());
        return result == 0 || errno == EEXIST;
#else
        int result = mkdir(path.c_str(), 0755);
        return result == 0 || errno == EEXIST;
#endif
    }

    // 获取高精度时间戳
    std::string getHighPrecisionTimestamp()
    {
        using namespace std::chrono;

        // 获取当前时间
        auto now = system_clock::now();
        auto now_time_t = system_clock::to_time_t(now);

        // 转换为时间结构
        std::tm tm;
#ifdef _WIN32
        localtime_s(&tm, &now_time_t);
#else
        localtime_r(&now_time_t, &tm);
#endif

        // 格式化为字符串
        std::ostringstream oss;
        oss << "[" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");

        // 添加毫秒/微秒部分
        auto since_epoch = now.time_since_epoch();
        auto sec_duration = duration_cast<std::chrono::seconds>(since_epoch);
        since_epoch -= sec_duration;

        switch (timestamp_precision_)
        {
        case TimestampPrecision::MILLISECONDS:
        {
            auto milliseconds = duration_cast<std::chrono::milliseconds>(since_epoch);
            oss << "." << std::setfill('0') << std::setw(3) << milliseconds.count();
            break;
        }
        case TimestampPrecision::MICROSECONDS:
        {
            auto microseconds = duration_cast<std::chrono::microseconds>(since_epoch);
            oss << "." << std::setfill('0') << std::setw(6) << microseconds.count();
            break;
        }
        default:
            break;
        }

        oss << "]";
        return oss.str();
    }

    // 获取位置信息
    std::string getLocationInfo(const char *file, const char *function, int line)
    {
        if (!file || !function)
            return "";

        std::string file_str(file);

        // 根据模式处理文件路径
        if (location_mode_ == LocationDisplayMode::FILENAME_ONLY)
        {
            // 只显示文件名
            size_t pos = file_str.find_last_of("/\\");
            if (pos != std::string::npos)
            {
                file_str = file_str.substr(pos + 1);
            }
        }
        else if (location_mode_ == LocationDisplayMode::RELATIVE_PATH && !base_path_.empty())
        {
            // 显示相对路径
            // 确保 base_path 以分隔符结尾
            std::string normalized_base = base_path_;
            if (normalized_base.back() != '/' && normalized_base.back() != '\\')
            {
                normalized_base += '/';
            }

            // 检查文件路径是否以 base_path 开头
            if (file_str.find(normalized_base) == 0)
            {
                file_str = file_str.substr(normalized_base.length());
            }
        }
        // FULL_PATH 模式保持原样

        std::ostringstream loc_stream;
        loc_stream << "[" << file_str << ":" << line << "-" << function << "]";
        return loc_stream.str();
    }

    // 获取有效的日志级别（考虑标签特定级别）
    LogLevel getEffectiveLevel(const char *tag)
    {
        if (!tag)
            return current_level_;

        auto it = tag_levels_.find(tag);
        if (it != tag_levels_.end())
        {
            return it->second;
        }
        return current_level_;
    }

    // 获取标签配置
    TagConfig getTagConfig(const std::string &tag)
    {
        auto it = tag_configs_.find(tag);
        if (it != tag_configs_.end())
        {
            return it->second;
        }
        return {ansi::cyan, "", true}; // 默认配置
    }

    // 日志级别转字符串
    const char *levelToString(LogLevel level)
    {
        static const char *const buffer[] = {
            "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL", "OFF"};
        return buffer[static_cast<int>(level)];
    }

    // 获取日志级别颜色
    const char *getLevelColor(LogLevel level)
    {
        switch (level)
        {
        case LogLevel::Trace:
            return ansi::cyan;
        case LogLevel::Debug:
            return ansi::blue;
        case LogLevel::Info:
            return ansi::green;
        case LogLevel::Warn:
            return ansi::yellow;
        case LogLevel::Error:
            return ansi::red;
        case LogLevel::Fatal:
            return ansi::magenta;
        default:
            return ansi::white;
        }
    }

    // 获取日志级别样式
    const char *getLevelStyle(LogLevel level)
    {
        switch (level)
        {
        case LogLevel::Fatal:
            return ansi::bold;
        case LogLevel::Error:
            return ansi::bold;
        default:
            return "";
        }
    }

    // 成员变量
    LogLevel current_level_;
    bool console_output_;                        // 是否输出到控制台
    std::unique_ptr<std::ofstream> file_output_; // 文件输出流
    std::string file_path_;                      // 当前日志文件路径

    std::unordered_map<std::string, LogLevel> tag_levels_;
    std::unordered_map<std::string, TagConfig> tag_configs_;

    ColorMode color_mode_;
    bool show_timestamp_;
    TimestampPrecision timestamp_precision_;
    LocationDisplayMode location_mode_;
    std::string base_path_;
    bool show_tags_;

    mutable std::recursive_mutex mutex_;
};

// ======================
// 日志宏定义 (带标签)
// ======================
#define LOG_TRACE_T(tag, fmt, ...) Logger::instance().log(LogLevel::Trace, tag, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define LOG_DEBUG_T(tag, fmt, ...) Logger::instance().log(LogLevel::Debug, tag, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define LOG_INFO_T(tag, fmt, ...) Logger::instance().log(LogLevel::Info, tag, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define LOG_WARN_T(tag, fmt, ...) Logger::instance().log(LogLevel::Warn, tag, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define LOG_ERROR_T(tag, fmt, ...) Logger::instance().log(LogLevel::Error, tag, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define LOG_FATAL_T(tag, fmt, ...) Logger::instance().log(LogLevel::Fatal, tag, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

// ======================
// 日志宏定义 (无标签)
// ======================
#define LOG_TRACE(fmt, ...) Logger::instance().log(LogLevel::Trace, nullptr, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) Logger::instance().log(LogLevel::Debug, nullptr, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) Logger::instance().log(LogLevel::Info, nullptr, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) Logger::instance().log(LogLevel::Warn, nullptr, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) Logger::instance().log(LogLevel::Error, nullptr, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define LOG_FATAL(fmt, ...) Logger::instance().log(LogLevel::Fatal, nullptr, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

#endif // _LITELOG_HPP_