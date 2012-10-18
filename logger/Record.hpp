// Record.hpp

#ifndef _FRAMEWORK_LOGGER_RECORD_HPP_
#define _FRAMEWORK_LOGGER_RECORD_HPP_

#include "framework/logger/Record.h"
#include "framework/logger/Buffer.h"
#include "framework/logger/Param.h"

namespace framework
{
    namespace logger
    {

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
    if (framework::logger::log_accept(_slogm(), level)) \
        framework::logger::log(_slogm(), level, framework::logger::logger_record<formator>( \
            framework::logger::ParamsBegin() operator params))

#endif // _FRAMEWORK_LOGGER_RECORD_HPP_
