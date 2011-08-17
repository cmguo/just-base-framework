// LoggerStdStream.cpp

#include "framework/Framework.h"
#include "framework/logger/LoggerStdStream.h"

#include <stdio.h>

#ifdef BOOST_WINDOWS_API
#include <windows.h>
#define SETCOLOR( color ) ::SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),color )
#define CLEARCOLOR() ::SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE )
int clr[8] = { \
FOREGROUND_INTENSITY | FOREGROUND_RED, \
FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN, \
FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE, \
FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE, \
FOREGROUND_INTENSITY | FOREGROUND_GREEN, \
FOREGROUND_INTENSITY | FOREGROUND_GREEN, \
FOREGROUND_INTENSITY | FOREGROUND_GREEN,\
FOREGROUND_INTENSITY | FOREGROUND_GREEN};

#else
#include <sys/uio.h>
#include <unistd.h>
char const clr[8] = {'1', '3', '2', '4', '6', '6', '6'};
#endif

namespace framework
{
    namespace logger
    {
        void LoggerStdStream::write( 
            muti_buffer_t const * logmsgs, 
            size_t len )
        {
            if ( !len ) return;

            if ( m_log_clr_ )
            {
                int index = 5;
                if ( !strcmp( logmsgs[0].buffer, "[ERROR] [" ) ) index = 0;
                if ( !strcmp( logmsgs[0].buffer, "[ALARM] [" ) ) index = 1;
                if ( !strcmp( logmsgs[0].buffer, "[EVENT] [" ) ) index = 2;
                if ( !strcmp( logmsgs[0].buffer, "[INFOR] [" ) ) index = 3;
                if ( !strcmp( logmsgs[0].buffer, "[DEBUG] [" ) ) index = 4;
#ifdef BOOST_WINDOWS_API
                /// 打印有颜色的串
                SETCOLOR( clr[index] );
                for ( size_t iLoop = 0; iLoop < len; ++iLoop )
                {
                    fwrite( logmsgs[iLoop].buffer, logmsgs[iLoop].len, 1, stdout );
                }
                CLEARCOLOR();
#else
                printf( "\033[1;%xm", (int)clr[index] );
                ::writev( 1, ( iovec * )logmsgs, len );
                printf( "\033[0m" );
#endif
            } 
            else 
            {
#ifdef BOOST_WINDOWS_API
                for ( size_t iLoop = 0; iLoop < len; ++iLoop )
                {
                    fwrite( logmsgs[iLoop].buffer, logmsgs[iLoop].len, 1, stdout );
                }
#  else 
                ::writev( 1, ( iovec * )logmsgs, len );
#endif
            }
        }
    } // namespace logger
} // namespace framework
