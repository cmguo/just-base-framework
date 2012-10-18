// DataRecord.h

#ifndef _FRAMEWORK_LOGGER_DATA_RECORD_H_
#define _FRAMEWORK_LOGGER_DATA_RECORD_H_

#include "framework/logger/Record.h"

#include <boost/asio/buffer.hpp>

namespace framework
{
    namespace logger
    {

        class DataRecord
            : public Record
        {
        public:
            DataRecord(
                char const * title, 
                unsigned char const * data, 
                size_t size)
                : Record(format_message)
                , title_(title)
                , data_(data)
                , size_(size)
            {
            }

            DataRecord(
                char const * title, 
                boost::asio::const_buffer const & data, 
                size_t size = size_t(-1))
                : Record(format_message)
                , title_(title)
                , data_(boost::asio::buffer_cast<unsigned char const *>(data))
                , size_(boost::asio::buffer_size(data))
            {
                if (size_ > size)
                    size_ = size;
            }

        private:
            static size_t format_message(
                Record const & base, 
                char * buf, 
                size_t len);

        private:
            char const * title_;
            unsigned char const * data_;
            size_t size_;
        };

    } // namespace logger
} // namespace framework

#define LOG_DATA(level, param) \
    if (framework::logger::log_accept(_slogm(), level)) \
        framework::logger::log(_slogm(), level, framework::logger::DataRecord param)

#endif // _FRAMEWORK_LOGGER_DATA_RECORD_H_
