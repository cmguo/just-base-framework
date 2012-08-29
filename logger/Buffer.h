// Buffer.h

#ifndef _FRAMEWORK_LOGGER_BUFFER_H_
#define _FRAMEWORK_LOGGER_BUFFER_H_

#include <streambuf>

namespace framework
{
    namespace logger
    {

        /// �ⲿ�ṩ������
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

            /// ��������Ч�ֽڵĸ���
            std::size_t size() const
            {
                return this->pptr() - this->pbase();
            }

        protected:
            /// �����������
            virtual int_type overflow(
                int_type _Meta)
            {
                return traits_type::eof();
            }
        };

    } // namespace logger
} // namespace framework

#endif // _FRAMEWORK_LOGGER_BUFFER_H_
