// TcpSocket.hpp

#ifndef _FRAMEWORK_NETWORK_TCP_SOCKET_HPP_
#define _FRAMEWORK_NETWORK_TCP_SOCKET_HPP_

#include "framework/network/TcpSocket.h"
#include "framework/network/Connector.hpp"
#include "framework/network/Acceptor.hpp"

namespace framework
{
    namespace network
    {

        template <typename Handler>
        void TcpSocket::async_connect(
            framework::network::NetName const & addr, 
            Handler const & handler)
        {
            connector_.async_connect(*this, 
                framework::network::NetName(framework::network::NetName::tcp, addr), handler);
        }

        template <typename Handler>
        void TcpSocket::async_accept(
            framework::network::NetName const & addr, 
            boost::asio::ip::tcp::acceptor & acceptor, 
            Handler const & handler)
        {
            try {
                boost::asio::ip::address address = boost::asio::ip::address::from_string(addr.host());
                boost::asio::ip::tcp::endpoint ep(address, addr.port());
                framework::network::async_accept<boost::asio::ip::tcp>(acceptor, ep, *this, handler);
            } catch (boost::system::system_error const & err) {
                get_io_service().post(boost::asio::detail::bind_handler(handler, err.code()));
            }
        }

        template <typename Handler>
        void TcpSocket::async_accept(
            boost::asio::ip::tcp::endpoint const & ep, 
            boost::asio::ip::tcp::acceptor & acceptor, 
            Handler const & handler)
        {
            framework::network::async_accept<boost::asio::ip::tcp>(acceptor, ep, *this, handler);
        }

} // namespace network
} // namespace framework

#endif // _FRAMEWORK_NETWORK_TCP_SOCKET_HPP_
