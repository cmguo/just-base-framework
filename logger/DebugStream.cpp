// DebugStream.cpp

#include "framework/Framework.h"
#include "framework/logger/DebugStream.h"
#include "framework/configure/Config.h"
using namespace framework::configure;

#ifdef BOOST_WINDOWS_API
#  include <windows.h>
#endif

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
            buffers_t const & buffers)
        {
#ifdef BOOST_WINDOWS_API
            for (size_t iLoop = 0; iLoop < buffers.size(); ++iLoop) {
                ::OutputDebugStringA(
                    boost::asio::buffer_cast<LPCSTR>(buffers[iLoop]));
            }

#else
#endif
        }

    } // namespace logger
} // namespace framework
