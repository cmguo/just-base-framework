// DebugStream.h

#ifndef _FRAMEWORK_LOGGER_DEBUG_STREAM_H_
#define _FRAMEWORK_LOGGER_DEBUG_STREAM_H_

#include "framework/logger/Stream.h"

namespace framework
{
    namespace logger
    {

        /// –¥»ÎOutputDebugString
        class DebugStream
            : public Stream
        {
        public:
            DebugStream();

            virtual ~DebugStream();

        private:
            virtual void write( 
                buffer_t const * bufs, 
                size_t len);

            virtual void load_config(
                framework::configure::ConfigModule & cm);
        };

        LOG_REG_STREAM_TYPE(debug, DebugStream)

    }// namespace logger
}// namespace framework

#endif // _FRAMEWORK_LOGGER_DEBUG_STREAM_H_
