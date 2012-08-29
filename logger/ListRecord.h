// LoggerListRecord.h

#ifndef _FRAMEWORK_LOGGER_LIST_RECORD_H_
#define _FRAMEWORK_LOGGER_LIST_RECORD_H_

#include "framework/logger/Record.h"

#include <boost/preprocessor/seq/for_each.hpp>

#include <ostream>

namespace framework
{
    namespace logger
    {

        template <
            typename _Front, 
            typename _Back>
        class LoggerLists;

        class LoggerListsBeg
        {
        public:
            template <
                typename Back
            >
            LoggerLists<LoggerListsBeg, Back> const operator()(
                Back const & nvp) const;

            template <
                typename Back
            >
            LoggerLists<LoggerListsBeg, Back> const operator<<(
                Back const & nvp) const
            {
                return (*this).operator()<Back>(nvp);
            }

        public:
            void format(
                std::string & str) const
            {
                str += "(";
            }
        };

        template <
            typename _Ty
        >
        class Nvp
            : public ParamRef<_Ty>
        {
        public:
            Nvp(
                char const * name, 
                _Ty const & value)
                : ParamRef<_Ty>(value)
                , name_(name)
            {
            }

        public:
            char const * name() const
            {
                return name_;
            }

        private:
            char const * name_;
        };

        class ListFormator
        {
        public:
            ListFormator(
                std::streambuf * buf)
                : os_(buf)
                , c_('(')
            {
            }

            ~ListFormator()
            {
                if (c_ == '(') {
                    os_.write(&c_, 1);
                }
                c_ = ')';
                os_.write(&c_, 1);
            }

            template <
                typename _Ty
            >
            void operator()(
                Nvp<_Ty> const & t)
            {
                os_.write(&c_, 1);
                os_ << t.name();
                c_ = '=';
                os_.write(&c_, 1);
                c_ = ',';
                os_ << t.value();
            }

        private:
            std::ostream os_;
            char c_;   /// ¸ñÊ½»¯´®
        };

        template <
            typename _Ty
        >
        Nvp<_Ty> const make_nvp(
            char const * name, 
            _Ty const & value)
        {
            return Nvp<_Ty>(name, value);
        }

    } // namespace logger
} // namespace framework

#define LOG_STRINGLIZE(s) #s

#define LOG_NVP(v) \
    framework::logger::make_nvp(LOG_STRINGLIZE(v), v)

#define LOG_PARAM(z, d, p) << LOG_NVP(p)

#define LOG_L(level, params) LOG_(level, ListFormator, , BOOST_PP_SEQ_FOR_EACH(LOG_PARAM, _, params))

#ifdef LOG
#  undef LOG
#endif
#define LOG LOG_L

#include "framework/logger/Macro.h"

#endif // _FRAMEWORK_LOGGER_LIST_RECORD_H_
