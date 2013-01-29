// TcpSocket.cpp

#include "framework/Framework.h"
#include "framework/network/TcpSocket.h"
#include "framework/network/TcpSocket.hpp"

namespace framework
{
    namespace network
    {

        boost::asio::detail::mutex TcpSocket::mutex_;

        TcpSocket::TcpSocket(
            boost::asio::io_service & io_svc)
            : super(io_svc)
            , connector_(io_svc, mutex_)
        {
        }

        boost::system::error_code TcpSocket::set_non_block(
            bool non_block, 
            boost::system::error_code & ec)
        {
            ec = boost::system::error_code();
            boost::asio::socket_base::non_blocking_io cmd(non_block);
            {
                boost::asio::detail::mutex::scoped_lock lock(mutex_);
                if (is_open() && io_control(cmd, ec)) {
                    return ec;
                }
            }
            connector_.set_non_block(non_block);
            return ec;
        }

        boost::system::error_code TcpSocket::set_time_out(
            boost::uint32_t time_out, 
            boost::system::error_code & ec)
        {
            ec = boost::system::error_code();
            //boost::asio::socket_base::non_blocking_io cmd(time_out);
            //{
            //    boost::asio::detail::mutex::scoped_lock lock(mutex_);
            //    if (is_open() && io_control(cmd, ec)) {
            //        return ec;
            //    }
            //}
            connector_.set_time_out(time_out);
            return ec;
        }

        bool TcpSocket::get_non_block(
            boost::system::error_code & ec) const
        {
            ec.clear();
            return connector_.get_non_block();
        }

        boost::uint32_t TcpSocket::get_time_out(
            boost::system::error_code & ec) const
        {
            ec.clear();
            return connector_.get_time_out();
        }

        void TcpSocket::connect(
            framework::network::NetName const & addr)
        {
            return connector_.connect(*this, 
                framework::network::NetName(framework::network::NetName::tcp, addr));
        }

        boost::system::error_code TcpSocket::connect(
            framework::network::NetName const & addr, 
            boost::system::error_code & ec)
        {
            return connector_.connect(*this, 
                framework::network::NetName(framework::network::NetName::tcp, addr), ec);
        }

        TcpSocket::Statistics const & TcpSocket::stat() const
        {
            return connector_.stat();
        }

        void TcpSocket::accept(
            Acceptor & acceptor)
        {
            acceptor.accept<boost::asio::ip::tcp>(*this);
        }

        boost::system::error_code TcpSocket::accept(
            Acceptor & acceptor, 
            boost::system::error_code & ec)
        {
            acceptor.accept<boost::asio::ip::tcp>(*this, ec);
            return ec;
        }

        void TcpSocket::cancel()
        {
            boost::asio::detail::mutex::scoped_lock lock(mutex_);
            connector_.cancel();
            // 取消异步操作，在某些平台仅仅close或者shutdown不能取消异步操作
            super::cancel();
#ifndef BOOST_WINDOWS_API
            // linux 需要shutdown套接字才能取消同步阻塞操作
            super::shutdown(boost::asio::socket_base::shutdown_both);
#else           // win32 shutdown套接字会发生错误
            super::close();
#endif
        }

        boost::system::error_code TcpSocket::cancel(
            boost::system::error_code & ec)
        {
            boost::asio::detail::mutex::scoped_lock lock(mutex_);
            connector_.cancel(ec);
            // 取消异步操作，在某些平台仅仅close或者shutdown不能取消异步操作
            super::cancel(ec);
#ifndef BOOST_WINDOWS_API
            // linux 需要shutdown套接字才能取消同步阻塞操作
            super::shutdown(boost::asio::socket_base::shutdown_both, ec);
#else           // win32 shutdown套接字会发生错误
            super::close(ec);
#endif
            return ec;
        }

        void TcpSocket::cancel_forever()
        {
            boost::asio::detail::mutex::scoped_lock lock(mutex_);
            connector_.cancel_forever();
            // 取消异步操作，在某些平台仅仅close或者shutdown不能取消异步操作
            super::cancel();
#ifndef BOOST_WINDOWS_API
            // linux 需要shutdown套接字才能取消同步阻塞操作
            super::shutdown(boost::asio::socket_base::shutdown_both);
#else           // win32 shutdown套接字会发生错误
            super::close();
#endif
        }

        boost::system::error_code TcpSocket::cancel_forever(
            boost::system::error_code & ec)
        {
            boost::asio::detail::mutex::scoped_lock lock(mutex_);
            connector_.cancel_forever(ec);
            // 取消异步操作，在某些平台仅仅close或者shutdown不能取消异步操作
            super::cancel(ec);
#ifndef BOOST_WINDOWS_API
            // linux 需要shutdown套接字才能取消同步阻塞操作
            super::shutdown(boost::asio::socket_base::shutdown_both, ec);
#else           // win32 shutdown套接字会发生错误
            super::close(ec);
#endif
            return ec;
        }

        void TcpSocket::close()
        {
            boost::asio::detail::mutex::scoped_lock lock(mutex_);
            connector_.close();
            super::close();
        }

        boost::system::error_code TcpSocket::close(
            boost::system::error_code & ec)
        {
            boost::asio::detail::mutex::scoped_lock lock(mutex_);
            connector_.close(ec);
            return super::close(ec);
        }

    } // namespace network
} // namespace framework
