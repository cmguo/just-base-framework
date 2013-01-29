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
            Acceptor & acceptor, 
            Handler const & handler)
        {
            acceptor.async_accept(*this, handler);
        }

    } // namespace network
} // namespace framework

#endif // _FRAMEWORK_NETWORK_TCP_SOCKET_HPP_
