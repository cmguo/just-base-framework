// Parse.h

/** 从格式化字符串解析出各种数据类型的函数模板
*/

#ifndef _FRAMEWORK_STRING_PARSE_H_
#define _FRAMEWORK_STRING_PARSE_H_

#include <boost/type_traits/is_pointer.hpp>
#include <boost/type_traits/is_array.hpp>
#include <boost/type_traits/is_enum.hpp>
#include <boost/type_traits/is_fundamental.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/mpl/if.hpp>

#include <framework/system/LogicError.h>

#include <sstream>
#include <functional>
#include <iterator>

namespace std
{

    inline bool from_string(
        std::string const & str, 
        std::string & v)
    {
        v = str;
        return true;
    }

}

namespace framework
{
    namespace string
    {

        struct from_string_pointer;
        struct from_string_enum;
        struct from_string_non_pointer;

        template <typename T>
        inline bool from_string(
            std::string const & str, 
            T & v)
        {
            typedef typename boost::mpl::if_<
                boost::is_pointer<T>, 
                from_string_pointer, 
                BOOST_DEDUCED_TYPENAME boost::mpl::if_<
                boost::is_enum<T>, 
                from_string_enum, 
                from_string_non_pointer
                >::type
            >::type invoke_type;
            return invoke_type::invoke(str, v);
        }

        template <typename T>
        inline T from_string(
            std::string const & str)
        {
            T v = T();
            from_string(str, v);
            return v;
        }

        struct from_string_pointer
        {
            template<typename T>
            static bool invoke(
                std::string const & str, 
                T const * & v)
            {
                T * t = new T;
                if (from_string(str, *t)) {
                    v = t;
                    return true;
                }
                return false;
            }

            template<typename T>
            static bool invoke(
                std::string const & str, 
                T const & v)
            {
                typedef typename T::value_type value_type;
                value_type * t = new value_type;
                if (from_string(str, *t)) {
                    v = t;
                    return true;
                }
                return false;
            }

            static bool invoke(
                std::string const & str, 
                void const * & v)
            {
                std::istringstream iss(str);
                void * v1;
                iss >> v1;
                if (iss.good()) {
                    v = v1;
                }
                return !!iss;
            }
        };

        struct from_string_enum
        {
            template<typename T>
            static bool invoke(
                std::string const & str, 
                T & v)
            {
                int i;
                if (from_string(str, i)) {
                    v = static_cast<T>(i);
                    return true;
                }
                return false;
            }
        };

        struct from_string_non_pointer
        {
            /// 处理基本类型序列化
            struct from_string_primitive
            {
                template<typename T>
                static bool invoke(
                    std::string const & str, 
                    T & v)
                {
                    std::istringstream iss(str);
                    iss >> v;
                    return !!iss;
                }

                static bool invoke(
                    std::string const & str, 
                    bool & v)
                {
                    v = str == "1" || str == "true";
                    return true;
                }

                static bool invoke(
                    std::string const & str, 
                    unsigned char & v)
                {
                    int i;
                    if (from_string(str, i)) {
                        v = static_cast<unsigned char>(i);
                        return true;
                    }
                    return false;
                }
            };

            /// 处理标准类型（非基本类型）序列化
            struct from_string_standard
            {
                static bool check(
                    bool result)
                {
                    return result;
                }

                static bool check(
                    boost::system::error_code const & ec)
                {
                    return !ec;
                }

                template<typename T>
                static bool invoke(
                    std::string const & str, 
                    T & v)
                {
                    return check(v.from_string(str));
                }
            };

            template<typename T>
            static bool invoke(
                std::string const & str, 
                T & v)
            {
                /// 根据类型类别（基本类型，标准类型），分别处理序列化
                typedef typename boost::mpl::if_<
                    boost::is_fundamental<T>, 
                    from_string_primitive, 
                    from_string_standard
                >::type invoke_type;
                return invoke_type::invoke(str, v);
            }
        };

        template <typename T>
        inline T parse(
            std::string const & str)
        {
            T v = T();
            from_string(str, v);
            return v;
        }

        template <typename T>
        inline boost::system::error_code parse2(
            std::string const & str, 
            T & v)
        {
            boost::system::error_code ec;
            if (!from_string(str, v)) {
                ec = framework::system::logic_error::invalid_argument;
            }
            return ec;
        }

    } // namespace string
} // namespace framework

#endif // _FRAMEWORK_STRING_PARSE_H_
