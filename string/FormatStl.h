// FormatStl.h

#ifndef _FRAMEWORK_STRING_FORMAT_STL_H_
#define _FRAMEWORK_STRING_FORMAT_STL_H_

#include "framework/string/Join.h"

namespace std
{

#ifndef _CONTAINER_DEF_
#define _CONTAINER_DEF_
    template<typename _It>
    struct container_def
    {
        static char const * prefix() { return "{"; };
        static char const * delim() { return ","; };
        static char const * suffix() { return "}"; };
    };
#endif // _RANGE_DEF_

    // 格式化集合类型的数据
    template<typename _It>
    std::string to_string_container(
        _It first, 
        _It last)
    {
        char const * prefix = container_def<_It>::prefix();
        char const * delim = container_def<_It>::delim();
        char const * suffix = container_def<_It>::suffix();
        return framework::string::join(first, last, delim, prefix, suffix);
    }
    
#ifndef _PAIR_DELIM_
#define _PAIR_DELIM_
    template<typename _Ty1, typename _Ty2>
    struct pair_def
    {
        static char const * delim() { return ":"; };
    };
#endif

    template<typename _Ty, typename _Ax>
    std::string to_string(
        std::vector<_Ty, _Ax> const & v)
    {
        return to_string_container(v.begin(), v.end());
    }

    template<typename _Ty, typename _Ax>
    std::string to_string(
        std::deque<_Ty, _Ax> const & v)
    {
        return to_string_container(v.begin(), v.end());
    }

    template<typename _Ty, typename _Ax>
    std::string to_string(
        std::list<_Ty, _Ax> const & v)
    {
        return to_string_container(v.begin(), v.end());
    }

    template<typename _Kty, typename _Pr, typename _Ax>
    std::string to_string(
        std::set<_Kty, _Pr, _Ax> const & v)
    {
        return to_string_container(v.begin(), v.end());
    }

    template<typename _Kty, typename _Ty, typename _Pr, typename _Ax>
    std::string to_string(
        std::map<_Kty, _Ty, _Pr, _Ax> const & v)
    {
        return to_string_container(v.begin(), v.end());
    }

    template<typename _Ty1, typename _Ty2>
    std::string to_string(
        std::pair<_Ty1, _Ty2> const & v)
    {
        using framework::string::to_string;
        return to_string(v.first) + pair_def<_Ty1, _Ty2>::delim() + to_string(v.second);
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

#endif // _FRAMEWORK_STRING_FORMAT_STL_H_
