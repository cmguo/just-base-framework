// Stream.h

#ifndef _FRAMEWORK_LOGGER_STREAM_H_ 
#define _FRAMEWORK_LOGGER_STREAM_H_

#include "framework/container/Array.h"

#include <boost/asio/buffer.hpp>

namespace framework
{

    namespace configure
    {
        class ConfigModule;
    }

    namespace logger
    {

        class Manager;
        class Group;

        /// 输出流接口定义
        class Stream
        {
        public:
            typedef boost::asio::const_buffer buffer_t;

            typedef framework::container::Array<buffer_t> buffers_t;

        public:
            static bool register_(
                std::string const & type, 
                Stream * (* creator)());

            static Stream * create(
                framework::configure::ConfigModule & cm);

        public:
            virtual void write( 
                buffers_t const & buffers) = 0;

        protected:
            Stream();

            virtual ~Stream() {}

            virtual void load_config(
                framework::configure::ConfigModule & cm);

        protected:
            friend class Manager;
            friend class Group;

            Stream * next_;       /// 下一个流
            std::string name_;   /// 名称
            size_t level_;             /// 日志等级
        };

        template <
            typename T
        >
        static Stream * create()
        {
            return new T;
        }

        struct StreamRegister
        {
            StreamRegister(
                std::string const & type, 
                Stream * (* creator)())
            {
                Stream::register_(type, creator);
            }
        };

    } //namespace logger
} // namespace framework

#define LOG_REG_STREAM_TYPE(type, class) \
    static StreamRegister reg_stream_type ## type(#type, create<class>);

#endif // _FRAMEWORK_LOGGER_STREAM_H_
