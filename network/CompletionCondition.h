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
                enable_cancel(
                    bool * cancel = NULL)
                    : cancel_(cancel ? cancel : &my_cancel_)
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
                explicit transfer_all_t(
                    bool * b = NULL)
                    : enable_cancel(b)
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
                    std::size_t minimum, 
                    bool * b = NULL)
                    : enable_cancel(b)
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

        inline detail::transfer_all_t transfer_all(
            bool * b)
        {
            return detail::transfer_all_t(b);
        }

        inline detail::transfer_at_least_t transfer_at_least(
            std::size_t minimum, 
            bool * b)
        {
            return detail::transfer_at_least_t(minimum, b);
        }

    } // namespace network
} // namespace framework

#endif // _FRAMEWORK_NETWORK_COMPLETION_CONDITION_H_
