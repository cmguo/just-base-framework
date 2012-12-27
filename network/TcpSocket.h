// TcpSocket.h

#ifndef _FRAMEWORK_NETWORK_TCP_SOCKET_H_
#define _FRAMEWORK_NETWORK_TCP_SOCKET_H_

#include "framework/network/NetName.h"
#include "framework/network/Connector.h"
#include "framework/network/Acceptor.h"

#include <boost/asio/ip/tcp.hpp>

namespace framework
{
    namespace network
    {

        class TcpSocket
            : public boost::asio::ip::tcp::socket
        {
        public:
            typedef boost::asio::ip::tcp::socket super;

        public:
            TcpSocket(
                boost::asio::io_service & io_svc);

        public:
            boost::system::error_code set_non_block(
                bool non_block, 
                boost::system::error_code & ec);

            boost::system::error_code set_time_out(
                boost::uint32_t time_out, 
                boost::system::error_code & ec);

            bool get_non_block(
                boost::system::error_code & ec) const;

            boost::uint32_t get_time_out(
                boost::system::error_code & ec) const;

        public:
            // connect
            typedef Connector::Statistics Statistics;

            using super::connect;

            void connect(
                framework::network::NetName const & addr);

            boost::system::error_code connect(
                framework::network::NetName const & addr, 
                boost::system::error_code & ec);

            Statistics const & stat() const;

            using super::async_connect;

            template <typename Handler>
            void async_connect(
                framework::network::NetName const & addr, 
                Handler const & handler);

        public:
            // accept

            void accept(
                framework::network::NetName const & addr, 
                boost::asio::ip::tcp::acceptor & acceptor);

            boost::system::error_code accept(
                framework::network::NetName const & addr, 
                boost::asio::ip::tcp::acceptor & acceptor, 
                boost::system::error_code & ec);

            void accept(
                boost::asio::ip::tcp::endpoint const & ep, 
                boost::asio::ip::tcp::acceptor & acceptor);

            boost::system::error_code accept(
                boost::asio::ip::tcp::endpoint const & ep, 
                boost::asio::ip::tcp::acceptor & acceptor, 
                boost::system::error_code & ec);

            template <typename Handler>
            void async_accept(
                framework::network::NetName const & addr, 
                boost::asio::ip::tcp::acceptor & acceptor, 
                Handler const & handler);

            template <typename Handler>
            void async_accept(
                boost::asio::ip::tcp::endpoint const & ep, 
                boost::asio::ip::tcp::acceptor & acceptor, 
                Handler const & handler);

        public:
            void cancel();

            boost::system::error_code cancel(
                boost::system::error_code & ec);

            void cancel_forever();

            boost::system::error_code cancel_forever(
                boost::system::error_code & ec);

            void close();

            boost::system::error_code close(
                boost::system::error_code & ec);

        protected:
            static boost::asio::detail::mutex mutex_;

        private:
            Connector connector_;
        };

    } // namespace network
} // namespace framework

#endif // _FRAMEWORK_NETWORK_TCP_SOCKET_H_
