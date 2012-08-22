// CompletionCondition.h

#ifndef _FRAMEWORK_NETWORK_COMPLETION_CONDITION_H_
#define _FRAMEWORK_NETWORK_COMPLETION_CONDITION_H_

namespace framework
{
    namespace network
    {

        namespace detail
        {

            class enable_cancel
            {
            public:
                typedef bool result_type;

            protected:
                enable_cancel()
                    : cancel_(&my_cancel_)
                    , my_cancel_(false)
                {
                }

                enable_cancel(
                    bool * cancel)
                    : cancel_(cancel)
                    , my_cancel_(false)
                {
                    *cancel_ = false;
                }

                enable_cancel(
                    enable_cancel const & r)
                    : cancel_(r.cancel_)
                    , my_cancel_(false)
                {
                    if (r.cancel_ == &r.my_cancel_)
                        cancel_ = &my_cancel_;
                }

            protected:
                bool canceled() const
                {
                    return *cancel_;
                }

            private:
                bool * cancel_;
                bool my_cancel_;
            };

            class transfer_all_t
                : public enable_cancel
            {
            public:
                transfer_all_t()
                {
                }

                template <typename Arg>
                transfer_all_t(Arg arg)
                    : enable_cancel(arg)
                {
                }

                template <typename Error>
                bool operator()(
                    const Error& err, 
                    std::size_t)
                {
                    return canceled() || (!!err);
                }
            };

            class transfer_at_least_t
                : public enable_cancel
            {
            public:
                explicit transfer_at_least_t(
                    std::size_t minimum)
                    : minimum_(minimum)
                {
                }

                template <typename Arg>
                explicit transfer_at_least_t(
                    std::size_t minimum, 
                    Arg arg)
                    : enable_cancel(arg)
                    , minimum_(minimum)
                {
                }

                template <typename Error>
                bool operator()(
                    const Error& err, 
                    std::size_t bytes_transferred)
                {
                    return canceled() || (!!err) || bytes_transferred >= minimum_;
                }

            protected:
                std::size_t minimum_;
            };

        }

        inline detail::transfer_all_t transfer_all()
        {
            return detail::transfer_all_t();
        }

        template <typename Arg>
        inline detail::transfer_all_t transfer_all(
            Arg arg)
        {
            return detail::transfer_all_t(arg);
        }

        inline detail::transfer_at_least_t transfer_at_least(
            std::size_t minimum)
        {
            return detail::transfer_at_least_t(minimum);
        }

        template <typename Arg>
        inline detail::transfer_at_least_t transfer_at_least(
            std::size_t minimum, 
            Arg arg)
        {
            return detail::transfer_at_least_t(minimum, arg);
        }

    } // namespace network
} // namespace framework

#endif // _FRAMEWORK_NETWORK_COMPLETION_CONDITION_H_
