// TimedHandler.h

#ifndef _FRAMEWORK_NETWORK_TIMED_HANDLER_H_
#define _FRAMEWORK_NETWORK_TIMED_HANDLER_H_

#include <boost/asio/deadline_timer.hpp>

namespace framework
{
    namespace network
    {
        
        namespace detail
        {

            template <
                typename Handler, 
                typename Canceler
            >
            struct timed_wrap_t;

        } // namespace detail

        class BindHandler;

        class TimedHandler
        {
        public:
            TimedHandler(
                boost::asio::io_service & io_svc);

            TimedHandler(
                boost::asio::io_service & io_svc, 
                boost::uint32_t delay);

            template <
                typename Canceler
            >
            TimedHandler(
                boost::asio::io_service & io_svc, 
                boost::uint32_t delay, 
                Canceler const & canceler);

            ~TimedHandler();

        public:
            template <
                typename Handler, 
                typename Canceler
            >
            detail::timed_wrap_t<Handler, Canceler> wrap(
                boost::uint32_t delay, 
                Handler const & handler, 
                Canceler const & canceler);

            template <
                typename Handler, 
                typename Canceler
            >
            detail::timed_wrap_t<Handler, Canceler> wrap(
                Handler const & handler, 
                Canceler const & canceler);

            template <
                typename Handler
            >
            detail::timed_wrap_t<Handler, BindHandler> wrap(
                Handler const & handler);

        private:
            boost::asio::deadline_timer timer_;
            boost::uint32_t delay_;
            BindHandler const * canceler_;
        };

    } // namespace network
} // namespace framework

#endif // _FRAMEWORK_NETWORK_TIMED_HANDLER_H_
