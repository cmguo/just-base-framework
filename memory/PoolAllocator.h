// PoolAllocator.h

// ��Ӧ��׼���allocator��Ĭ��ʹ�� BigFixedPool

#ifndef _FRAMEWORK_MEMORY_POOL_ALLOCATOR_H_
#define _FRAMEWORK_MEMORY_POOL_ALLOCATOR_H_

#include "framework/memory/MemoryPool.h"
#include "framework/memory/PrivateMemory.h"
#include "framework/memory/BigFixedPool.h"
#include "framework/generic/NativePointer.h"

namespace framework
{
    namespace memory
    {

        template <
            typename _BTy, // base type
            typename _Pl
        >
        inline _Pl & allocator_pool()
        {
            static PrivateMemory _pm;
            static _Pl _pl(_pm);
            return _pl;
        }

        template <
            typename _Ty, 
            typename _BTy = _Ty, // base type
            typename _Pl = framework::memory::BigFixedPool, 
            typename _Pt = framework::generic::NativePointerTraits<_Ty>
        >
        class PoolAllocator
        {
        public:
            typedef _Ty value_type;
            typedef _BTy base_type;

            typedef typename _Pt::pointer pointer;
            typedef typename _Pt::const_pointer const_pointer;
            typedef typename _Pt::reference reference;
            typedef typename _Pt::const_reference const_reference;
            typedef typename _Pt::size_type size_type;
            typedef typename _Pt::difference_type difference_type;

        public:
            PoolAllocator()
            {
            }

            PoolAllocator(
                PoolAllocator const & r)
            {
            }

            ~PoolAllocator()
            {
            }

        public:
            pointer address(reference x) const
            {
                return &x;
            }

            const_pointer address(const_reference x) const
            {
                return &x;
            }

        public:
            pointer allocate(
                size_type n, 
                const_pointer = 0) throw(std::bad_alloc)
            {
                void * p = _pool().alloc(n * sizeof(_Ty));
                if (!p)
                    throw std::bad_alloc();
                return pointer(static_cast<value_type *>(p));
            }

            void deallocate(
                pointer p, 
                size_type n)
            {
                _pool().free(p, n * sizeof(_Ty));
            }

            void deallocate(
                void * p, 
                size_type)
            {
                _pool().free(p);
            }

            size_type max_size() const
            {
                return static_cast<size_type>(-1) / sizeof(value_type);
            }

        public:
            bool operator == (
                const PoolAllocator &) const
            {
                return true;
            }

            bool operator != (
                const PoolAllocator &) const
            {
                return false;
            }

        public:
            void construct(
                pointer p, 
                const value_type& x)
            {
                new(p) value_type(x);
            }

            void destroy(
                pointer p)
            {
                (&*p)->~_Ty();
            }

        public:
            static _Pl const & pool()
            {
                return allocator_pool<_BTy, _Pl>();
            }

            template <typename _Ty1>
            PoolAllocator(
                PoolAllocator<
                    _Ty1, 
                    _BTy, 
                    _Pl, 
                    typename _Pt::template rebind<_Ty1>::type
                > const & r)
            {
            }
			
            template <class _Ty1>
            struct rebind
            {
                typedef typename _Pt::template rebind<_Ty1>::type _Pt1;
                typedef PoolAllocator<_Ty1, _BTy, _Pl, _Pt1> other;
            };

        private:
            void operator = (
                const PoolAllocator &);

            _Pl & _pool()
            {
                return allocator_pool<_BTy, _Pl>();
            }

        };


        template<
            typename _BTy, 
            typename _Pl, 
            typename _Pt
        >
        class PoolAllocator<void, _BTy, _Pl, _Pt>
        {
        public:
            typedef void value_type;

            typedef void * pointer;

            typedef const void * const_pointer;

        public:
            template <class _Ty1>
            struct rebind
            {
                typedef PoolAllocator<
                    _Ty1, 
                    _BTy, 
                    _Pl, 
                    typename _Pt::template rebind<_Ty1>::type> other;
            };
        };

    } // namespace memory
} // namespace framework

#endif // _FRAMEWORK_MEMORY_POOL_ALLOCATOR_H_
