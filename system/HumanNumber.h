// HumanNumber.h

#ifndef _FRAMEWORK_SYSTEM_HUMAN_NUMBER_H_
#define _FRAMEWORK_SYSTEM_HUMAN_NUMBER_H_

#include <framework/system/BytesOrder.h>

namespace framework
{
    namespace system
    {

        template <
            typename _Ty = boost::uint64_t
        >
        class HumanNumber
        {
        public:
            HumanNumber()
                : n_(0)
            {
            }
 
            HumanNumber(
                _Ty n)
                : n_(n)
            {
            }
 
            HumanNumber(
                HumanNumber const & r)
                : n_(r.n_)
            {
            }
 
        public:
            operator _Ty() const
            {
                return n_;
            }
 
            _Ty value() const
            {
                return n_;
            }

        public:
            bool from_string(
                std::string const & str);

            std::string to_string() const;

        public:
            HumanNumber & operator=(
                HumanNumber const & r)
            {
                n_ = r.n_;
                return *this;
            }

            HumanNumber & operator=(
                _Ty n)
            {
                n_ = n;
                return *this;
            }

            friend bool operator==(
                HumanNumber const & l, 
                HumanNumber const & r)
            {
                return l.n_ == r.n_;
            }

            friend bool operator==(
                HumanNumber const & l, 
                _Ty r)
            {
                return l.n_ == r;
            }

            friend bool operator<(
                HumanNumber const & l, 
                HumanNumber const & r)
            {
                return l.n_ < r.n_;
            }

            friend bool operator<(
                HumanNumber const & l, 
                _Ty r)
            {
                return l.n_ < r;
            }

            friend HumanNumber rotate(
                HumanNumber const & v)
            {
                return HumanNumber(BytesOrder::rotate(v.n_));
            }

        private:
            _Ty n_;
        };

    }
}

#endif // _FRAMEWORK_SYSTEM_HUMAN_NUMBER_H_

