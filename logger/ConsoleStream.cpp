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
static const size_t color_str_len[] = {4, , 6, 6, 6, 6, 4, 4};
#endif

namespace framework
{
    namespace logger
    {

        ConsoleStream::ConsoleStream()
#ifdef BOOST_WINDOWS_API
            : handle_(GetStdHandle(STD_OUTPUT_HANDLE))
#else
            : fd_(stdout)
#endif
            , color_(false) 
        {
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
            buffer_t const * bufs, 
            size_t len )
        {
            if ( !len ) return;

            if (color_) {
                LevelEnum lvl = str_to_level(bufs[mi_level].buf[1]);
#ifdef BOOST_WINDOWS_API
                /// 打印有颜色的串
                ::SetConsoleTextAttribute(handle_, clr[lvl]);
#else
                --bufs;
                len += 1;
                ((buffer_t *)bufs)->buf = color_str[lvl];
                bufs->len = color_str_len[lvl];
#endif
            }

#ifdef BOOST_WINDOWS_API
            DWORD dw = 0;
            for (size_t iLoop = 0; iLoop < len; ++iLoop) {
                ::WriteFile(
                    handle_, 
                    bufs[iLoop].buf, 
                    bufs[iLoop].len, 
                    &dw, 
                    NULL);
            }
#  else 
            ::writev(
                fd_, 
                (iovec *)bufs, 
                len);
#endif

            if (color_) {
#ifdef BOOST_WINDOWS_API
                ::SetConsoleTextAttribute(handle_, clr[0]);
#endif
            }
        }

    } // namespace logger
} // namespace framework
