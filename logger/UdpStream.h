// UdpStream.h

#ifndef _FRAMEWORK_LOGGER_UDP_STREAM_H_
#define _FRAMEWORK_LOGGER_UDP_STREAM_H_

#include "framework/logger/Stream.h"

#include <boost/asio/ip/udp.hpp>

namespace framework
{
    namespace logger
    {
        /// Ð´ÈëÎÄ¼þ
        class UdpStream
            : public Stream
        {
        public:
            UdpStream();

            virtual ~UdpStream();

        private:
            virtual void write( 
                buffer_t const * bufs, 
                size_t len);

            virtual void load_config(
                framework::configure::ConfigModule & cm);

        private:
            bool open();

            bool close();

        private:
            boost::asio::io_service io_svc_;
            boost::asio::ip::udp::socket socket_;
            std::string addr_;
        };

        LOG_REG_STREAM_TYPE(udp, UdpStream)

    }// namespace logger
}// namespace framework

#endif // _FRAMEWORK_LOGGER_UDP_STREAM_H_
