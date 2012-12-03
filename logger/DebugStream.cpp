// DebugStream.cpp

#include "framework/Framework.h"
#include "framework/logger/DebugStream.h"
#include "framework/configure/Config.h"
using namespace framework::configure;

namespace framework
{
    namespace logger
    {

        DebugStream::DebugStream()
        {
        }

        DebugStream::~DebugStream()
        {
        }

        void DebugStream::load_config(
            ConfigModule & cm)
        {
            Stream::load_config(cm);
        }

        void DebugStream::write( 
            buffer_t const * bufs, 
            size_t len )
        {
#ifdef BOOST_WINDOWS_API
            DWORD dw = 0;
            for ( size_t iLoop = 0; iLoop < len; ++iLoop ) {
                ::OutputDebugStringA(
                    bufs[iLoop].buf);
            }

#else
#endif
        }

    } // namespace logger
} // namespace framework
