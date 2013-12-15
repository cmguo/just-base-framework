// Convert.h

/** Convert�����ַ�����ת��
���ڲ�ͬ��ʵ�ַ�ʽ����linux���ܹ�֧���ձ�ı����ʽ����Windows��ֻ֧��
unicode��acp��tacp��utf8��gbk
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

            // ����һ��ת������ָ��Դ�����ʽ��Ŀ������ʽ
            Convert(
                std::string const & to_code, 
                std::string const & from_code);

            ~Convert();

        public:
            bool open(
                std::string const & to_code, 
                std::string const & from_code, 
                boost::system::error_code & ec);

            // ʹ��ת����ת���ַ�����
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
