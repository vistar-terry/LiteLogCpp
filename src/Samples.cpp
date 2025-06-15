#include "LiteLog.hpp"
#include <thread>

int main()
{
    // 配置日志系统
    Logger::instance().setLevel(LogLevel::Debug);
    Logger::instance().enableTimestamp(true);
    Logger::instance().setTimestampPrecision(TimestampPrecision::MICROSECONDS);
    Logger::instance().enableTags(true);
    Logger::instance().configureTag("INITIAL", ansi::yellow, ansi::bold);
    LOG_INFO_T("INITIAL", "初始配置日志系统, 设置日志等级为Debug, 开启时间戳显示, 设置微秒级时间戳, 显示标签");

    // 设置日志目录（自动创建目录和文件）
    Logger::instance().setLogDirectory("./logs", "myapp", true, true);

    // 获取当前日志文件路径
    LOG_INFO("当前日志文件路径: %s", Logger::instance().getLogFilePath().c_str());

    // 设置位置信息显示模式
    #ifdef PROJECT_ROOT
    // 如果定义了项目根目录，使用相对路径模式
    Logger::instance().setLocationMode(LocationDisplayMode::RELATIVE_PATH, PROJECT_ROOT);
    #else
    // 否则只显示文件名
    Logger::instance().setLocationMode(LocationDisplayMode::FILENAME_ONLY);
    #endif

    // 添加自定义标签配置
    Logger::instance().configureTag("AUDIO", ansi::magenta);
    Logger::instance().configureTag("PHYSICS", ansi::blue, ansi::bold);
    
    // 为特定标签设置不同的日志级别
    Logger::instance().setTagLevel("NETWORK", LogLevel::Warn);
    Logger::instance().setTagLevel("DEBUG", LogLevel::Trace);
    
    // 禁用某些标签
    Logger::instance().enableTag("SYSTEM", false);

    std::cout << std::endl;

    // 演示不同位置信息模式
    LOG_INFO("=== 位置信息模式演示 ===");
    
    // 完整路径模式
    Logger::instance().setLocationMode(LocationDisplayMode::FULL_PATH);
    LOG_INFO("完整路径模式: 显示文件的完整路径");
    
    // 只显示文件名
    Logger::instance().setLocationMode(LocationDisplayMode::FILENAME_ONLY);
    LOG_INFO("文件名模式: 只显示文件名");
    
    // 相对路径模式 (需要设置基础路径)
    #ifdef PROJECT_ROOT
    Logger::instance().setLocationMode(LocationDisplayMode::RELATIVE_PATH, PROJECT_ROOT);
    LOG_INFO("相对路径模式: 显示相对于项目根目录的路径");
    #endif
    
    // 不显示位置信息
    Logger::instance().setLocationMode(LocationDisplayMode::NONE);
    LOG_INFO("无位置信息模式: 不显示文件位置");
    
    // 恢复为默认模式
    Logger::instance().setLocationMode(LocationDisplayMode::FILENAME_ONLY);

    // 更改日志文件（自动关闭旧文件，打开新文件）
    Logger::instance().setLogFile("new_log.log");
    LOG_INFO("已切换到新日志文件");

    std::cout << std::endl;

    // 演示整行颜色控制
    LOG_INFO("=== 整行颜色控制演示 ===");
    
    // 启用整行颜色
    Logger::instance().setColorMode(ColorMode::LINE);
    LOG_TRACE("整行颜色: TRACE级别");
    LOG_DEBUG("整行颜色: DEBUG级别");
    LOG_INFO("整行颜色: INFO级别");
    LOG_WARN("整行颜色: WARN级别");
    LOG_ERROR("整行颜色: ERROR级别");
    LOG_FATAL("整行颜色: FATAL级别");
    
    // 禁用整行颜色
    Logger::instance().setColorMode(ColorMode::TAG);
    LOG_INFO("禁用整行颜色: 各部分单独着色");

    std::cout << std::endl;
    
    // 混合使用标签和位置信息
    LOG_INFO("=== 标签和位置信息混合演示 ===");
    LOG_INFO_T("MAIN", "主模块日志");
    LOG_DEBUG_T("DEBUG", "调试信息: %d", 42);
    LOG_WARN_T("NETWORK", "网络警告: 连接超时");
    LOG_ERROR_T("DATABASE", "数据库错误: 查询失败");

    std::cout << std::endl;

    // 演示不同时间精度
    LOG_INFO("=== 时间戳精度演示 ===");

    // 1. 秒级精度
    Logger::instance().setTimestampPrecision(TimestampPrecision::SECONDS);
    LOG_INFO("秒级精度时间戳");

    // 2. 毫秒级精度
    Logger::instance().setTimestampPrecision(TimestampPrecision::MILLISECONDS);
    LOG_INFO("毫秒级精度时间戳");

    // 3. 微秒级精度
    Logger::instance().setTimestampPrecision(TimestampPrecision::MICROSECONDS);
    LOG_INFO("微秒级精度时间戳");

    std::cout << std::endl;

    // 高精度时间测量
    LOG_INFO("=== 高精度时间测量演示 ===");

    auto start = std::chrono::high_resolution_clock::now();

    // 执行一些操作
    for (int i = 0; i < 1000000; i++)
    {
        // 模拟工作负载
        volatile int j = i * i;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    LOG_DEBUG("计算耗时: %lld 微秒", duration.count());

    std::cout << std::endl;

    // 精确时间间隔日志
    LOG_INFO("=== 精确时间间隔日志 ===");

    auto last_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 5; i++)
    {
        // 精确睡眠
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - last_time);
        last_time = now;

        LOG_INFO("精确时间间隔: %lld 微秒 (循环 %d)", elapsed.count(), i);
    }

    std::cout << std::endl;

    // 并发日志时间精度
    LOG_INFO("=== 并发日志时间精度 ===");

    auto log_task = [](int id)
    {
        for (int i = 0; i < 3; i++)
        {
            LOG_INFO_T("THREAD", "线程 %d - 日志 %d", id, i);
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    };

    std::thread t1(log_task, 1);
    std::thread t2(log_task, 2);
    std::thread t3(log_task, 3);

    t1.join();
    t2.join();
    t3.join();

    // 获取当前日志文件路径
    LOG_INFO("当前日志文件: %s", Logger::instance().getLogFilePath().c_str());

    // 关闭日志文件（可选，析构时会自动关闭）
    Logger::instance().closeLogFile();
    
    // 此时日志不会写入文件
    LOG_INFO("这条日志不会写入文件");

    return 0;
}