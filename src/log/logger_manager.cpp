#include "logger_manager.h"
#include "log4cplus/consoleappender.h"
#include "log4cplus/fileappender.h"
#include "log4cplus/initializer.h"

ecs::log::internal::LoggerManager::LoggerManager()
{
    log4cplus::initialize();

    // create a conversion pattern
    const log4cplus::tstring PATTERN = LOG4CPLUS_TEXT(LOG_PATTERN);
    // create console log output
    log4cplus::SharedAppenderPtr consoleApp(new log4cplus::ConsoleAppender());
    consoleApp->setLayout(std::unique_ptr<log4cplus::Layout>(new log4cplus::PatternLayout(PATTERN)));

    // create log file output
    log4cplus::SharedAppenderPtr fileApp(new log4cplus::FileAppender(LOG4CPLUS_TEXT(LOG_FILE_NAME)));
    fileApp->setLayout(std::unique_ptr<log4cplus::Layout>(new log4cplus::PatternLayout(PATTERN)));

    // get root logger
    this->m_RootLogger = log4cplus::Logger::getRoot();

    // set root log level
    this->m_RootLogger.setLogLevel(log4cplus::ERROR_LOG_LEVEL);

    // add appenders
    this->m_RootLogger.addAppender(consoleApp);
    this->m_RootLogger.addAppender(fileApp);
}

ecs::log::internal::LoggerManager::~LoggerManager()
{
    // cleanup logger
    for (auto& it : this->m_Cache)
    {
        delete it.second;
        it.second = nullptr;
    }

    log4cplus::Logger::getRoot().shutdown();
    log4cplus::threadCleanup();
}

ecs::log::Logger* ecs::log::internal::LoggerManager::GetLogger(const char* name)
{
    auto it = this->m_Cache.find(name);
    if (it == this->m_Cache.end())
    {
        auto tmp = this->m_RootLogger.getInstance(name);
        this->m_Cache.insert(std::make_pair<std::string, Logger*>(name, new Logger(tmp)));
    }

    return this->m_Cache[name];
}
