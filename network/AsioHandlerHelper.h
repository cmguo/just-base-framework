// AsioHandlerHelper.h

#ifndef _FRAMEWORK_NETWORK_ASIO_HANDLER_HELPER_H_
#define _FRAMEWORK_NETWORK_ASIO_HANDLER_HELPER_H_

#define PASS_DOWN_ASIO_HANDLER_FUNCTION(type, member) \
 \
inline friend void* asio_memberallocate( \
    std::size_t size, \
    type * this_handler) \
{ \
    return boost_asio_memberalloc_helpers::allocate( \
        size, &this_handler->member); \
} \
 \
inline friend void asio_memberdeallocate( \
    void * pointer, \
    std::size_t size, \
    type * this_handler) \
{ \
    boost_asio_memberalloc_helpers::deallocate( \
        pointer, size, &this_handler->member); \
} \
 \
template <typename Function> \
inline friend void asio_memberinvoke( \
                                       const Function & function, \
                                       type * this_handler) \
{ \
    boost_asio_memberinvoke_helpers::invoke( \
        function, &this_handler->member); \
} \

#endif // _FRAMEWORK_NETWORK_ASIO_HANDLER_HELPER_H_
