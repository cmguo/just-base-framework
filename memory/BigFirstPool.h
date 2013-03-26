// BigFirstPool.h

#ifndef _FRAMEWORK_MEMORY_BIG_FIRST_POOL_H_
#define _FRAMEWORK_MEMORY_BIG_FIRST_POOL_H_

#include "framework/container/Ordered.h"
#include "framework/container/OrderedUnidirList.h"
#include "framework/memory/MemoryPool.h"

namespace framework
{
    namespace memory
    {

        class BigFirstPool
            : public MemoryPool
        {
        public:
            BigFirstPool(
                BlockMemory & bm, 
                size_t capacity = (size_t)-1)
                : MemoryPool(bm, capacity)
            {
            }

            template <
                typename _Mm
            >
            BigFirstPool(
                _Mm const & mm, 
                size_t capacity = (size_t)-1)
                : MemoryPool(mm, capacity)
            {
            }

            BigFirstPool(
                BigFirstPool const & r, 
                AllocatorBind *)
                : MemoryPool(r)
            {
            }

            ~BigFirstPool()
            {
                clear();
            }

        private:
            struct Object;

            struct Block
                : framework::container::OrderedHook<Block>::type
            {
                size_t size;
                size_t size_used;

                Block(
                    size_t size)
                    : size(size)
                    , size_used(0)
                {
                }

                bool operator < (
                    Block const & r) const
                {
                    return this < &r;
                }

                bool full()
                {
                    return size_used == 0;
                }

                Object * first()
                {
                    return reinterpret_cast<Object *>(this + 1);
                }

                Object * last()
                {
                    return reinterpret_cast<Object *>(
                        reinterpret_cast<char *>(this) + size);
                }
            };

            struct ObjectHead
            {
                ObjectHead(
                    Block * blk, 
                    size_t size)
                    : blk(blk)
                    , size(size)
                {
                }

                Block * blk;
                size_t size; // 整个内存块的大小
            };

            struct Object
                : MemoryPool::Object<ObjectHead>
                , framework::container::OrderedHook<Object>::type
            {
                Object(
                    Block * blk, 
                    size_t size)
                    : MemoryPool::Object<ObjectHead>(blk, size)
                {
                }

                Object * successive()
                {
                    return reinterpret_cast<Object *>(
                        reinterpret_cast<char *>(this) + size);
                }

                void * data()
                {
                    return (char *)(this) + size_head;
                }

                void * to_data()
                {
                    assert(!is_used());
                    use();
                    static_cast<hook_type *>(this)->~hook_type();
                    return (char *)(this) + size_head;
                }

                void from_data()
                {
                    assert(is_used());
                    new (data()) hook_type;
                    unusemark();
                }

                static Object * from_data(
                    void * ptr)
                {
                    Object * obj = (Object *)((char *)ptr - size_head);
                    obj->from_data();
                    return obj;
                }

                static Object const * from_data(
                    void const * ptr)
                {
                    return (Object const *)((char const *)ptr - size_head);
                }

                void use()
                {
                    blk = (Block *)((unsigned long)blk | 1);
                }

                void unuse()
                {
                    blk = (Block *)((unsigned long)blk & -2);
                }

                bool is_used()
                {
                    return ((unsigned long)blk & 1) != 0;
                }

                void mark()
                {
                    blk = (Block *)((unsigned long)blk | 2);
                }

                void unmark()
                {
                    blk = (Block *)((unsigned long)blk & -3);
                }

                bool is_marked()
                {
                    return ((unsigned long)blk & 2) != 0;
                }

                void unusemark()
                {
                    blk = (Block *)((unsigned long)blk & -4);
                }

                bool operator < (
                    Object const & r) const
                {
                    return size > r.size;
                }

                BOOST_STATIC_CONSTANT(size_t, size_head = 
                    MemoryPage::static_align_object<sizeof(MemoryPool::Object<ObjectHead>)>::value);
            };

        public:
            void * alloc(
                size_t size);

            void free(
                void * ptr);

            size_t query_size(
                void const * ptr);

            void mark();

            void leak();

        private:
            void attach_block(
                void * ptr, 
                size_t size);

            void detach_block(
                Block * blk);

            void clear();

            void check();

        private:
            typedef framework::container::Ordered<
                Object, 
                framework::container::identity<Object>, 
                std::less<Object>, 
                framework::container::ordered_non_unique_tag
            > FreeObjectSet;

            typedef framework::container::Ordered<
                Block
            > BlockSet;

        private:
            BOOST_STATIC_CONSTANT(size_t, size_head = Object::size_head);

        private:
            size_t size_used_;
            BlockSet blocks_;
            FreeObjectSet free_objs_;
        };

    } // namespace memory
} // namespace framework

#endif // _FRAMEWORK_MEMORY_BIG_FIRST_POOL_H_
