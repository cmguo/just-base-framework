// TimedHandler.hpp

#ifndef _FRAMEWORK_NETWORK_TIMED_HANDLER_HPP_
#define _FRAMEWORK_NETWORK_TIMED_HANDLER_HPP_

#include "framework/network/TimedHandler.h"
#include "framework/network/BindHandler.h"
#include "framework/network/AsioHandlerHelper.h"

#include <boost/intrusive_ptr.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/detail/bind_handler.hpp>

namespace framework
{
    namespace network
    {
        
        namespace detail
        {

            static inline boost::system::error_code _m(
                boost::system::error_code const &)
            {
                return boost::asio::error::timed_out;
            }

            template <typename Arg>
            static inline Arg const & _m(
                Arg const * arg)
            {
                return arg;
            }

            template <
                typename Canceler
            >
            struct timed_dispatcher_impl
            {
                timed_dispatcher_impl(
                    boost::asio::deadline_timer & timer, 
                    boost::uint32_t delay, 
                    Canceler const & canceler)
                    : nref_(0)
                    , canceler_(canceler)
                    , timer_(&timer)
                    , time_out_or_callback_(false)
                    {
                    }
                size_t nref_;
                Canceler canceler_;
                boost::asio::deadline_timer * timer_;
                bool time_out_or_callback_;
                BindHandler delay_handler_;
                boost::system::error_code ec_;
            };

            template <typename Canceler>
            void intrusive_ptr_add_ref(timed_dispatcher_impl<Canceler> * p)
            {
                ++p->nref_;
            }

            template <typename Canceler>
            void intrusive_ptr_release(timed_dispatcher_impl<Canceler> * p)
            {
                if (--p->nref_ == 0)
                    delete p;
            }


            template <
                typename Canceler
            >
            struct timed_dispatcher
            {
                static bool running_in_this_thread() { return true; }

                typedef timed_dispatcher_impl<Canceler> impl_t;

                timed_dispatcher(
                    boost::asio::deadline_timer & timer, 
                    boost::uint32_t delay, 
                    Canceler const & canceler)
                    : impl_(new impl_t(timer, delay, canceler))
                {
                    timer.expires_from_now(boost::posix_time::milliseconds(delay));
                    timer.async_wait(boost::bind(&timed_dispatcher::handler_timer, impl_, _1));
                }

                template <
                    typename Handler
                >
                void dispatch(
                    Handler const & handler)
                {
                    if (impl_->time_out_or_callback_) {
                        boost_asio_handler_invoke_helpers::invoke(handler, handler);
                    } else {
                        impl_->time_out_or_callback_ = true;
                        impl_->delay_handler_.bind(handler);
                        impl_->timer_->cancel(impl_->ec_);
                    }
                }

                static void handler_timer(
                    boost::intrusive_ptr<impl_t> impl, 
                    boost::system::error_code const & ec)
                {
                    if (impl->time_out_or_callback_) {
                        impl->delay_handler_();
                        impl->delay_handler_.clear();
                    } else {
                        impl->time_out_or_callback_ = true;
                        impl->canceler_();
                    }
                }

            private:
                boost::intrusive_ptr<impl_t> impl_;
            };

        } // namespace detail

        template <
            typename Canceler
        >
        TimedHandler::TimedHandler(
            boost::asio::io_service & io_svc, 
            boost::uint32_t delay, 
            Canceler const & canceler)
            : timer_(io_svc)
            , delay_(delay)
        {
            canceler_ = new BindHandler(canceler);
        }

        template <
            typename Handler, 
            typename Canceler
        >
        typename detail::timed_wrapped_type<Handler, Canceler>::wrapped_handler TimedHandler::wrap(
            boost::uint32_t delay, 
            Handler handler, 
            Canceler const & canceler)
        {
            typedef typename detail::timed_wrapped_type<Handler, Canceler>::dispatcher dispatcher;
            typedef typename detail::timed_wrapped_type<Handler, Canceler>::wrapped_handler wrapped_handler;
            dispatcher d(timer_, delay, canceler);
            return wrapped_handler(d, handler);
        }

        template <
            typename Handler, 
            typename Canceler
        >
        typename detail::timed_wrapped_type<Handler, Canceler>::wrapped_handler TimedHandler::wrap(
            Handler handler, 
            Canceler const & canceler)
        {
            typedef typename detail::timed_wrapped_type<Handler, Canceler>::dispatcher dispatcher;
            typedef typename detail::timed_wrapped_type<Handler, Canceler>::wrapped_handler wrapped_handler;
            dispatcher d(timer_, delay_, canceler);
            return wrapped_handler(d, handler);
        }

        template <
            typename Handler
        >
        typename detail::timed_wrapped_type<Handler, RefHandler<BindHandler const> >::wrapped_handler TimedHandler::wrap(
            Handler handler)
        {
            assert(canceler_);
            typedef typename detail::timed_wrapped_type<Handler, RefHandler<BindHandler const> >::dispatcher dispatcher;
            typedef typename detail::timed_wrapped_type<Handler, RefHandler<BindHandler const> >::wrapped_handler wrapped_handler;
            dispatcher d(timer_, delay_, RefHandler<BindHandler>(*canceler_));
            return wrapped_handler(d, handler);
        }

    } // namespace network
} // namespace framework

#endif // _FRAMEWORK_NETWORK_TIMED_HANDLER_HPP_
