// Connector.hpp

#ifndef _FRAMEWORK_NETWORK_CONNECTOR_HPP_
#define _FRAMEWORK_NETWORK_CONNECTOR_HPP_

#include "framework/logger/Logger.h"
#include "framework/logger/StreamRecord.h"
#include "framework/logger/Section.h"
#include "framework/network/AsioHandlerHelper.h"
#include "framework/network/TimedHandler.hpp"

#include <boost/asio/detail/throw_error.hpp>
#include <boost/asio/detail/socket_ops.hpp>

namespace framework
{
    namespace network
    {

        template <typename SocketType>
        void Connector::start_connect(
            SocketType & peer, 
            Endpoint const & ep, 
            boost::system::error_code & ec)
        {
            if (!non_block_ && time_out_) {
                boost::asio::socket_base::non_blocking_io cmd(true);
                peer.io_control(cmd, ec);
                if (ec)
                    return;
            }
            if (!connect_started_) {
                connect_started_ = true;
                peer.connect(ep, ec);
            }
            if ((ec == boost::asio::error::in_progress || 
                ec == boost::asio::error::already_started)) {
                    ec = boost::asio::error::would_block;
            }
        }

        template <typename SocketType>
        void Connector::pool_connect(
            SocketType & peer, 
            boost::system::error_code & ec)
        {
            fd_set writefds;
            fd_set exceptfds;
            timeval timeout;
            FD_ZERO(&writefds);
            FD_ZERO(&exceptfds);
            typename SocketType::native_type fd = peer.native();
            FD_SET(fd, &writefds);
            FD_SET(fd, &exceptfds);
            timeval * ptimeout = &timeout;
            if (non_block_) {
                timeout.tv_sec = 0;
                timeout.tv_usec = 0;
            } else if (time_out_) {
                timeout.tv_sec = time_out_ / 1000;
                timeout.tv_usec = (time_out_ % 1000) * 1000;
            } else {
                ptimeout = NULL;
            }
            int ret = boost::asio::detail::socket_ops::select(
                fd + 1, NULL, &writefds, &exceptfds, ptimeout, ec);
            if (ret > 0) {
                boost::asio::socket_base::error err;
                peer.get_option(err);
                ec = boost::asio::error::basic_errors(err.value());
            } else if (ret == 0) {
                if (non_block_) {
                    ec = boost::asio::error::would_block;
                } else {
                    ec = boost::asio::error::timed_out;
                }
            }
        }

        template <typename SocketType>
        void Connector::post_connect(
            SocketType & peer, 
            boost::system::error_code & ec)
        {
            if (!non_block_ && time_out_) {
                boost::asio::socket_base::non_blocking_io cmd(false);
                boost::system::error_code ec1;
                peer.io_control(cmd, ec1);
            }
            // support non-blocking mode
            if (ec == boost::asio::error::in_progress || 
                ec == boost::asio::error::already_started || 
                ec == boost::asio::error::would_block) {
                    ec = boost::asio::error::would_block;
            } else if (ec == boost::asio::error::already_connected) {
                ec = boost::system::error_code();
            } else {
                connect_started_ = false;
            }
        }

        template <typename SocketType>
        boost::system::error_code Connector::connect(
            SocketType & peer1, 
            Endpoint const & endpoint, 
            boost::system::error_code & ec)
        {
            typedef typename SocketType::protocol_type::socket socket;
            typedef typename socket::endpoint_type endpoint_type;

            socket & peer(peer1); // we should use the real socket type, not the child type

            if (!started_) {
                canceled_ = canceled_forever_;
                stat_.reset();
                stat_.resolve_time = 0;
                if (ec) {
                    return ec;
                }
                {
                    boost::asio::detail::mutex::scoped_lock lock(mutex_);
                    if (canceled_) {
                        ec = boost::asio::error::operation_aborted;
                        canceled_ = false;
                    } else if (peer.is_open()) {
                        peer.close(ec);
                    }
                    boost::asio::socket_base::non_blocking_io cmd1(non_block_);
#ifndef UNDER_CE
                    boost::asio::socket_base::receive_time_out cmd2(time_out_);
#endif
                    ec || peer.open(endpoint_type(endpoint).protocol(), ec) 
                        || peer.io_control(cmd1, ec)
#ifndef UNDER_CE
                        || peer.set_option(cmd2, ec)
#endif
                        ;
                    started_ = true;
                }
                connect_started_ = false;
                if (ec)
                    return ec;
                start_connect(peer, endpoint, ec);
            } else {
                ec = boost::asio::error::would_block;
            }
            if (ec == boost::asio::error::would_block) {
                pool_connect(peer, ec);
            }
            if (ec == boost::asio::error::would_block) {
                if (time_out_ && stat_.elapse() > time_out_) {
                    ec = boost::asio::error::timed_out;
                }
            }
            if (ec != boost::asio::error::would_block) {
                if (!ec) {
                    post_connect(peer, ec);
                } else {
                    boost::system::error_code ec1;
                    post_connect(peer, ec1);
                }
                stat_.connect_time = stat_.elapse();
                started_ = false;
                canceled_ = false;
            } 
            return ec;
        }

