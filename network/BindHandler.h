// BindHandler.h

#ifndef _FRAMEWORK_NETWORK_BIND_HANDLER_H_
#define _FRAMEWORK_NETWORK_BIND_HANDLER_H_

namespace framework
{
    namespace network
    {
        
        class BindHandler
        {
        public:
            BindHandler()
                : handler_(NULL)
                , invoke_handler_(NULL)
                , delete_handler_(NULL)
            {
            }

            BindHandler(
                BindHandler & r)
                : handler_(r.handler_)
                , invoke_handler_(r.invoke_handler_)
                , delete_handler_(r.delete_handler_)
            {
                r.delete_handler_ = NULL;
            }

            BindHandler(
                BindHandler const & r)
                : handler_(r.handler_)
                , invoke_handler_(r.invoke_handler_)
                , delete_handler_(NULL)
            {
            }

            template <
                typename Handler
            >
            BindHandler(
                Handler const & handler)
            {
                handler_ = new Handler(handler);
                invoke_handler_ = invoke_handler<Handler>;
                delete_handler_ = delete_handler<Handler>;
            }

            template <
                typename Handler, 
                typename Arg1
            >
            BindHandler(
                Handler const & handler, 
                Arg1 const & arg1)
            {
                typedef boost::asio::detail::binder1<Handler, Arg1> bind_handler;
                handler_ = new bind_handler(handler, arg1);
                invoke_handler_ = invoke_handler<bind_handler>;
                delete_handler_ = delete_handler<bind_handler>;
            }

            template <
                typename Handler, 
                typename Arg1, 
                typename Arg2
            >
            BindHandler(
                Arg1 const & arg1, 
                Arg2 const & arg2)
            {
                typedef boost::asio::detail::binder2<Handler, Arg1, Arg2> bind_handler;
                handler_ = new bind_handler(handler, arg1, arg2);
                invoke_handler_ = invoke_handler<bind_handler>;
                delete_handler_ = delete_handler<bind_handler>;
            }

            template <
                typename Handler, 
                typename Arg1, 
                typename Arg2, 
                typename Arg3
            >
            BindHandler(
                Arg1 const & arg1, 
                Arg2 const & arg2, 
                Arg3 const & arg3)
            {
                typedef boost::asio::detail::binder3<Handler, Arg1, Arg2, Arg3> bind_handler;
                handler_ = new bind_handler(handler, arg1, arg2, arg3);
                invoke_handler_ = invoke_handler<bind_handler>;
                delete_handler_ = delete_handler<bind_handler>;
            }

            ~BindHandler()
            {
                if (delete_handler_)
                    delete_handler_(handler_);
            }

        public:
            BindHandler & operator=(
                BindHandler & r)
            {
                BindHandler(r).swap(*this);
                return *this;
            }

            BindHandler & operator=(
                BindHandler const & r)
            {
                BindHandler(r).swap(*this);
                return *this;
            }

        public:
            template <
                typename Handler
            >
            static void invoke_handler(
                void * handler)
            {
                Handler * handler2 = (Handler *)handler;
                (*handler2)();
            }

            template <
                typename Handler
            >
            static void delete_handler(
                void * handler)
            {
                Handler * handler2 = (Handler *)handler;
                delete handler2;
            }

            void operator()()
            {
                invoke_handler_(handler_);
            }

        private:
            void swap(
                BindHandler & r)
            {
                std::swap(handler_, r.handler_);
                std::swap(invoke_handler_, r.invoke_handler_);
                std::swap(delete_handler_, r.delete_handler_);
            }

        private:
            void * handler_;
            void (* invoke_handler_)(void *);
            void (* delete_handler_)(void *);
        };

    } // namespace network
} // namespace framework

#endif // _FRAMEWORK_NETWORK_BIND_HANDLER_H_
