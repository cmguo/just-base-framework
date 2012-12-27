// TimedHandler.hpp

#ifndef _FRAMEWORK_NETWORK_TIMED_HANDLER_HPP_
#define _FRAMEWORK_NETWORK_TIMED_HANDLER_HPP_

#include "framework/network/TimedHandler.h"
#include "framework/network/BindHandler.h"
#include "framework/network/AsioHandlerHelper.h"

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
                typename Handler, 
                typename Canceler
            >
            struct timed_wrap_t
            {
                timed_wrap_t(
                    boost::asio::deadline_timer & timer, 
                    boost::uint32_t delay, 
                    Handler const & handler, 
                    Canceler const & canceler)
                    : handler_(handler)
                    , canceler_(canceler)
                    , timer_(&timer)
                    , time_out_(false)
                {
                    timer_->expires_from_now(boost::posix_time::milliseconds(delay));
                    timer_->async_wait(boost::bind(&timed_wrap_t::handler_timer, this, _1));
                }

                void operator()()
                {
                    if (time_out_) {
                        handler_();
                        delete this;
                    } else {
                        delay_handler_ = BindHandler(handler_);
                        timer_->cancel(ec_);
                    }
                }

                template <typename Arg1>
                void operator()(
                    Arg1 const & arg1)
                {
                    if (time_out_) {
                        handler_(_m(arg1));
                        delete this;
                    } else {
                        delay_handler_ = BindHandler(handler_, arg1);
                        timer_->cancel(ec_);
                    }
                }

                template <typename Arg1, typename Arg2>
                void operator()(
                    Arg1 const & arg1, 
                    Arg2 const & arg2)
                {
                    if (time_out_) {
                        handler_(_m(arg1), _m(arg2));
                        delete this;
                    } else {
                        delay_handler_ = BindHandler(handler_, arg1, arg2);
                        timer_->cancel(ec_);
                    }
                }

                template <typename Arg1, typename Arg2, typename Arg3>
                void operator()(
                    Arg1 const & arg1, 
                    Arg2 const & arg2, 
                    Arg3 const & arg3)
                {
                    if (time_out_) {
                        handler_(_m(arg1), _m(arg2), _m(arg3));
                        delete this;
                    } else {
                        delay_handler_ = BindHandler(handler_, arg1, arg2, arg3);
                        timer_->cancel(ec_);
                    }
                }

                void handler_timer(
                    boost::system::error_code & ec)
                {
                    if (time_out_) {
                        delay_handler_();
                        delete this;
                    } else {
                        time_out_ = true;
                        canceler_();
                    }
                }

                Handler handler_;
                Canceler canceler_;
                boost::asio::deadline_timer * timer_;
                bool time_out_;
                BindHandler delay_handler_;
                boost::system::error_code ec_;
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
        detail::timed_wrap_t<Handler, Canceler> TimedHandler::wrap(
            boost::uint32_t delay, 
            Handler const & handler, 
            Canceler const & canceler)
        {
            return detail::timed_wrap_t<Handler, Canceler>(timer_, delay, handler, canceler);
        }

        template <
            typename Handler, 
            typename Canceler
        >
        detail::timed_wrap_t<Handler, Canceler> TimedHandler::wrap(
            Handler const & handler, 
            Canceler const & canceler)
        {
            return detail::timed_wrap_t<Handler, Canceler>(timer_, delay_, handler, canceler);
        }

        template <
            typename Handler
        >
        detail::timed_wrap_t<Handler, BindHandler> TimedHandler::wrap(
            Handler const & handler)
        {
            return detail::timed_wrap_t<Handler, BindHandler>(timer_, delay_, handler, *canceler_);
        }

    } // namespace network
} // namespace framework

#endif // _FRAMEWORK_NETWORK_TIMED_HANDLER_HPP_
