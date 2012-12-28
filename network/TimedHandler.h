// TimedHandler.h

#ifndef _FRAMEWORK_NETWORK_TIMED_HANDLER_H_
#define _FRAMEWORK_NETWORK_TIMED_HANDLER_H_

#include "framework/network/RefHandler.h"

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/detail/wrapped_handler.hpp>

namespace framework
{
    namespace network
    {
        
        namespace detail
        {

            template <
                typename Canceler
            >
            struct timed_dispatcher_ref;

            template <
                typename Handler, 
                typename Canceler
            >
            struct timed_wrapped_type
            {
                typedef detail::timed_dispatcher_ref<Canceler> dispatcher;
                typedef boost::asio::detail::wrapped_handler<dispatcher, Handler> wrapped_handler;
            };

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
            typename detail::timed_wrapped_type<Handler, Canceler>::wrapped_handler wrap(
                boost::uint32_t delay, 
                Handler const & handler, 
                Canceler const & canceler);

            template <
                typename Handler, 
                typename Canceler
            >
            typename detail::timed_wrapped_type<Handler, Canceler>::wrapped_handler wrap(
                Handler const & handler, 
                Canceler const & canceler);

            template <
                typename Handler
            >
            typename detail::timed_wrapped_type<Handler, RefHandler<BindHandler const> >::wrapped_handler wrap(
                Handler const & handler);

        private:
            boost::asio::deadline_timer timer_;
            boost::uint32_t delay_;
            BindHandler const * canceler_;
        };

    } // namespace network
} // namespace framework

#endif // _FRAMEWORK_NETWORK_TIMED_HANDLER_H_
