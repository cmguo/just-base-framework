// PrivateMemory.h

#ifndef _FRAMEWORK_MEMORY_PRIVATE_MEMORY_H_
#define _FRAMEWORK_MEMORY_PRIVATE_MEMORY_H_

#include "framework/memory/MemoryPage.h"

namespace framework
{
    namespace memory
    {

        class PrivateMemory
            : public MemoryPage
        {
        public:
            static size_t min_block_size()
            {
                return page_size();
            }

        public:
            void * alloc_block(
                size_t size);

            void free_block(
                void * addr, 
                size_t size);
        };

    } // namespace memory
} // namespace framework

#endif // _FRAMEWORK_MEMORY_PRIVATE_MEMORY_H_
