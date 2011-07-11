// BytesOrder.h

#ifndef _FRAMEWORK_SYSTEM_BYTES_ORDER_H_
#define _FRAMEWORK_SYSTEM_BYTES_ORDER_H_

#include <boost/detail/endian.hpp>

namespace framework
{
    namespace system
    {

        namespace BytesOrder
        {

            namespace detail
            {

                inline boost::uint16_t rotate(
                    boost::uint16_t v)
                {
                    return v >> 8 | v << 8;
                }

                inline boost::uint32_t rotate(
                    boost::uint32_t v)
                {
                    // 1 2 3 4
                    // 2 3 4 1
                    // 4 1 2 3
                    return (((v >> 24 | v << 8) & 0x00ff00ff) 
                        | ((v >> 8 | v << 24) & 0xff00ff00));
                }

                inline boost::uint64_t rotate(
                    boost::uint64_t v)
                {
                    // 1 2 3 4 5 6 7 8
                    // 2 3 4 5 6 7 8 1
                    // 4 5 6 7 8 1 2 3
                    // 6 7 8 1 2 3 4 5
                    // 8 1 2 3 4 5 6 7
                    return (((v >> 56 | v << 8) & 0x000000ff000000ffULL) 
                        | ((v >> 40 | v << 24) & 0x0000ff000000ff00ULL)
                        | ((v >> 24 | v << 40) & 0x00ff000000ff0000ULL)
                        | ((v >> 8 | v << 56) & 0xff000000ff000000ULL));
                }

                template <
                    size_t _Sz
                >
                struct call_rotate;

                template <
                >
                struct call_rotate<2>
                {
                    static inline boost::uint16_t invoke(
                        boost::uint16_t v)
                    {
                        return rotate(v);
                    }
                };

                template <
                >
                struct call_rotate<4>
                {
                    static inline boost::uint32_t invoke(
                        boost::uint32_t v)
                    {
                        return rotate(v);
                    }
                };

                template <
                >
                struct call_rotate<8>
                {
                    static inline boost::uint64_t invoke(
                        boost::uint64_t v)
                    {
                        return rotate(v);
                    }
                };

            }

#if (defined BOOST_BIG_ENDIAN)

            template <
                typename _Ty
            >
            inline _Ty host_to_big_endian(
                _Ty v)
            {
                return v;
            }

            template <
                typename _Ty
            >
            inline _Ty big_endian_to_host(
                _Ty v)
            {
                return v;
            }

            template <
                typename _Ty
            >
            inline _Ty host_to_little_endian(
                _Ty v)
            {
                return detail::call_rotate<sizeof(v)>::invoke(v);
            }

            template <
                typename _Ty
            >
            inline _Ty little_endian_to_host(
                _Ty v)
            {
                return detail::call_rotate<sizeof(v)>::invoke(v);
            }

#elif (defined BOOST_LITTLE_ENDIAN)

            template <
                typename _Ty
            >
            inline _Ty host_to_big_endian(
                _Ty v)
            {
                return detail::call_rotate<sizeof(v)>::invoke(v);
            }

            template <
                typename _Ty
            >
            inline _Ty big_endian_to_host(
                _Ty v)
            {
                return detail::call_rotate<sizeof(v)>::invoke(v);
            }

            template <
                typename _Ty
            >
            inline _Ty host_to_little_endian(
                _Ty v)
            {
                return v;
            }

            template <
                typename _Ty
            >
            inline _Ty little_endian_to_host(
                _Ty v)
            {
                return v;
            }

#else

            template <
                typename _Ty
            >
            inline _Ty host_to_big_endian(
                _Ty v)
            {
                union {
                    boost::uint8_t ch[sizeof(_Ty)];
                    _Ty res;
                } u;
                for (size_t i = 0; i < sizeof(_Ty); ++i) {
                    u.ch[i] = (boost::uint8_t)(v >> ((sizeof(_Ty) - i - 1) * 8));
                }
                return u.res;
            }

