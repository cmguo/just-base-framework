// Record.h

#ifndef _FRAMEWORK_LOGGER_RECORD_H_
#define _FRAMEWORK_LOGGER_RECORD_H_

namespace framework
{
    namespace logger
    {

        class Record
        {
        protected:
            typedef size_t (*format_message_type)(
                Record const & rec, 
                char * buf, 
                size_t len);

            Record(
                format_message_type format_message)
                : format_message_(format_message)
                , other_buf_(NULL)
                , other_len_(0)
            {
            }

        public:
            size_t format_message(
                char * buf, 
                size_t len) const
            {
                return format_message_(*this, buf, len);
            }

            char const * other_buffer(
                size_t & size) const
            {
                size = other_len_;
                return other_buf_;
            }

        protected:
            void set_other_buffer(
                char const * buf, 
                size_t len) const
            {
                other_buf_ = buf;
                other_len_ = len;
            }

        private:
            format_message_type format_message_;
            mutable char const * other_buf_;
            mutable size_t other_len_;
        };


    } // namespace logger
} // namespace framework

#endif // _FRAMEWORK_LOGGER_RECORD_H_
