// Record.h

#ifndef _FRAMEWORK_LOGGER_RECORD_H_
#define _FRAMEWORK_LOGGER_RECORD_H_

#include "framework/logger/Buffer.h"
#include "framework/logger/Param.h"

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

        template <
            typename Formator, 
            typename Params
        > 
        class RecordT
            : private Params
            , public Record
        {
        public:
            RecordT(
                Params const & params)
                : Params(params)
                , Record(format_message)
            {
            }

        private:
            static size_t format_message(
                Record const & base, 
                char * buf, 
                size_t len)
            {
                RecordT const & me = 
                    static_cast<RecordT const &>(base);

                Buffer sbuf(buf, len);
                {
                    Formator fmt(&sbuf);
                    me.format(fmt);
                }
                sbuf.finish();
                return sbuf.size();
            }
        };

        template <
            typename Formator, 
            typename Params
        >
        static inline RecordT<Formator, Params> const logger_record(
            Params const & params)
        {
            return RecordT<Formator, Params>(params);
        }

    } // namespace logger
} // namespace framework

#define LOG_(level, formator, operator, params) \
    framework::logger::log(_slogm(), level, framework::logger::logger_record<formator>( \
        framework::logger::ParamsBegin() operator params))

#endif // _FRAMEWORK_LOGGER_RECORD_H_
