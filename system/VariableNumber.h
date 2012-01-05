// VariableNumber.h

#ifndef _FRAMEWORK_SYSTEM_VARIABLE_NUMBER_H_
#define _FRAMEWORK_SYSTEM_VARIABLE_NUMBER_H_

#include "framework/system/BytesOrder.h"

#include <boost/type_traits/is_unsigned.hpp>

namespace framework
{
    namespace system
    {

        template <
            typename _Ty
        >
        struct VDef
        {
            boost::uint8_t size;
            boost::uint8_t size_left;
            boost::uint8_t mask_bit_test;
            boost::uint8_t mask_bit;
#if (!defined BOOST_BIG_ENDIAN)
            boost::uint8_t data_off;
#endif
            _Ty mask_byte_test;
            _Ty mask_byte;
            _Ty sign_off; // 有符号的差值
        };


        template <
            typename _Ty
        >
        struct VTable
        {
            VDef<_Ty> tbl[sizeof(_Ty)];
        };

        template <
            typename _Ty
        >
        inline VTable<_Ty> make_v_tbl()
        {
            VTable<_Ty> tbl;
            boost::uint8_t mask_bit_test = 0xff;
            boost::uint8_t mask_bit = 0x80;
            _Ty mask_byte_test = (~(_Ty)0) << 7;
            _Ty sign_off = 0x3f;
            for (size_t i = 0; i < sizeof(_Ty); ++i) {
                tbl.tbl[i].size = i + 1;
                tbl.tbl[i].size_left = i;
#if (!defined BOOST_BIG_ENDIAN)
                tbl.tbl[i].data_off = sizeof(_Ty) - 1 - i;
#endif
                tbl.tbl[i].mask_bit_test = ~mask_bit_test;
                mask_bit_test >>= 1;
                tbl.tbl[i].mask_bit = mask_bit;
                mask_bit >>= 1;
                tbl.tbl[i].mask_byte_test = mask_byte_test;
                tbl.tbl[i].mask_byte = ~mask_byte_test;
                mask_byte_test = mask_byte_test << 7;
                tbl.tbl[i].sign_off = sign_off;
                sign_off = (sign_off << 7) | 0x7f;
            }
            return tbl;
        }

        template <
            typename _Ty
        >
        static VTable<_Ty> const & v_tbl()
        {
            static VTable<_Ty> tbl = make_v_tbl<_Ty>();
            return tbl;
        }

        template <
            typename _Ty = boost::uint64_t
        >
        class VariableNumber
        {
        public:
            VariableNumber()
                : v_(v_tbl<_Ty>().tbl[0])
                , n_(0)
            {
            }
 
            VariableNumber(
                _Ty n)
                : v_(v(prepare(n)))
                , n_(encode(n, v_))
            {
            }
 
            VariableNumber(
                boost::uint8_t b = 0)
                : v_(v(b))
                , n_(0)
            {
            }
 
            VariableNumber(
                VariableNumber const & r)
                : v_(r.v_)
                , n_(r.n_)
            {
            }
 
        public:
            operator _Ty() const
            {
                return value();
            }
 
            _Ty value() const
            {
                return decode(n_, v_);
            }

            _Ty size() const
            {
                return v_.size;
            }

            VariableNumber & operator=(
                VariableNumber const & r)
            {
                new (this) VariableNumber(r);
                return *this;
            }

            VariableNumber & operator=(
                _Ty n)
            {
                new (this) VariableNumber(n);
                return *this;
            }

            friend bool operator==(
                VariableNumber const & l, 
                VariableNumber const & r)
            {
                return l.n_ == r. n_;
            }

            friend bool operator<(
                VariableNumber const & l, 
                VariableNumber const & r)
            {
                return l.n_ < r. n_;
            }

        public:
            boost::uint8_t * bytes()
            {
                return (boost::uint8_t *)&n_;
            }
 
            boost::uint8_t const * bytes() const
            {
                return (boost::uint8_t const *)&n_;
            }
 
