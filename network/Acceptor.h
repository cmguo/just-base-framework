// Acceptor.h

#ifndef _FRAMEWORK_NETWORK_ACCEPTOR_H_
#define _FRAMEWORK_NETWORK_ACCEPTOR_H_

#include <boost/asio/detail/throw_error.hpp>
#include <boost/asio/detail/bind_handler.hpp>
#include <boost/asio/error.hpp>

namespace framework
{
    namespace network
    {

        template <typename InternetProtocol>
        boost::system::error_code accept(
            typename InternetProtocol::acceptor & a,
            typename InternetProtocol::endpoint const & e,
            typename InternetProtocol::socket & s, // 外部创建的套接字，不需要open
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
                if (a.listen(1, ec))                    return ec;
            }
            while (a.accept(s, ec) == boost::asio::error::connection_aborted);
            return ec;
        }

        template <typename InternetProtocol>
        void accept(
            typename InternetProtocol::acceptor & a, 
            typename InternetProtocol::endpoint const & e, 
            typename InternetProtocol::socket & s) // 外部创建的套接字，不需要open
        {
            boost::system::error_code ec;
            accept<InternetProtocol>(a, e, s, ec);
            boost::asio::detail::throw_error(ec);
        }
/*
        template <typename InternetProtocol, typename MutableBuffer>
        boost::system::error_code accept(
            typename InternetProtocol::acceptor & a,
            typename InternetProtocol::endpoint const & e,
            typename InternetProtocol::socket & s, // 外部创建的套接字，不需要open
            MutableBuffer & b, // 接收缓冲区，必须由外部提供，socks可能会多接收数据
            boost::system::error_code & ec)
        {
            return accept<InternetProtocol>(a, e, s, ec);
        }

        template <typename InternetProtocol, typename MutableBuffer>
        void accept(
            typename InternetProtocol::acceptor & a, 
            typename InternetProtocol::endpoint const & e, 
            typename InternetProtocol::socket & s,  // 外部创建的套接字，不需要open
            MutableBuffer & b) // 接收缓冲区，必须由外部提供，socks可能会多接收数据
        {
            accept<InternetProtocol>(a, e, s);
        }
*/
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

                //private:
                acceptor & acceptor_;
                socket & socket_; // TCP套接字
                AcceptHandler handler_;
            };

            template <typename InternetProtocol, typename AcceptHandler>
            inline void* asio_handler_allocate(std::size_t size,
                accept_handler<InternetProtocol, AcceptHandler> * this_handler)
            {
                return boost_asio_handler_alloc_helpers::allocate(
                    size, &this_handler->handler_);
            }

            template <typename InternetProtocol, typename AcceptHandler>
            inline void asio_handler_deallocate(void* pointer, std::size_t size,
                accept_handler<InternetProtocol, AcceptHandler>* this_handler)
            {
                boost_asio_handler_alloc_helpers::deallocate(
                    pointer, size, &this_handler->handler_);
            }

            template <typename Function, typename InternetProtocol, typename AcceptHandler>
            inline void asio_handler_invoke(const Function& function,
                accept_handler<InternetProtocol, AcceptHandler>* this_handler)
            {
                boost_asio_handler_invoke_helpers::invoke(
                    function, &this_handler->handler_);
            }

        } // namespace detail

        template <typename InternetProtocol, typename AcceptHandler>
        void async_accept(
            typename InternetProtocol::acceptor & a,
            typename InternetProtocol::endpoint const & e,
            typename InternetProtocol::socket & s, // 外部创建的套接字，不需要open
            AcceptHandler const & handler)
        {
            if (!a.is_open()) {
                boost::system::error_code ec;
                if (a.open(e.protocol(), ec)) {
                    a.get_io_service().post(
                        boost::asio::detail::bind_handler(handler, ec));
                    return;
                }
                {
                    boost::system::error_code ec1;
                    boost::asio::socket_base::reuse_address cmd(true);
                    a.set_option(cmd, ec1);
                }
                if (a.bind(e, ec)) {
                    a.get_io_service().post(
                        boost::asio::detail::bind_handler(handler, ec));
                    return;
                }
                if (a.listen(1, ec)) {
                    a.get_io_service().post(
                        boost::asio::detail::bind_handler(handler, ec));
                    return;
                }
            }
            a.async_accept(s, 
                detail::accept_handler<InternetProtocol, AcceptHandler>(a, s, handler));
        }
/*
        template <typename InternetProtocol, typename MutableBuffer, typename AcceptHandler>
        void async_accept(
            typename InternetProtocol::acceptor & a,
            typename InternetProtocol::endpoint const & e,
            typename InternetProtocol::socket & s, // 外部创建的套接字，不需要open
            MutableBuffer & b, // 接收缓冲区，必须由外部提供，socks可能会多接收数据
            AcceptHandler const & handler)
        {
            async_accept(a, e, s, handler);
        }
*/
    } // namespace network
} // namespace framework

#endif // _FRAMEWORK_NETWORK_ACCEPTOR_H_
