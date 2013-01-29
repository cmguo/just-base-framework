// Acceptor.hpp

#ifndef _FRAMEWORK_NETWORK_ACCEPTOR_HPP_
#define _FRAMEWORK_NETWORK_ACCEPTOR_HPP_

#include "framework/network/Acceptor.h"
#include "framework/network/AsioHandlerHelper.h"

#include <boost/asio/detail/throw_error.hpp>
#include <boost/asio/detail/bind_handler.hpp>
#include <boost/asio/error.hpp>

namespace framework
{
    namespace network
    {

        template <typename InternetProtocol>
        boost::system::error_code Acceptor::open(
            NetName const & n,
            boost::system::error_code & ec)
        {
            using namespace boost::asio::ip;
            typename InternetProtocol::endpoint ep(address::from_string(n.host()), n.port());
            open<InternetProtocol>((ep, ec));
            if (n.svc().find('+') ) {
                size_t ntry = 20;
                while (ec == boost::asio::error::address_in_use && ntry) {
                    (this->*closer_)(ec);
                    ep.port(ep.port() + 1);
                    --ntry;
                    open<InternetProtocol>((ep, ec));
                }
            }
            return ec;
        }

        template <typename InternetProtocol>
        void Acceptor::open(
            NetName const & n)
        {
            boost::system::error_code ec;
            open<InternetProtocol>(n, ec);
            boost::asio::detail::throw_error(ec);
        }

        template <typename InternetProtocol>
        boost::system::error_code Acceptor::open(
            typename InternetProtocol::endpoint const & e,
            boost::system::error_code & ec)
        {
            assert(closer_ == NULL);
            assert(sizeof(typename InternetProtocol::acceptor) <= sizeof(buf_));
            typedef typename InternetProtocol::acceptor acceptor;
            acceptor & a(* new (buf_) acceptor(io_svc_));
            if (a.open(e.protocol(), ec)) {
                return ec;
            }
            {
                boost::system::error_code ec1;
                boost::asio::socket_base::reuse_address cmd(true);
                a.set_option(cmd, ec1);
            }
            if (a.bind(e, ec))
                return ec;
            if (a.listen(1, ec))
                return ec;
            ec.clear();
            closer_ = &closer<InternetProtocol>;
            return ec;
        }

        template <typename InternetProtocol>
        void Acceptor::open(
            typename InternetProtocol::endpoint const & e)
        {
            boost::system::error_code ec;
            open<InternetProtocol>(e, ec);
            boost::asio::detail::throw_error(ec);
        }

        template <typename InternetProtocol>
        boost::system::error_code Acceptor::accept(
            typename InternetProtocol::socket & s, // �ⲿ�������׽��֣�����Ҫopen
            boost::system::error_code & ec)
        {
            typename InternetProtocol::acceptor & a(as<InternetProtocol>());
            while (a.accept(s, ec) == boost::asio::error::connection_aborted);
            return ec;
        }

        template <typename InternetProtocol>
        void Acceptor::accept(
            typename InternetProtocol::socket & s) // �ⲿ�������׽��֣�����Ҫopen
        {
            boost::system::error_code ec;
            accept<InternetProtocol>(s, ec);
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
        void Acceptor::async_accept(
            typename InternetProtocol::socket & s, // �ⲿ�������׽��֣�����Ҫopen
            AcceptHandler const & handler)
        {
            typename InternetProtocol::acceptor & a(as<InternetProtocol>());
            a.async_accept(s, 
                detail::accept_handler<InternetProtocol, AcceptHandler>(a, s, handler));
        }

        template <typename InternetProtocol>
        void Acceptor::closer(
            boost::system::error_code & ec)
        {
            typedef typename InternetProtocol::acceptor acceptor;
            acceptor & a(as<InternetProtocol>());
            a.close(ec);
            (&a)->~acceptor();
            closer_ = NULL;
        }

        template <typename InternetProtocol>
        typename InternetProtocol::acceptor & Acceptor::as()
        {
            assert(closer_ == (closer_t)&Acceptor::closer<InternetProtocol>);
            return *(typename InternetProtocol::acceptor *)buf_;
        }

    } // namespace network
} // namespace framework

#endif // _FRAMEWORK_NETWORK_ACCEPTOR_H_
