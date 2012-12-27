// Connector.cpp

#include "framework/Framework.h"
#include "framework/network/Connector.h"

#include "framework/logger/Logger.h"
#include "framework/logger/StreamRecord.h"

namespace framework
{
    namespace network
    {

        Connector::Connector(
            boost::asio::io_service & io_svc, 
            boost::asio::detail::mutex & mutex)
            : non_block_(false)
            , time_out_(0)
            , started_(false)
            , connect_started_(false)
            , canceled_(false)
            , canceled_forever_(false)
            , resolver_(io_svc)
            , mutex_(mutex)
            , timed_handler_(io_svc)
        {
        }

        void Connector::set_non_block(
            bool non_block)
        {
            non_block_ = non_block;
        }

        void Connector::set_time_out(
            boost::uint32_t time_out)
        {
            time_out_ = time_out;
        }

        boost::system::error_code Connector::set_non_block(
            bool non_block, 
            boost::system::error_code & ec)
        {
            non_block_ = non_block;
            return ec = boost::system::error_code();
        }

        boost::uint32_t Connector::get_time_out() const
        {
            return time_out_;
        }

        bool Connector::get_non_block() const
        {
            return non_block_;
        }

        boost::system::error_code Connector::cancel_forever(
            boost::system::error_code & ec)
        {
            canceled_ = true;
            canceled_forever_ = true;
            resolver_.cancel(ec);
            return ec;
        }

        void Connector::cancel_forever()
        {
            canceled_ = true;
            canceled_forever_ = true;
            boost::system::error_code ec;
            cancel(ec);
            boost::asio::detail::throw_error(ec);
        }

        boost::system::error_code Connector::cancel(
            boost::system::error_code & ec)
        {
            canceled_ = true;
            resolver_.cancel(ec);
            return ec;
        }

        void Connector::cancel()
        {
            canceled_ = true;
            boost::system::error_code ec;
            cancel(ec);
            boost::asio::detail::throw_error(ec);
        }

        boost::system::error_code Connector::close(
            boost::system::error_code & ec)
        {
            started_ = false;
            ec.clear();
            canceled_ = false;
            return ec;
        }

        void Connector::close()
        {
            started_ = false;
            canceled_ = false;
        }

    } // namespace network
} // namespace framework
