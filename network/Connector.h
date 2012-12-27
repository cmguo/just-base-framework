// Connector.h

#ifndef _FRAMEWORK_NETWORK_CONNECTOR_H_
#define _FRAMEWORK_NETWORK_CONNECTOR_H_

#include "framework/network/NetName.h"
#include "framework/network/Resolver.h"
#include "framework/network/Statistics.h"
#include "framework/network/TimedHandler.h"

#include "framework/logger/Logger.h"

namespace framework
{
    namespace network
    {

        class Connector
        {
            FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("framework.network.Connector", framework::logger::Warn);

        public:
            Connector(
                boost::asio::io_service & io_svc, 
                boost::asio::detail::mutex & mutex);

        public:
            void set_non_block(
                bool non_block);

            void set_time_out(
                boost::uint32_t time_out);

            boost::system::error_code set_non_block(
                bool non_block, 
                boost::system::error_code & ec);

            boost::uint32_t get_time_out() const;

            bool get_non_block() const;

        public:
            template <typename SocketType>
            boost::system::error_code connect(
                SocketType & peer, 
                Endpoint const & endpoint, 
                boost::system::error_code & ec);

            template <typename SocketType>
            boost::system::error_code connect(
                SocketType & peer, 
                NetName const & netname, 
                boost::system::error_code & ec);

            template <typename SocketType>
            void connect(
                SocketType & peer, 
                Endpoint const & netname);

            template <typename SocketType>
            void connect(
                SocketType & peer, 
                NetName const & netname);

            template <
                typename SocketType, 
                typename ConnectHandler
            >
            void async_connect(
                SocketType & peer, 
                Endpoint const & netname, 
                ConnectHandler const & handler);

            template <
                typename SocketType, 
                typename ConnectHandler
            >
            void async_connect(
                SocketType & peer, 
                NetName const & netname, 
                ConnectHandler const & handler);

            boost::system::error_code cancel_forever(
                boost::system::error_code & ec);

            void cancel_forever();

            boost::system::error_code cancel(
                boost::system::error_code & ec);

            void cancel();

            boost::system::error_code close(
                boost::system::error_code & ec);

            void close();

        public:
            struct Statistics
                : public TimeStatistics
            {
                void reset()
                {
                    TimeStatistics::reset();
                    resolve_time = connect_time = (boost::uint32_t)-1;
                    last_error.clear();
                }

                void zero()
                {
                    resolve_time = connect_time = 0;
                    last_error.clear();
                }

                boost::uint32_t resolve_time;
                boost::uint32_t connect_time; // total time
                boost::system::error_code last_error;
            };

            Statistics const & stat() const
            {
                return stat_;
            }

        private:
            template <typename SocketType>
            void start_connect(
                SocketType & peer, 
                Endpoint const & ep, 
                boost::system::error_code & ec);

            template <typename SocketType>
            void pool_connect(
                SocketType & peer, 
                boost::system::error_code & ec);

            template <typename SocketType>
            void post_connect(
                SocketType & peer, 
                boost::system::error_code & ec);

        private:
            bool non_block_;
            boost::uint32_t time_out_; // 同步connector超时时间/ms
            bool started_;
            bool connect_started_;
            bool canceled_;
            bool canceled_forever_;
            Resolver resolver_;
            ResolverIterator resolver_iterator_;
            boost::asio::detail::mutex & mutex_;
            Statistics stat_;
            TimedHandler timed_handler_;
        };

    } // namespace network
} // namespace framework

#endif // _FRAMEWORK_NETWORK_CONNECTOR_H_
