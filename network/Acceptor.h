// Acceptor.h

#ifndef _FRAMEWORK_NETWORK_ACCEPTOR_H_
#define _FRAMEWORK_NETWORK_ACCEPTOR_H_

#include "framework/network/NetName.h"

namespace framework
{
    namespace network
    {

        class Acceptor
        {
        public:
            Acceptor(
                boost::asio::io_service & io_svc);

            ~Acceptor();

        public:
            template <typename InternetProtocol>
            boost::system::error_code open(
                NetName const & n,
                boost::system::error_code & ec);

            template <typename InternetProtocol>
            void open(
                NetName const & n);

            template <typename InternetProtocol>
            boost::system::error_code open(
                typename InternetProtocol::endpoint const & e,
                boost::system::error_code & ec);

            template <typename InternetProtocol>
            void open(
                typename InternetProtocol::endpoint const & e);

            bool is_open();

            boost::system::error_code close(
                boost::system::error_code & ec);

            void close();

        public:
            boost::asio::io_service & get_io_service()
            {
                return io_svc_;
            }

            boost::uint16_t listen_port() const
            {
                return port_;
            }

        public:
            template <
                typename SocketType
            >
            boost::system::error_code accept(
                SocketType & s, // 外部创建的套接字，不需要open
                boost::system::error_code & ec);

            template <
                typename SocketType
            >
            void accept(
                SocketType & s);

            template <
                typename SocketType, 
                typename AcceptHandler
            >
            void async_accept(
                SocketType & s, // 外部创建的套接字，不需要open
                AcceptHandler const & handler);

        private:
            template <
                typename AcceptorType
            >
            void closer(
                boost::system::error_code & ec);

            template <
                typename AcceptorType
            >
            AcceptorType & get();

            template <
                typename AcceptorType
            >
            AcceptorType & as();

        private:
            typedef void (Acceptor::*closer_t)(
                boost::system::error_code & ec);

            char buf_[256];
            boost::asio::io_service & io_svc_;
            closer_t closer_;
            boost::uint16_t port_;
        };

    } // namespace network
} // namespace framework

#endif // _FRAMEWORK_NETWORK_ACCEPTOR_H_
