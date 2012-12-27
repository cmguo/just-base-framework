// Acceptor.h

#ifndef _FRAMEWORK_NETWORK_ACCEPTOR_H_
#define _FRAMEWORK_NETWORK_ACCEPTOR_H_

namespace framework
{
    namespace network
    {

        template <typename InternetProtocol>
        boost::system::error_code acceptor_open(
            typename InternetProtocol::acceptor & a,
            typename InternetProtocol::endpoint const & e,
            boost::system::error_code & ec);

        template <typename InternetProtocol>
        boost::system::error_code accept(
            typename InternetProtocol::acceptor & a,
            typename InternetProtocol::endpoint const & e,
            typename InternetProtocol::socket & s, // 外部创建的套接字，不需要open
            boost::system::error_code & ec);

        template <typename InternetProtocol>
        void accept(
            typename InternetProtocol::acceptor & a, 
            typename InternetProtocol::endpoint const & e,
            typename InternetProtocol::socket & s);

        template <typename InternetProtocol, typename AcceptHandler>
        void async_accept(
            typename InternetProtocol::acceptor & a,
            typename InternetProtocol::endpoint const & e,
            typename InternetProtocol::socket & s, // 外部创建的套接字，不需要open
            AcceptHandler const & handler);

    } // namespace network
} // namespace framework

#endif // _FRAMEWORK_NETWORK_ACCEPTOR_H_
