// UdpStream.cpp

#include "framework/Framework.h"
#include "framework/logger/UdpStream.h"
#include "framework/network/Endpoint.h"
#include "framework/configure/Config.h"
using namespace framework::configure;

namespace framework
{
    namespace logger
    {

        UdpStream::UdpStream()
            : socket_(io_svc_)
        {
        }

        UdpStream::~UdpStream()
        {
            close();
        }

        void UdpStream::load_config(
            ConfigModule & cm)
        {
            cm << CONFIG_PARAM_NAME_RDWR("addr", addr_);
            open();
        }

        void UdpStream::write( 
            buffer_t const * bufs, 
            size_t len )
        {
            std::vector<boost::asio::const_buffer> vec;
            for (size_t i = 0; i < len; ++i) {
                vec.push_back(boost::asio::buffer(bufs[i].buf, bufs[i].len));
            }
            boost::system::error_code ec;
            socket_.send(vec, 0, ec);
        }

        bool UdpStream::open()
        {
            boost::system::error_code ec;
            socket_.connect(framework::network::Endpoint(addr_), ec);
            return !ec;
        }

        bool UdpStream::close()
        {
            boost::system::error_code ec;
            socket_.close(ec);
            return !ec;
        }

    } // namespace logger
} // namespace framework
