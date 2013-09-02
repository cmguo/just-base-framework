// ConsoleStream.cpp

#include "framework/Framework.h"
#include "framework/logger/ConsoleStream.h"
#include "framework/logger/Context.h"
#include "framework/configure/Config.h"
using namespace framework::configure;

#include <stdio.h>

#ifdef BOOST_WINDOWS_API
#  include <windows.h>
#else
#  include <sys/uio.h>
#  include <unistd.h>
#endif

#ifdef BOOST_WINDOWS_API

#define FOREGROUND_BROWN    FOREGROUND_GREEN | FOREGROUND_BLUE
#define FOREGROUND_YELLOW   FOREGROUND_RED | FOREGROUND_GREEN
#define FOREGROUND_WHITE    FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE

int clr[] = {
    FOREGROUND_WHITE,                           // Off
    FOREGROUND_INTENSITY | FOREGROUND_BROWN,    // Fatal
    FOREGROUND_INTENSITY | FOREGROUND_RED,      // Error
    FOREGROUND_INTENSITY | FOREGROUND_YELLOW,   // Warn
    FOREGROUND_INTENSITY | FOREGROUND_GREEN,    // Info
    FOREGROUND_INTENSITY | FOREGROUND_WHITE,    // Debug
    FOREGROUND_WHITE,                           // Trace
    FOREGROUND_WHITE,                           // All
};

#else
#include <sys/uio.h>
#include <unistd.h>
static char const * const color_str[] = {
    "\033[0m",      // Off
    "\033[5;31m",   // Fatal
    "\033[1;31m",   // Error
    "\033[1;33m",   // Warn
    "\033[1;32m",   // Info
    "\033[1;37m",   // Debug
    "\033[0m",      // Trace
    "\033[0m",      // All
};
static const size_t color_str_len[] = {4, 7, 7, 7, 7, 7, 4, 4};
#endif

namespace framework
{
    namespace logger
    {

        ConsoleStream::ConsoleStream()
            : color_(false) 
        {
            boost::system::error_code ec;
#ifdef BOOST_WINDOWS_API
            file_.assign(GetStdHandle(STD_OUTPUT_HANDLE), ec);
#else
            file_.assign(fileno(stdout));
#endif
        }

        ConsoleStream::~ConsoleStream()
        {
        }

        void ConsoleStream::load_config(
            ConfigModule & cm)
        {
            Stream::load_config(cm);

            cm << CONFIG_PARAM_NAME_RDWR("color", color_);
        }

        void ConsoleStream::write(
            buffers_t const & buffers)
        {
            buffers_t buffers2 = buffers;
            if (color_) {
#ifdef BOOST_WINDOWS_API
                LevelEnum lvl = str_to_level(boost::asio::buffer_cast<char const *>(buffers[mi_level])[1]);
                /// 打印有颜色的串
                ::SetConsoleTextAttribute(file_.native(), clr[lvl]);
                boost::system::error_code ec;
                file_.write_some(buffers, ec);
#else
                buffers2 = buffers_t(buffers.address() - 1, buffers.size() + 1);
                buffers2[0] = boost::asio::buffer(color_str[lvl], color_str_len[lvl]);
                boost::system::error_code ec;
                file_.write_some(buffers2, ec);
#endif
            }
            boost::system::error_code ec;
            file_.write_some(buffers2, ec);
        }

    } // namespace logger
} // namespace framework
