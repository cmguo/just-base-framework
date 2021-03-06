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

        template <
            typename InternetProtocol
        >
        boost::system::error_code Acceptor::open(
            NetName const & n,
            boost::system::error_code & ec)
        {
            using namespace boost::asio::ip;
            typename InternetProtocol::endpoint ep(address::from_string(n.host()), n.port());
            open<InternetProtocol>(ep, ec);
            if (n.svc().find('+') != std::string::npos) {
                size_t ntry = 20;
                while (ec == boost::asio::error::address_in_use && ntry) {
                    ep.port(ep.port() + 1);
                    --ntry;
                    open<InternetProtocol>(ep, ec);
                }
            }
            return ec;
        }

        template <
            typename InternetProtocol
        >
        void Acceptor::open(
            NetName const & n)
        {
            boost::system::error_code ec;
            open<InternetProtocol>(n, ec);
            boost::asio::detail::throw_error(ec);
        }

        struct close_helper
        {
            close_helper(
                Acceptor & acceptor)
                : acceptor_(&acceptor)
            {
            }

            ~close_helper()
            {
                if (acceptor_) {
                    boost::system::error_code ec;
                    acceptor_->close(ec);
                }
            }

            void release()
            {
                acceptor_ = NULL;
            }

            Acceptor * acceptor_;
        };

        template <
            typename InternetProtocol
        >
        boost::system::error_code Acceptor::open(
            typename InternetProtocol::endpoint const & e,
            boost::system::error_code & ec)
        {
            assert(closer_ == NULL);
            assert(sizeof(typename InternetProtocol::acceptor) <= sizeof(buf_));
            typedef typename InternetProtocol::acceptor acceptor;
            acceptor & a(get<acceptor>());
            close_helper ch(*this);
            if (a.open(e.protocol(), ec)) {
                return ec;
            }
#ifndef BOOST_WINDOWS_API
            {
                boost::system::error_code ec1;
                boost::asio::socket_base::reuse_address cmd(true);
                a.set_option(cmd, ec1);
            }
#endif
            if (a.bind(e, ec))
                return ec;
            if (a.listen(boost::asio::socket_base::max_connections, ec))
                return ec;
            port_ = e.port();
            ec.clear();
            ch.release();
            return ec;
        }

        template <
            typename InternetProtocol
        >
        void Acceptor::open(
            typename InternetProtocol::endpoint const & e)
        {
            boost::system::error_code ec;
            open<InternetProtocol>(e, ec);
            boost::asio::detail::throw_error(ec);
        }

        template <
            typename SocketType
        >
        boost::system::error_code Acceptor::accept(
            SocketType & s, // 外部创建的套接字，不需要open
            boost::system::error_code & ec)
        {
            typedef typename SocketType::protocol_type::acceptor acceptor;
            acceptor & a(as<acceptor>());
            while (a.accept(s, ec) == boost::asio::error::connection_aborted);
            return ec;
        }

        template <
            typename SocketType
        >
        void Acceptor::accept(
            SocketType & s) // 外部创建的套接字，不需要open
        {
            boost::system::error_code ec;
            accept(s, ec);
            boost::asio::detail::throw_error(ec);
        }

        namespace detail
        {

            template <
                typename SocketType, 
                typename AcceptHandler
            >
            class accept_handler
            {
            public:
                /// The network type.
                typedef typename SocketType::protocol_type protocol_type;

                /// The acceptor type.
                typedef typename protocol_type::acceptor acceptor;

                /// The socket type.
                typedef typename protocol_type::socket socket;

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
                socket & socket_; // TCP套接字
                AcceptHandler handler_;
            };

        } // namespace detail

        template <
            typename SocketType, 
            typename AcceptHandler
        >
        void Acceptor::async_accept(
            SocketType & s, // 外部创建的套接字，不需要open
            AcceptHandler const & handler)
        {
            typedef typename SocketType::protocol_type::acceptor acceptor;
            acceptor & a(as<acceptor>());
            a.async_accept(s, 
                detail::accept_handler<SocketType, AcceptHandler>(a, s, handler));
        }

        template <
            typename AcceptorType
        >
        void Acceptor::closer(
            boost::system::error_code & ec)
        {
            port_ = 0;
            AcceptorType & a(as<AcceptorType>());
            a.close(ec);
            (&a)->~AcceptorType();
            closer_ = NULL;
        }

        template <
            typename AcceptorType
        >
        AcceptorType & Acceptor::get()
        {
            assert(!is_open());
            AcceptorType & a(* new (buf_) AcceptorType(io_svc_));
            closer_ = &Acceptor::closer<AcceptorType>;
            return a;
        }

        template <
            typename AcceptorType
        >
        AcceptorType & Acceptor::as()
        {
            // ios: internal compiler error
            // assert(closer_ == &Acceptor::closer<AcceptorType>);
            closer_t closer = &Acceptor::closer<AcceptorType>;
            assert(closer_ == closer);
            (void)closer;
            union {
                char * n;
                AcceptorType * t;
            } u = {buf_};
            return *u.t;
        }

    } // namespace network
} // namespace framework

#endif // _FRAMEWORK_NETWORK_ACCEPTOR_H_