        template <typename SocketType>
        boost::system::error_code Connector::connect(
            SocketType & peer1, 
            NetName const & netname, 
            boost::system::error_code & ec)
        {
            typedef typename SocketType::protocol_type::socket socket;
            typedef typename socket::endpoint_type endpoint_type;

            socket & peer(peer1); // we should use the real socket type, not the child type

            if (netname.is_digit()) {
                return connect(peer, netname.endpoint(), ec);
            }
            if (!started_) {
                canceled_ = canceled_forever_;
                stat_.reset();
                connect_started_ = false;
                boost::asio::detail::mutex::scoped_lock lock(mutex_);
                if (canceled_) {
                    ec = boost::asio::error::operation_aborted;
                    canceled_ = false;
                } else {
                    lock.unlock();
                    resolver_iterator_ = resolver_.resolve(netname, ec);
                    lock.lock();
                }
                stat_.resolve_time = stat_.elapse();
                if (ec) {
                    return ec;
                } else if (canceled_) {
                    canceled_ = false;
                    return ec = boost::asio::error::operation_aborted;
                }
                started_ = true;
            }
            ResolverIterator end;
            for (; resolver_iterator_ != end; ++resolver_iterator_) {
                if (!connect_started_) {
                    Endpoint const & e = *resolver_iterator_;
                    {
                        boost::asio::detail::mutex::scoped_lock lock(mutex_);
                        if (canceled_) {
                            ec = boost::asio::error::operation_aborted;
                            canceled_ = false;
                        } else {
                            if (peer.is_open()) {
                                peer.close(ec);
                            }
                            boost::asio::socket_base::non_blocking_io cmd1(non_block_);
#ifndef UNDER_CE
                            boost::asio::socket_base::receive_time_out cmd2(time_out_);
#endif
                            ec || peer.open(endpoint_type(e).protocol(), ec) 
                                || peer.io_control(cmd1, ec) 
#ifndef UNDER_CE
                                || peer.set_option(cmd2, ec)
#endif
                                ;
                        }
                    }
                    if (ec) {
                        break;
                    }
                    LOG_TRACE("[connect] try server, ep: " << e.to_string());
                    start_connect(peer, e, ec);
                } else {
                    ec = boost::asio::error::would_block;
                }
                if (ec == boost::asio::error::would_block) {
                    pool_connect(peer, ec);
                }
                if (ec != boost::asio::error::would_block) {
                    if (!ec) {
                        post_connect(peer, ec);
                    } else {
                        boost::system::error_code ec1;
                        post_connect(peer, ec1);
                    }
                }
                if (!ec || ec == boost::asio::error::would_block || canceled_) {
                    break;
                }
                LOG_DEBUG("[connect] failed, ep: " << 
                    resolver_iterator_->to_string() << ",ec: " << ec.message());
            } // for
            if ((!ec || ec == boost::asio::error::would_block) && canceled_) {
                ec = boost::asio::error::operation_aborted;
            }
            if (ec != boost::asio::error::would_block) {
                stat_.connect_time = stat_.elapse();
                started_ = false;
                canceled_ = false;
            }
            return ec;
        }

