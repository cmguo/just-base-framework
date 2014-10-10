// Format.h

/** 将各种类型的数据格式化为字符串的函数模板和类模板
*/

#ifndef _FRAMEWORK_STRING_FORMAT_H_
#define _FRAMEWORK_STRING_FORMAT_H_

#include <boost/type_traits/is_pointer.hpp>
#include <boost/type_traits/is_array.hpp>
#include <boost/type_traits/is_enum.hpp>
#include <boost/type_traits/is_fundamental.hpp>
#include <boost/mpl/if.hpp>

#include <framework/system/LogicError.h>

#include <sstream>
#include <functional>

namespace std
{

    inline std::string to_string(
        std::string const & v)
    {
        return v;
    }

}

namespace framework
{
    namespace string
    {

        // 函数模板，可以不要指定数据类型，编译器自动决定
        inline std::string operator + (
            char const * left, 
            std::string const & right)
        {
            return std::string(left) + right;
        }

        struct to_string_pointer;
        struct to_string_enum;
        struct to_string_non_pointer;

        template <typename T>
        inline std::string to_string(
            T const & v)
        {
            typedef typename boost::mpl::if_<
                boost::is_pointer<T>, 
                to_string_pointer, 
                BOOST_DEDUCED_TYPENAME boost::mpl::if_<
                    boost::is_enum<T>, 
                    to_string_enum, 
                    to_string_non_pointer
                >::type
            >::type invoke_type;
            return invoke_type::invoke(v);
        }
        
        template <typename T>
        inline void to_string(
            std::string & str, 
            T const & v)
        {
            str = to_string(v);
        }

        struct to_string_pointer
        {
            template<typename T>
            static std::string invoke(
                T const & v)
            {
                return to_string(*v);
            }

            static std::string invoke(
                void const * v)
            {
                std::ostringstream oss;
                oss << v;
                return oss.str();
            }
        };

        struct to_string_enum
        {
            template<typename T>
            static std::string invoke(
                T const & v)
            {
                const int i = static_cast<int>(v);
                return to_string(i);
            }
        };

        struct to_string_non_pointer
        {
            /// 处理基本类型序列化
            struct to_string_primitive
            {
                template<typename T>
                static std::string invoke(
                    T const & v)
                {
                    std::ostringstream oss;
                    oss << v;
                    return oss.str();
                }

                static std::string invoke(
                    unsigned char const & v)
                {
                    return to_string((int)v);
                }

                static std::string invoke(
                    bool const & v)
                {
                    return v ? "true" : "false";
                }
            };

            /// 处理标准类型（非基本类型）序列化
            struct to_string_standard
            {
                template<typename T>
                static std::string invoke(
                    T const & v)
                {
                    return v.to_string();
                }
            };

            template<typename T>
            static std::string invoke(
                T const & v)
            {
                /// 根据类型类别（基本类型，标准类型），分别处理序列化
                typedef typename boost::mpl::if_<
                    boost::is_fundamental<T>, 
                    to_string_primitive, 
                    to_string_standard
                >::type invoke_type;
                return invoke_type::invoke(v);
            }
        }; // struct to_string_non_pointer

        template <typename T>
        inline std::string format(
            T const & v)
        {
            return to_string(v);
        }

        template <typename T>
        inline boost::system::error_code format2(
            std::string & str, 
            T const & v)
        {
            str = to_string(v);
            return boost::system::error_code();
        }

    } // namespace string
} // namespace framework

#endif // _FRAMEWORK_STRING_FORMAT_H_
