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
            struct timed_dispatcher
            {
                timed_dispatcher(
                    boost::asio::deadline_timer & timer, 
                    boost::uint32_t delay, 
                    Canceler const & canceler)
                    : canceler_(canceler)
                    , timer_(&timer)
                    , time_out_or_callback_(false)
                    , nref_(0)
                {
                    timer_->expires_from_now(boost::posix_time::milliseconds(delay));
                    timer_->async_wait(boost::bind(&timed_dispatcher::handler_timer, boost::intrusive_ptr<timed_dispatcher>(this), _1));
                }

                template <
                    typename Handler
                >
                void dispatch(
                    Handler const & handler)
                {
                    if (time_out_or_callback_) {
                        boost_asio_handler_invoke_helpers::invoke(handler, &handler);
                    } else {
                        time_out_or_callback_ = true;
                        delay_handler_.bind(handler);
                        timer_->cancel(ec_);
                    }
                }

                void handler_timer(
                    boost::system::error_code & ec)
                {
                    if (time_out_or_callback_) {
                        delay_handler_();
                        delay_handler_.clear();
                    } else {
                        time_out_or_callback_ = true;
                        canceler_();
                    }
                }

            private:
                Canceler canceler_;
                boost::asio::deadline_timer * timer_;
                bool time_out_or_callback_;
                BindHandler delay_handler_;
                boost::system::error_code ec_;
            public:
                size_t nref_;
            };

            template <typename Canceler>
            void intrusive_ptr_add_ref(timed_dispatcher<Canceler>* p)
            {
                ++p->nref_;
            }

            template <typename Canceler>
            void intrusive_ptr_release(timed_dispatcher<Canceler>* p)
            {
                if (--p->nref_ == 0)
                    delete p;
            }

            template <
                typename Canceler
            >
            struct timed_dispatcher_ref
            {
                timed_dispatcher_ref(
                    boost::asio::deadline_timer & timer, 
                    boost::uint32_t delay, 
                    Canceler const & canceler)
                    : dispatcher_(new timed_dispatcher<Canceler>(timer, delay, canceler))
                {
                }

                template <
                    typename Handler
                >
                void dispatch(
                    Handler const & handler)
                {
                    dispatcher_->dispatch(handler);
                }

                boost::intrusive_ptr<timed_dispatcher<Canceler> > dispatcher_;
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
            Handler const & handler, 
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
            Handler const & handler, 
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
            Handler const & handler)
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