        template <typename SocketType>
        void Connector::connect(
            SocketType & peer, 
            Endpoint const & endpoint)
        {
            boost::system::error_code ec;
            connect(peer, endpoint, ec);
            boost::asio::detail::throw_error(ec);
        }

        template <typename SocketType>
        void Connector::connect(
            SocketType & peer, 
            NetName const & netname)
        {
            boost::system::error_code ec;
            connect(peer, netname, ec);
            boost::asio::detail::throw_error(ec);
        }

        namespace detail
        {

            template <
                typename SocketType, 
                typename ConnectHandler
            >
            class connect_handler
            {
                FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("framework.network.Connector", framework::logger::Warn)
            public:
                typedef void result_type;

                connect_handler(
                    SocketType & s, 
                    bool non_block, 
                    boost::asio::detail::mutex & mutex, 
                    Resolver & rsv, 
                    bool & canceled, 
                    Connector::Statistics & stat, 
                    ConnectHandler handler,
                    boost::uint32_t time_out, 
                    TimedHandler & timed_handler)
                    : socket_(s)
                    , non_block_(non_block)
                    , mutex_(mutex)
                    , canceled_(canceled)
                    , resolver_(rsv)
                    , stat_(stat)
                    , handler_(handler)
                    , time_out_(time_out)
                    , timed_handler_(timed_handler)
                {
                }

                struct ref
                {
                    ref(connect_handler & r) : ref_(r) {}
                    connect_handler & ref_;
                    
                    void operator ()( // handle_resolve
                        boost::system::error_code const & ec, 
                        ResolverIterator iter)
                    {
                        ref_(ec, iter);
                    }
                    void operator ()( // handle connect
                        boost::system::error_code const & ec)
                    {
                        ref_(ec);
                    }
                };

                void start(
                    Endpoint const & e)
                {
                    typedef typename SocketType::endpoint_type endpoint_type;

                    boost::system::error_code ec;
                    {
                        boost::asio::detail::mutex::scoped_lock lock(mutex_);
                        if (canceled_) {
                            ec = boost::asio::error::operation_aborted;
                        } else {
                            boost::asio::socket_base::non_blocking_io cmd(non_block_);
                            socket_.open(endpoint_type(e).protocol(), ec)
                                || socket_.io_control(cmd, ec);
                        }
                    }
                    if (ec) {
                        stat_.connect_time = stat_.elapse();
                        canceled_ = false;
                        socket_.get_io_service().post(
                            boost::asio::detail::bind_handler(handler_, ec));
                        delete this;
                    } else {
                        if (time_out_ != 0) {
                            socket_.async_connect(e, timed_handler_.wrap(
                                time_out_, 
                                ref(*this), 
                                boost::bind(&connect_handler::cancel, this)));
                        } else {
                            socket_.async_connect(e, 
                                ref(*this));
                        }
                    }
                }

                void start(
                    NetName const & netname)
                {
                    boost::system::error_code ec;
                    boost::asio::detail::mutex::scoped_lock lock(mutex_);
                    if (canceled_) {
                        ec = boost::asio::error::operation_aborted;
                    } else {
                        // 在加锁的情况下启动
                        resolver_.async_resolve(netname, 
                            ref(*this));
                        return;
                    }
                    canceled_ = false;
                    socket_.get_io_service().post(
                        boost::asio::detail::bind_handler(handler_, ec));
                    delete this;
                }

                void cancel()
                {
                    boost::system::error_code ec;
                    socket_.cancel(ec);
                }

            public:
                void operator ()( // handle_resolve
                    boost::system::error_code const & ec, 
                    ResolverIterator iter)
                {
                    typedef typename SocketType::endpoint_type endpoint_type;

                    stat_.resolve_time = stat_.elapse();

                    boost::system::error_code ec1 = ec;

                    iterator_ = iter;
                    ResolverIterator end;
                    if (!ec1 && iterator_ != end) {
                        Endpoint e = *iterator_;
                        {
                            boost::asio::detail::mutex::scoped_lock lock(mutex_);
                            if (canceled_) {
                                ec1 = boost::asio::error::operation_aborted;
                            } else {
                                boost::asio::socket_base::non_blocking_io cmd(non_block_);
                                socket_.close(ec1);
                                socket_.open(endpoint_type(e).protocol(), ec1) 
                                    || socket_.io_control(cmd, ec1);
                            }
                        }
                        if (!ec1) {
                            LOG_DEBUG("[async_connect] try server, ep: " << e.to_string());
                            if (time_out_ != 0) {
                                socket_.async_connect(e, timed_handler_.wrap(
                                    time_out_, 
                                    ref(*this), 
                                    boost::bind(&connect_handler::cancel, this)));
                            } else {
                                socket_.async_connect(e, 
                                    ref(*this));
                            }
                            return;
                        }
                        stat_.connect_time = stat_.elapse();
                    }
                    LOG_DEBUG("[async_connect] finish, ep: " 
                        << iterator_->to_string() << ", ec: " << ec1.message());
                    canceled_ = false;
                    handler_(ec1);
                    delete this;
                }

