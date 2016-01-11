// BigFirstPool.cpp

#include "framework/Framework.h"
#include "framework/memory/BigFirstPool.h"

namespace framework
{
    namespace memory
    {

        void * BigFirstPool::alloc(
            size_t size)
        {
            size = MemoryPage::align_object(size);
            size += size_head;
            if (size < sizeof(Object)) {
                size = sizeof(Object);
            }
            if (free_objs_.empty() || free_objs_.begin()->size < size) {
                size_t block_size = MemoryPage::align_page(size + sizeof(Block));
                if (consumption() + block_size > capacity())
                    return NULL;
                void * ptr = MemoryPool::alloc_block(block_size);
                if (ptr == NULL)
                    return NULL;
                attach_block(ptr, block_size);
                check();
            }
            Object * obj = free_objs_.first();
            Block * blk = obj->blk;
            free_objs_.erase(obj);
            size_used_ += obj->size;
            //check();
            size_used_ -= obj->size;
            assert(obj->size >= size);
            // 如果剩余的大小还能构成一个Object，创建一个新Object
            if (obj->size - size > sizeof(Object)) {
                obj->size -= size;
                assert((int)obj->size > (int)sizeof(Object));
                free_objs_.insert(obj);
                obj = obj->successive();
                new (obj) Object(blk, size);
            }
            size_used_ += obj->size;
            blk->size_used += obj->size;
            MemoryPool::alloc_object(obj);
            void * ptr = obj->to_data();
            check();
            return ptr;
        }

        void BigFirstPool::free(
            void * ptr)
        {
            // ignore NULL
            if (ptr == NULL)
                return;

            Object * obj = Object::from_data(ptr);

            assert(!obj->is_used());

            Block * blk = obj->blk;
            blk->size_used -= obj->size;

            //check();

            size_used_ -= obj->size;
            MemoryPool::free_object(obj);

            Object * obj1 = blk->first();
            for (; obj1 < obj; ) {
                if (obj1->successive() == obj) {
                    if (!obj1->is_used()) {
                        free_objs_.erase(obj1);
                        obj1->size += obj->size;
                        obj->~Object();
                        obj = obj1;
                    }
                    break;
                }
                obj1 = obj1->successive();
            }

            obj1 = obj->successive();
            if (obj1 < blk->last() && !obj1->is_used()) {
                free_objs_.erase(obj1);
                obj->size += obj1->size;
                obj1->~Object();
            }

            free_objs_.insert(obj);

            check();

            if (blk->full() && size_used_ + blk->size + MemoryPage::page_size() < consumption()) {
                detach_block(blk);
                MemoryPool::free_block(blk, blk->size);
            }

            check();
        }

        size_t BigFirstPool::query_size(
            void const * ptr)
        {
            Object const * obj = Object::from_data(ptr);
            return obj->size - sizeof(Object);
        }

        void BigFirstPool::mark()
        {
            for (Block * blk = blocks_.first(); blk; blk = blocks_.next(blk)) {
                if (!blk->full()) {
                    Object * obj_beg = blk->first();
                    Object * obj_end = blk->last();
                    for (Object * obj = obj_beg; obj < obj_end; ) {
                        if (obj->is_used()) {
                            obj->mark();
                        }
                        obj = obj->successive();
                    }
                }
            }
        }

        void BigFirstPool::leak()
        {
            for (Block * blk = blocks_.first(); blk; blk = blocks_.next(blk)) {
                if (!blk->full()) {
                    Object * obj_beg = blk->first();
                    Object * obj_end = blk->last();
                    for (Object * obj = obj_beg; obj < obj_end; ) {
                        if (obj->is_used() && !obj->is_marked()) {
                            leak_object(obj, obj->data(), obj->size - size_head);
                        }
                        obj = obj->successive();
                    }
                }
            }
        }

        void BigFirstPool::attach_block(
            void * ptr, 
            size_t size)
        {
            Block * blk = new (ptr) Block(size);
            blocks_.insert(blk);
            size_used_ += sizeof(Block);
            Object * obj = new (blk + 1) Object(blk, size - sizeof(Block));
            free_objs_.insert(free_objs_.begin(), obj);
        }

        void BigFirstPool::detach_block(
            Block * blk)
        {
            Object * obj = (Object *)(blk + 1);
            assert(obj->blk == blk && obj->size == blk->size - sizeof(Block));
            free_objs_.erase(obj);
            obj->~Object();
            blocks_.erase(blk);
            blk->~Block();
            size_used_ -= sizeof(Block);
        }

        void BigFirstPool::clear()
        {
            while (Block * blk = blocks_.first()) {
                if (!blk->full()) {
                    Object * obj_beg = blk->first();
                    Object * obj_end = blk->last();
                    for (Object * obj = obj_beg; obj < obj_end; ) {
                        if (obj->is_used()) {
                            leak_object(obj, obj->data(), obj->size - size_head);
                            obj->from_data();
                            blk->size_used -= obj->size;
                        } else {
                            free_objs_.erase(obj);
                        }
                        assert(obj->blk == blk);
                        Object * obj1 = obj;
                        obj = obj->successive();
                        if (obj1 != obj_beg) {
                            obj1->~Object();
                        }
                    }
                    // for detach_block to erase
                    free_objs_.insert(new (obj_beg) Object(blk, blk->size - sizeof(Block)));
                }
                detach_block(blk);
                free_block(blk, blk->size);
            }
            assert(free_objs_.empty());
        }

        void BigFirstPool::check()
        {
#if (defined DEBUG && 0)
            size_t size = size_used_;
            size_t size2 = 0;
            for (Object * obj = free_objs_.first(); obj; obj = free_objs_.next(obj)) {
                assert(obj->!is_used());
                assert((int)obj->size >= (int)sizeof(Object));
                size += obj->size;
                //std::cout << obj << " " << obj->size << std::endl;
            }
            //std::cout << std::endl;
            for (Block * blk = blocks_.first(); blk; blk = blocks_.next(blk)) {
                Object * obj_beg = blk->first();
                Object * obj_end = blk->last();
                size_t size3 = sizeof(Block);
                size_t size4 = sizeof(Block);
                for (Object * obj = obj_beg; obj < obj_end; obj = obj->successive()) {
                    if (obj->!is_used()) {
                        assert(free_objs_.exist(obj));
                        size3 += obj->size;
                    }
                    size4 += obj->size;
                }
                assert(size3 == blk->size - blk->size_used);
                assert(size4 == blk->size);
                size2 += blk->size;
            }
            assert(size == size2);
#endif
        }

    } // namespace memory
} // namespace framework
