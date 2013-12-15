// BackTraceSim.h

// simulation of back trace

#define WORD_WIDTH 16

namespace framework
{
    namespace debuging
    {

        bool back_trace_init()
        {
            return false;
        }
        
        size_t back_trace(
            void ** addrs, 
            size_t num)
        {
            return 0;
        }
        
        char ** back_trace_symbols(
            void *const * addrs, 
            size_t num)
        {
            size_t cnt;
            size_t total = 0;
            char **result;
        
            /* We can compute the text size needed for the symbols since we print
            them all as "[+0x<addr>]".  */
            total = num * (WORD_WIDTH + 6);
        
            /* Allocate memory for the result.  */
            result = (char **)malloc(num * sizeof (char *) + total);
            if (result != NULL)
            {
                char *last = (char *) (result + num);
        
                for (cnt = 0; cnt < num; ++cnt)
                {
                    result[cnt] = last;
                    last += 1 + sprintf(last, "[+%p]", addrs[cnt]);
                }
            }
        
            return result;
        }
        
        void release_symbols(
            char ** ptr)
        {
            ::free(ptr);
        }
        
        void back_trace_symbols_fd(
            void *const * addrs, 
            size_t num, 
            int fd)
        {
        }

    } // namespace memory
} // namespace framework