                void operator ()( // handle connect
                    boost::system::error_code const & ec)
                {
                    typedef typename SocketType::endpoint_type endpoint_type;

                    LOG_SECTION();

                    boost::system::error_code ec1 = ec;
                    if (ec1) {
                        LOG_DEBUG("[async_connect] failed, ep: " 
                            << iterator_->to_string() << ", ec: " << ec1.message());
                        ResolverIterator end;
                        /* 通过ip连接，iterator_一开始就等于end */
                        if (iterator_ != end && ++iterator_ != end) {
                            ec1.clear();
                            Endpoint e = *iterator_;
                            {
                                boost::asio::detail::mutex::scoped_lock lock(mutex_);
                                if (canceled_) {
                                    ec1 = boost::asio::error::operation_aborted;
                                } else {
                                    boost::asio::socket_base::non_blocking_io cmd(non_block_);
                                    socket_.close(ec1);
                                    socket_.open(endpoint_type(e).protocol(), ec1) 
                                        || socket_.io_control(cmd, ec1);
                                }
                            }
                            if (!ec1) {
                                LOG_SECTION();
                                LOG_DEBUG("[async_connect] try server, ep: " << e.to_string());
                                if (time_out_ != 0) {
                                    socket_.async_connect(e, timed_handler_.wrap(
                                        time_out_, 
                                        ref(*this), 
                                        boost::bind(&connect_handler::cancel, this)));
                                } else {
                                    socket_.async_connect(e, 
                                        ref(*this));
                                }
                                return;
                            }
                        }
                    }
                    LOG_DEBUG("[async_connect] finish, ep: " 
                        << iterator_->to_string() << ", ec: " << ec1.message());
                    stat_.connect_time = stat_.elapse();
                    canceled_ = false;
                    handler_(ec1);
                    delete this;
                }

                PASS_DOWN_ASIO_HANDLER_FUNCTION(connect_handler, handler_)

            private:
                SocketType & socket_; // TCP套接字
                bool non_block_;
                boost::asio::detail::mutex & mutex_;
                bool & canceled_;
                Resolver & resolver_; // DNS解析器
                ResolverIterator iterator_;
                Connector::Statistics & stat_;
                ConnectHandler handler_;
                boost::uint32_t time_out_;
                TimedHandler & timed_handler_;
            };

        } // namespace detail

        template <
            typename SocketType, 
            typename ConnectHandler
        >
        void Connector::async_connect(
            SocketType & peer, // 外部创建的套接字，不需要open
            NetName const & netname, 
            ConnectHandler const & handler)
        {
            typedef typename SocketType::protocol_type::socket socket;
            typedef detail::connect_handler<socket, ConnectHandler> connect_handler_t;

            canceled_ = canceled_forever_;
            if (netname.is_digit()) {
                stat_.reset();
                stat_.resolve_time = 0;
                connect_handler_t * handler2 = 
                    new connect_handler_t(peer, non_block_, mutex_, resolver_, canceled_, stat_, handler, time_out_, timed_handler_);
                handler2->start(netname.endpoint());
            } else {
                stat_.reset();
                connect_handler_t * handler2 = 
                    new connect_handler_t(peer, non_block_, mutex_, resolver_, canceled_, stat_, handler, time_out_, timed_handler_);
                handler2->start(netname);
            }
        }

    } // namespace network
} // namespace framework

#endif // _FRAMEWORK_NETWORK_CONNECTOR_HPP_
