// Buffer.h

#ifndef _FRAMEWORK_LOGGER_BUFFER_H_
#define _FRAMEWORK_LOGGER_BUFFER_H_

#include <streambuf>

namespace framework
{
    namespace logger
    {

        /// 外部提供缓冲区
        class Buffer 
            : public std::streambuf
        {
        public:
            Buffer(
                char * buf, 
                size_t len)
            {
                this->setp(buf, buf + len - 1);
            }

            void finish()
            {
                char * p = pptr();
                if (*(p - 1) != '\n') {
                    if (p == epptr()) {
                        *(p - 1) = '\n';
                    } else {
                        *p = '\n';
                        pbump(1);
                    }
                }
                *pptr() = '\0';
            }

            /// 缓冲区有效字节的个数
            std::size_t size() const
            {
                return this->pptr() - this->pbase();
            }

        protected:
            /// 数据溢出调用
            virtual int_type overflow(
                int_type _Meta)
            {
                return traits_type::eof();
            }
        };

    } // namespace logger
} // namespace framework

#endif // _FRAMEWORK_LOGGER_BUFFER_H_
