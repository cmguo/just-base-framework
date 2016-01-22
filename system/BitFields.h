// BitFields.h

#ifndef _FRAMEWORK_SYSTEM_BIT_FIELDS_H_
#define _FRAMEWORK_SYSTEM_BIT_FIELDS_H_

#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_pod.hpp>

#include <memory>

namespace framework {
namespace system {

    template <size_t W>
    struct BitResult
    {
        typedef typename boost::mpl::if_c<
            W < 8, 
            boost::uint8_t, 
            typename boost::mpl::if_c<
                W < 16, 
                boost::uint16_t,
                typename boost::mpl::if_c<
                    W < 32, 
                    boost::uint32_t, 
                    boost::uint64_t
                >::type
            >::type
        >::type type;
    };

    template <size_t T, size_t W, size_t S>
    struct BitField
    {
        typedef BitField<T, S, W> BF_TYPE;
        typedef typename BitResult<T>::type BF_BASE_TYPE;
        typedef typename BitResult<S>::type BF_RESULT_SUB;
        typedef typename BitResult<W>::type BF_RESULT;
        typedef typename BitResult<S + W>::type BF_RESULT_ALL;
        static size_t const BF_SHIFT        = S;
        static size_t const BF_WIDTH        = W;
        static size_t const BF_WIDTH_ALL    = S + W;
        static BF_RESULT_SUB const BF_MASK_SUB  = (BF_RESULT_SUB(1) << S) - 1;
        static BF_RESULT_ALL const BF_MASK      = ((BF_RESULT_ALL(1) << W) - 1) << S;
        static BF_RESULT_ALL const BF_MASK_ALL  = (BF_RESULT_ALL(1) << (S + W)) - 1;
        static BF_RESULT_SUB const BF_MAX_SUB   = BF_RESULT_SUB(1) << S;
        static BF_RESULT     const BF_MAX       = BF_RESULT(1) << W;
        static BF_RESULT_ALL const BF_MAX_ALL   = BF_RESULT_ALL(1) << (S +W);

        static BF_RESULT_SUB get_sub(
            BF_BASE_TYPE n)
        {
            return BF_RESULT_SUB(n & BF_MASK_SUB);
        }

        static BF_RESULT get(
            BF_BASE_TYPE n)
        {
            return BF_RESULT((n & BF_MASK) >> BF_SHIFT);
        }

        static BF_RESULT get_all(
            BF_BASE_TYPE n)
        {
            return BF_RESULT_ALL(n & BF_MASK_ALL);
        }

        static BF_BASE_TYPE set_sub(
            BF_BASE_TYPE n, 
            BF_RESULT_SUB d)
        {
            assert(d < BF_MAX_SUB);
            return (n & ~BF_MAX_SUB) | d;
        }

        // also set sub = 0
        static BF_BASE_TYPE set(
            BF_BASE_TYPE n, 
            BF_RESULT d)
        {
            assert(d < BF_MAX);
            return (n & ~BF_MASK_ALL) | (d << BF_SHIFT);
        }

        static BF_BASE_TYPE set_all(
            BF_BASE_TYPE n, 
            BF_RESULT_ALL d)
        {
            assert(d < BF_MAX_ALL);
            return (n & ~BF_MAX_ALL) | d;
        }

        // also set sub = 0
        static BF_BASE_TYPE inc(
            BF_BASE_TYPE n)
        {
            assert(((n >> BF_SHIFT) + 1) < BF_MAX);
            return ((n >> BF_SHIFT) + 1) << BF_SHIFT;
        }

        // also set sub = 0
        static BF_BASE_TYPE inc(
            BF_BASE_TYPE n, 
            BF_RESULT d)
        {
            assert(((n >> BF_SHIFT) + d) < BF_MAX);
            return ((n >> BF_SHIFT) + d) << BF_SHIFT;
        }
    };

    template <
        size_t N, 
        size_t N0, 
        size_t N1 = 0, 
        size_t N2 = 0, 
        size_t N3 = 0, 
        size_t N4 = 0, 
        size_t N5 = 0, 
        size_t N6 = 0, 
        size_t N7 = 0
    >
    struct BitFields
    {
        typedef BitField<N, N0, N1 + N2 + N3 + N4 + N5 + N6 + N7> BF_TYPE;
        typedef BitFields<N, N1, N2, N3, N4, N5, N6, N7, 0> BFS_CHILD;
        static size_t const LEVEL = BFS_CHILD::LEVEL + 1;

        template <size_t L>
        struct bit_field
        {
            typedef typename boost::mpl::if_c<
                L == 0, 
                BF_TYPE, 
                typename BFS_CHILD::template bit_field<L - 1>::type
            >::type type;
        };

        template <class C, class T>
        static void print(
            std::basic_ostream<C, T> & os, 
            typename BF_TYPE::BF_BASE_TYPE n)
        {
            os << (int)BF_TYPE::get(n) << '/';
            BFS_CHILD::print(os, n);
        }
    };

    template <
        size_t N, 
        size_t N0
    >
    struct BitFields<N, N0, 0, 0, 0, 0, 0, 0, 0>
    {
        typedef BitField<N, N0, 0> BF_TYPE;
        typedef BitFields<N, 0, 0, 0, 0, 0, 0, 0, 0> BFS_CHILD;
        static size_t const LEVEL = 1;

        template <size_t L>
        struct bit_field
        {
            typedef typename boost::mpl::if_c<
                L == 0, 
                BF_TYPE, 
                void
            >::type type;
        };

        template <class C, class T>
        static void print(
            std::basic_ostream<C, T> & os, 
            typename BF_TYPE::BF_BASE_TYPE n)
        {
            os << (int)BF_TYPE::get(n);
        }
    };

} // namespace system
} // namespace framework 

#endif // _FRAMEWORK_SYSTEM_BIT_FIELDS_H_
