// LoggerFormatStreambuff.h

#ifndef _FRAMEWORK_LOGGER_LOGGERFORMATSTREAMBUFF_H_
#define _FRAMEWORK_LOGGER_LOGGERFORMATSTREAMBUFF_H_

#include <streambuf>
#include <iostream>

namespace framework
{
    namespace logger
    {
        /// �ⲿ�ṩ������
        class FormatStreambuf 
            : public std::streambuf
        {
        public:
            FormatStreambuf( char * buf, std::size_t bufsize, std::size_t datasize = 0 )
                : msg_buf_( buf )
                , buf_size_( bufsize )
            {
                this->setg( msg_buf_, msg_buf_ + datasize, msg_buf_ + bufsize );
                this->setp( msg_buf_, msg_buf_ + bufsize );
                this->pbump( datasize );
            }

            // ��������Ч�ֽڵĸ���
            std::size_t size()
            {
                return this->pptr() - this->gptr();
            }

            // ȡ����
            char * data()
            {
                msg_buf_[size()] = '\0';
                return msg_buf_;
            }

            virtual ~FormatStreambuf() {}

        protected:
            virtual int sync()
            { 
                rewindPos();
                return 0;
            }

            // �����������
            virtual int_type overflow( int_type _Meta )
            {
                msg_buf_[buf_size_ - 1] = '\0';
                rewindPos();

                return traits_type::eof();
            }

            // ���ö�д����Ϊ��ʼλ��
            void rewindPos()
            {
                this->setg( msg_buf_, msg_buf_, msg_buf_ + buf_size_ );
                this->setp( msg_buf_, msg_buf_ + buf_size_ );
                this->pbump( 0 );
                this->gbump( 0 );
            }

        private:
            char * msg_buf_;        // �����׵�ַ
            std::size_t buf_size_;  // �����С
        };
    } // namespace logger
} // namespace framework

#endif // _FRAMEWORK_LOGGER_LOGGERFORMATSTREAMBUFF_H_
