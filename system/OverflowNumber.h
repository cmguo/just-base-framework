// OverflowNumber.h

#ifndef _FRAMEWORK_SYSTEM_OVERFLOW_NUMBER_H_
#define _FRAMEWORK_SYSTEM_OVERFLOW_NUMBER_H_

namespace framework
{
    namespace system
    {

        template <
            typename Ty = boost::uint64_t
        >
        class OverflowNumber
        {
        public:
            OverflowNumber(
                size_t width)
                : test_bit_((Ty)1 << (width - 1))
                , carry_bit_((Ty)1 << width)
            {
            }

        public:
            void reset(
                Ty n = 0)
                : low_bits_(n)
                , high_bits_(0)
            {
                assert(n < carry_bit_);
            }
 
        public:
            Ty transfer(
                Ty n)
            {
                assert(n < carry_bit_);
                if ((low_bits_ ^ n) & test_bit_) {
                    if (low_bits_ & test_bit_) {
                        high_bits_ += carry_bit_;
                    }
                }
                low_bits_ = n;
                return low_bits_ | high_bits_;
            }

            Ty current() const
            {
                return low_bits_ | high_bits_;
            }

        private:
            Ty test_bit_;
            Ty carry_bit_;
            Ty low_bits_;
            Ty high_bits_;
        };

    }
}

#endif // _FRAMEWORK_SYSTEM_OVERFLOW_NUMBER_H_

