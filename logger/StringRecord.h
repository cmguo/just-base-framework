// StringRecord.h

#ifndef _FRAMEWORK_LOGGER_STRING_RECORD_H_
#define _FRAMEWORK_LOGGER_STRING_RECORD_H_

#include "framework/logger/Record.h"

namespace framework
{
    namespace logger
    {

        class StringRecord
            : public Record
        {
        public:
            StringRecord(
                char const * title, 
                char const * str, 
                size_t size = 0)
                : Record(format_message)
                , title_(title)
                , str_(str)
                , size_(size)
            {
            }

            StringRecord(
                char const * title, 
                std::string const & str, 
                size_t size = size_t(-1))
                : Record(format_message)
                , title_(title)
                , str_(str.c_str())
                , size_(str.size())
            {
                if (size_ > size)
                    size_ = size;
            }

        private:
            static size_t format_message(
                Record const & base, 
                char * buf, 
                size_t len)
            {
                StringRecord const & me = 
                    static_cast<StringRecord const &>(base);

                char const * title = me.title_;
                char const * str = me.str_;
                size_t size = me.size_;

                Buffer sbuf(buf, len);
                std::ostream os(&sbuf);
                os << title << " [string: s" << size << " bytes]\n";
                sbuf.finish();

                me.set_other_buffer(str, size);

                return sbuf.size();
            }

        private:
            char const * title_;
            char const * str_;
            size_t size_;
        };

    } // namespace logger
} // namespace framework

#define LOG_STR(level, param) \
    framework::logger::log(_slogm(), level, framework::logger::StringRecord param)

#endif // _FRAMEWORK_LOGGER_STRING_RECORD_H_
