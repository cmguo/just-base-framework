// IntrusivePointer.h

#ifndef _FRAMEWORK_GENERIC_INTRUSIVE_POINTER_H_
#define _FRAMEWORK_GENERIC_INTRUSIVE_POINTER_H_

#include <boost/intrusive_ptr.hpp>

namespace framework
{
    namespace generic
    {

        template <typename _Ty>
        struct IntrusivePointerTraits
        {
            typedef _Ty value_type;

            typedef boost::intrusive_ptr<_Ty> pointer;

            typedef boost::intrusive_ptr<_Ty const> const_pointer;

            typedef value_type & reference;

            typedef const value_type & const_reference;

            typedef size_t size_type;

            typedef ptrdiff_t difference_type;

            template <typename _Ty1>
            struct rebind
            {
                typedef IntrusivePointerTraits<_Ty1> type;
            };
        };

    } // namespace generic
} // namespace framework

#endif // _FRAMEWORK_GENERIC_INTRUSIVE_POINTER_H_
