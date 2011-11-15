// LimitNumber.h

#ifndef _FRAMEWORK_SYSTEM_LIMIT_NUMBER_H_
#define _FRAMEWORK_SYSTEM_LIMIT_NUMBER_H_

namespace framework
{
    namespace system
    {

        template <
            size_t N, 
            typename Ty = boost::uint64_t
        >
        class LimitNumber
        {
        public:
            LimitNumber(
                Ty n = 0)
                : mask_((Ty)1 << N)
                , low_bits_(n)
                , high_bits_(0)
            {
                assert(n < mask_);
            }
 
        public:
            Ty transfer(
                Ty n)
            {
                assert(n < mask_);
                if ((low_bits_ ^ n) & mask_) {
                    if (low_bits_ & mask_) {
                        high_bits_ += mask_;
                    }
                }
                low_bits_ = n;
                return low_bits_ | high_bits_;
            }
 
        private:
            Ty mask_;
            Ty low_bits_;
            Ty high_bits_;
        };

    }
}

#endif // _FRAMEWORK_SYSTEM_LIMIT_NUMBER_H_

