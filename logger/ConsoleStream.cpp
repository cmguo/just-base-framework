// ConsoleStream.cpp

#include "framework/Framework.h"
#include "framework/logger/ConsoleStream.h"
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

int clr[8] = {
    FOREGROUND_INTENSITY | FOREGROUND_RED, 
    FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN, 
    FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE, 
    FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE, 
    FOREGROUND_INTENSITY | FOREGROUND_GREEN, 
    FOREGROUND_INTENSITY | FOREGROUND_GREEN, 
    FOREGROUND_INTENSITY | FOREGROUND_GREEN, 
    FOREGROUND_INTENSITY | FOREGROUND_GREEN
};

#  define SETCOLOR(handle, color) 
#  define CLEARCOLOR() ::SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE )
#else
#include <sys/uio.h>
#include <unistd.h>
char const clr[8] = {'1', '3', '2', '4', '6', '6', '6'};
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
            cm << CONFIG_PARAM_NAME_RDWR("color", color_);
        }

        void ConsoleStream::write( 
            buffer_t const * logmsgs, 
            size_t len )
        {
            if ( !len ) return;

            if (color_) {
                LevelEnum lvl = str_to_level(logmsgs[mi_level].buf[1]);
#ifdef BOOST_WINDOWS_API
                /// 打印有颜色的串
                ::SetConsoleTextAttribute(handle_, clr[lvl]);
#else
                logmsgs[mi_color].buf[4] = clr[lvl];
#endif
            }
            else 
            {
#ifdef BOOST_WINDOWS_API
#else
                ++logmsgs;
                len -= 2;
#endif
            }

#ifdef BOOST_WINDOWS_API
            DWORD dw = 0;
            for (size_t iLoop = 0; iLoop < len; ++iLoop) {
                ::WriteFile(
                    handle_, 
                    logmsgs[iLoop].buf, 
                    logmsgs[iLoop].len, 
                    &dw, 
                    NULL);
            }
#  else 
            ::writev(
                fd_, 
                (iovec *)logmsgs, 
                len);
#endif

            if (color_) {
#ifdef BOOST_WINDOWS_API
                ::SetConsoleTextAttribute(handle_, clr[0]);
#else
                logmsgs[mi_color].buf[4] = clr[0];
#endif
            }
        }

    } // namespace logger
} // namespace framework
