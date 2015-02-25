// ParseStl.h

#ifndef _FRAMEWORK_STRING_PARSE_STL_H_
#define _FRAMEWORK_STRING_PARSE_STL_H_

#include "framework/string/Parse.h"
#include "framework/string/Slice.h"
#include "framework/string/StringToken.h"

#include <boost/type_traits/remove_const.hpp>

#include <iterator>

namespace std
{

#ifndef _CONTAINER_DEF_
#define _CONTAINER_DEF_
    template<
        typename _Ct
    >
    struct container_def
    {
        static char const * prefix() { return "{"; };
        static char const * delim() { return ","; };
        static char const * suffix() { return "}"; };
    };
#endif // _CONTAINER_DEF_

    // 解析集合类型的数据
    template<
        typename _Ct
    >
    inline bool from_string_container(
        std::string const & str, 
        _Ct & ct)
    {
        char const * prefix = container_def<_Ct>::prefix();
        char const * delim = container_def<_Ct>::delim();
        char const * suffix = container_def<_Ct>::suffix();
        ct.clear();
        typedef typename _Ct::value_type value_type;
        return !framework::string::slice<value_type>(str, std::inserter(ct, ct.end()), delim, prefix, suffix);
    }

#ifndef _PAIR_DELIM_
#define _PAIR_DELIM_
    template<typename _Ty1, typename _Ty2>
    struct pair_def
    {
        static char const * delim() { return ":"; };
    };
#endif

    template<typename _Kty, typename _Alloc>
    bool from_string(
        std::string const & str, 
        std::vector<_Kty, _Alloc> & v)
    {
        return from_string_container(str, v);
    }

    template<typename _Kty, typename _Alloc>
    bool from_string(
        std::string const & str, 
        std::list<_Kty, _Alloc> & v)
    {
        return from_string_container(str, v);
    }

    template<typename _Kty, typename _Pr, typename _Alloc>
    bool from_string(
        std::string const & str, 
        std::set<_Kty, _Pr, _Alloc> & v)
    {
        return from_string_container(str, v);
    }

    template<typename _Kty, typename _Ty, typename _Pr, typename _Alloc>
    bool from_string(
        std::string const & str, 
        std::map<_Kty, _Ty, _Pr, _Alloc> & v)
    {
        return from_string_container(str, v);
    }

    template<typename _Ty1, typename _Ty2>
    bool from_string(
        std::string const & str, 
        std::pair<_Ty1, _Ty2> & v)
    {
        framework::string::StringToken st(str.substr(1, str.size() - 2), ",");
        std::string str1;
        std::string str2;
        typedef typename boost::remove_const<_Ty1>::type typef;
        boost::system::error_code ec;
        return !st.next_token(str1, ec)
            && from_string(str1, const_cast<typef &>(v.first))
            && from_string(st.remain(), v.second);
    }

} // namespace std

#ifndef FRAMEWORK_STRING_CONTAINER_DEFINE
#define FRAMEWORK_STRING_CONTAINER_DEFINE(_container, _prefix, _delim, _suffix) \
    namespace std { \
    template<> \
struct container_def<_container::iterator> \
{ \
    static char const * prefix() { return _prefix; }; \
    static char const * delim() { return _delim; }; \
    static char const * suffix() { return _suffix; }; \
}; \
    template<>  \
struct container_def<_container::const_iterator> \
{ \
    static char const * prefix() { return _prefix; }; \
    static char const * delim() { return _delim; }; \
    static char const * suffix() { return _suffix; }; \
}; \
}
#endif // CONTAINER_DEFINE

#ifndef FRAMEWORK_STRING_PAIR_DEFINE
#define FRAMEWORK_STRING_PAIR_DEFINE(_Ty1, _Ty2, _delim) \
    namespace std { \
    template<> \
struct pair_def<_Ty1, _Ty2> \
{ \
    static char const * delim() { return _delim; }; \
}; \
}
#endif // PAIR_DEFINE

#endif // _FRAMEWORK_STRING_PARSE_STL_H_