            VariableNumber encode() const
            {
                return VariableNumber(v_, n_ | v_.mask_bit);
            }

            VariableNumber decode() const
            {
                return VariableNumber(v_, n_ & (~v_.mask_bit));
            }

            boost::uint8_t * data()
            {
#if (defined BOOST_BIG_ENDIAN)
                return (boost::uint8_t *)&n_;
#else
                return (boost::uint8_t *)&n_ + v_.data_off;
#endif
            }
 
            boost::uint8_t const * data() const
            {
#if (defined BOOST_BIG_ENDIAN)
                return (boost::uint8_t const *)&n_;
#else
                return (boost::uint8_t const *)&n_ + v_.data_off;
#endif
            }
 
        private:
            VariableNumber(
                VDef<_Ty> const & v, 
                _Ty n)
                : v_(v_)
                , n_(n_)
            {
            }
 
            typedef boost::is_unsigned<_Ty> is_unsigned;

            static _Ty prepare(
                _Ty n, 
                boost::false_type * is_unsigned)
            {
                _Ty u = (n <= 0 ? (~n + 1) : (n - 1)) << 1;
                return u;
            }

            static _Ty prepare(
                _Ty n, 
                boost::true_type * is_unsigned)
            {
                return (_Ty)n;
            }

            static _Ty prepare(
                _Ty n)
            {
                return prepare(n, (is_unsigned *)NULL);
            }

            static _Ty encode(
                _Ty n, 
                VDef<_Ty> const & v, 
                boost::false_type * is_unsigned)
            {
                return ((_Ty)n + v.sign_off) & v.mask_byte;
            }

            static _Ty encode(
                _Ty n, 
                VDef<_Ty> const & v, 
                boost::true_type * is_unsigned)
            {
                return (_Ty)n;
            }

            static _Ty encode(
                _Ty n, 
                VDef<_Ty> const & v)
            {
                return encode(n, v, (is_unsigned *)NULL);
            }

            static _Ty decode(
                _Ty n, 
                VDef<_Ty> const & v, 
                boost::false_type * is_unsigned)
            {
                return n - v.sign_off;
            }

            static _Ty decode(
                _Ty n, 
                VDef<_Ty> const & v, 
                boost::true_type * is_unsigned)
            {
                return (_Ty)n;
            }

            static _Ty decode(
                _Ty n, 
                VDef<_Ty> const & v)
            {
                return decode(n, v, (is_unsigned *)NULL);
            }

            static VDef<_Ty> const & v(
                boost::uint8_t n)
            {
                size_t b = 0;
                size_t e = sizeof(_Ty) - 1;
                size_t m = (b + e) / 2;
                VDef<_Ty> const (& tbl)[sizeof(_Ty)] = v_tbl<_Ty>().tbl;
                while (m > b || m < e) {
                    if (tbl[m].mask_bit_test & n) {
                        b = m + 1;
                    } else {
                        e = m;
                    }
                    m = (b + e) / 2;
                }
                assert((tbl[m].mask_bit_test & n) == 0);
                return tbl[m];
            }

            static VDef<_Ty> const & v(
                _Ty n)
            {
                size_t b = 0;
                size_t e = sizeof(_Ty) - 1;
                size_t m = (b + e) / 2;
                VDef<_Ty> const (& tbl)[sizeof(_Ty)] = v_tbl<_Ty>().tbl;
                while (m > b || m < e) {
                    if (tbl[m].mask_byte_test & n) {
                        b = m + 1;
                    } else {
                        e = m;
                    }
                    m = (b + e) / 2;
                }
                assert((tbl[m].mask_byte_test & n) == 0);
                return tbl[m];
            }

            friend VariableNumber rotate(
                VariableNumber const & v)
            {
                return VariableNumber(v.v_, BytesOrder::detail::rotate(v.n_));
            }

        private:
            VDef<_Ty> const & v_;
            _Ty n_;
        };

    }
}

#endif // _FRAMEWORK_SYSTEM_VARIABLE_NUMBER_H_