            template <
                typename _Ty
            >
            inline _Ty big_endian_to_host(_Ty v)
            {
                boost::uint8_t * ch = (boost::uint8_t *)&v;
                _Ty res = 0;
                for (size_t i = 0; i < sizeof(_Ty); ++i) {
                    res |= ch[i] << ((sizeof(_Ty) - i - 1) * 8);
                }
                return res;
            }

            template <
                typename _Ty
            >
            inline _Ty host_to_little_endian(_Ty v)
            {
                union {
                    boost::uint8_t ch[sizeof(_Ty)];
                    _Ty res;
                } u;
                for (size_t i = 0; i < sizeof(_Ty); ++i) {
                    u.ch[i] = (boost::uint8_t)(v >> (i * 8));
                }
                return u.res;
            }

            template <
                typename _Ty
            >
            inline _Ty little_endian_to_host(_Ty v)
            {
                boost::uint8_t * ch = (boost::uint8_t *)&v;
                _Ty res = 0;
                for (size_t i = 0; i < sizeof(_Ty); ++i) {
                    res |= ch[i] << (i * 8);
                }
                return res;
            }

#endif

            // host <--> big_endian

            inline boost::uint16_t host_to_big_endian_short(
                boost::uint16_t v)
            {
                return host_to_big_endian(v);
            }

            inline boost::uint32_t host_to_big_endian_long(
                boost::uint32_t v)
            {
                return host_to_big_endian(v);
            }

            inline boost::uint64_t host_to_big_endian_longlong(
                boost::uint64_t v)
            {
                return host_to_big_endian(v);
            }

            inline boost::uint16_t big_endian_to_host_short(
                boost::uint16_t v)
            {
                return big_endian_to_host(v);
            }

            inline boost::uint32_t big_endian_to_host_long(
                boost::uint32_t v)
            {
                return big_endian_to_host(v);
            }

            inline boost::uint64_t big_endian_to_host_longlong(
                boost::uint64_t v)
            {
                return big_endian_to_host(v);
            }

            // host <--> little_endian

            inline boost::uint16_t host_to_little_endian_short(
                boost::uint16_t v)
            {
                return host_to_little_endian(v);
            }

            inline boost::uint32_t host_to_little_endian_long(
                boost::uint32_t v)
            {
                return host_to_little_endian(v);
            }

            inline boost::uint64_t host_to_little_endian_longlong(
                boost::uint64_t v)
            {
                return host_to_little_endian(v);
            }

            inline boost::uint16_t little_endian_to_host_short(
                boost::uint16_t v)
            {
                return little_endian_to_host(v);
            }

            inline boost::uint32_t little_endian_to_host_long(
                boost::uint32_t v)
            {
                return little_endian_to_host(v);
            }

            inline boost::uint64_t little_endian_to_host_longlong(
                boost::uint64_t v)
            {
                return little_endian_to_host(v);
            }

            // host <--> net

            inline boost::uint16_t net_to_host_short(
                boost::uint16_t v)
            {
                return big_endian_to_host_short(v);
            }

            inline boost::uint32_t net_to_host_long(
                boost::uint32_t v)
            {
                return big_endian_to_host_long(v);
            }

            inline boost::uint64_t net_to_host_longlong(
                boost::uint64_t v)
            {
                return big_endian_to_host_longlong(v);
            }

            inline boost::uint16_t host_to_net_short(
                boost::uint16_t v)
            {
                return host_to_big_endian_short(v);
            }

            inline boost::uint32_t host_to_net_long(
                boost::uint32_t v)
            {
                return host_to_big_endian_long(v);
            }

            inline boost::uint64_t host_to_net_longlong(
                boost::uint64_t v)
            {
                return host_to_big_endian_longlong(v);
            }

        }

    } // namespace system
} // namespace boost

#endif // _FRAMEWORK_SYSTEM_BYTES_ORDER_H_
