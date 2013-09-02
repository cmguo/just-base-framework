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
            Stream::load_config(cm);

            cm << CONFIG_PARAM_NAME_RDWR("addr", addr_);
            open();
        }

        void UdpStream::write(
            buffers_t const & buffers)
        {
            boost::system::error_code ec;
            socket_.send(buffers, 0, ec);
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
