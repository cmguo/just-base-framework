// Convert.h

/** Convert用于字符编码转换
由于不同的实现方式，在linux下能够支持普遍的编码格式，在Windows下只支持
unicode、acp、tacp、utf8、gbk
*/

#ifndef _FRAMEWORK_STRING_CONVERT_H_
#define _FRAMEWORK_STRING_CONVERT_H_

namespace framework
{
    namespace string
    {

        class Convert
        {
        public:
            Convert();

            // 构造一定转换器，指定源编码格式和目标编码格式
            Convert(
                std::string const & to_code, 
                std::string const & from_code);

            ~Convert();

        public:
            bool open(
                std::string const & to_code, 
                std::string const & from_code, 
                boost::system::error_code & ec);

            // 使用转换器转换字符编码
            bool convert(
                std::string const & from, 
                std::string & to, 
                boost::system::error_code & ec);

            void close();

            std::size_t err_pos() const
            {
                return err_pos_;
            }

        private:
            void * ctx_;
            std::size_t err_pos_;
        };

    } // namespace string
} // namespace framework

#endif // _FRAMEWORK_STRING_CONVERT_H_
