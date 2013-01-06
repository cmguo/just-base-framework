// Acceptor.h

#ifndef _FRAMEWORK_NETWORK_ACCEPTOR_HPP_
#define _FRAMEWORK_NETWORK_ACCEPTOR_HPP_

#include "framework/network/AsioHandlerHelper.h"

#include <boost/asio/detail/throw_error.hpp>
#include <boost/asio/detail/bind_handler.hpp>
#include <boost/asio/error.hpp>

namespace framework
{
    namespace network
    {

        template <typename InternetProtocol>
        boost::system::error_code acceptor_open(
            typename InternetProtocol::acceptor & a,
            typename InternetProtocol::endpoint const & e,
            boost::system::error_code & ec)
        {
            if (!a.is_open()) {
                if (a.open(e.protocol(), ec))
                    return ec;
                {
                    boost::system::error_code ec1;
                    boost::asio::socket_base::reuse_address cmd(true);
                    a.set_option(cmd, ec1);
                }
                if (a.bind(e, ec))
                    return ec;
                if (a.listen(1, ec))
                    return ec;
            }
            ec.clear();
            return ec;
        }

        template <typename InternetProtocol>
        boost::system::error_code accept(
            typename InternetProtocol::acceptor & a,
            typename InternetProtocol::endpoint const & e,
            typename InternetProtocol::socket & s, // �ⲿ�������׽��֣�����Ҫopen
            boost::system::error_code & ec)
        {
            if (acceptor_open<InternetProtocol>(a, e, ec)) {
                return ec;
            }
            while (a.accept(s, ec) == boost::asio::error::connection_aborted);
            return ec;
        }

        template <typename InternetProtocol>
        void accept(
            typename InternetProtocol::acceptor & a, 
            typename InternetProtocol::endpoint const & e,
            typename InternetProtocol::socket & s) // �ⲿ�������׽��֣�����Ҫopen
        {
            boost::system::error_code ec;
            accept<InternetProtocol>(a, e, s, ec);
            boost::asio::detail::throw_error(ec);
        }

        namespace detail
        {

            template <typename InternetProtocol, typename AcceptHandler>
            class accept_handler
            {
            public:
                /// The network type.
                typedef InternetProtocol protocol_type;

                /// The acceptor type.
                typedef typename InternetProtocol::acceptor acceptor;

                /// The socket type.
                typedef typename InternetProtocol::socket socket;

                accept_handler(
                    acceptor & a, 
                    socket & s, 
                    AcceptHandler handler)
                    : acceptor_(a)
                    , socket_(s)
                    , handler_(handler)
                {
                }

            public:
                void operator ()(
                    boost::system::error_code const & ec)
                {
                    if (ec == boost::asio::error::connection_aborted) {
                        acceptor_.async_accept(socket_, *this);
                        return;
                    }
                    handler_(ec);
                }

                PASS_DOWN_ASIO_HANDLER_FUNCTION(accept_handler, handler_)

            private:
                acceptor & acceptor_;
                socket & socket_; // TCP�׽���
                AcceptHandler handler_;
            };

        } // namespace detail

        template <typename InternetProtocol, typename AcceptHandler>
        void async_accept(
            typename InternetProtocol::acceptor & a,
            typename InternetProtocol::endpoint const & e,
            typename InternetProtocol::socket & s, // �ⲿ�������׽��֣�����Ҫopen
            AcceptHandler const & handler)
        {
            if (!a.is_open()) {
                boost::system::error_code ec;
                if (acceptor_open<InternetProtocol>(a, e, ec)) {
                    a.get_io_service().post(
                        boost::asio::detail::bind_handler(handler, ec));
                    return;
                }
            }
            a.async_accept(s, 
                detail::accept_handler<InternetProtocol, AcceptHandler>(a, s, handler));
        }

    } // namespace network
} // namespace framework

#endif // _FRAMEWORK_NETWORK_ACCEPTOR_H_