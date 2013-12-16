// ConvertLinux.h

#include <sstream>

#include <errno.h>
#include <iconv.h>

namespace framework
{
    namespace string
    {

        bool Convert::open(
            std::string const & t0, 
            std::string const & from, 
            boost::system::error_code & ec)
        {
            iconv_t cd = iconv_open(t1.c_str(), t2.c_str());
            if (cd == (iconv_t)-1) {
                ec = framework::system::last_system_error();
                return false;
            }
            ctx_ = new iconv_t(cd);
            ec.clear();
            return true;
        }

        bool Convert::convert(
            std::string const & s1, 
            std::string & s2, 
            boost::system::error_code & ec)
        {
            if (ctx_ == NULL) {
                ec = invalid_argument;
                return false;
            }
            iconv_t cd = *(iconv_t *)ctx_;
            std::ostringstream oss;
            char buf[1024];
            char *inbuf = (char*)s1.c_str();
            size_t inbytesleft = s1.size();
            char *outbuf = buf;
            size_t outbytesleft = sizeof(buf);
            size_t ret = iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
            while (ret == (size_t)-1) {
                if (errno != E2BIG) {
                    err_pos_ = inbuf - s1.c_str();
                    ec = framework::system::last_system_error();
                    break;
                }
                oss.write(buf, outbuf - buf);
                outbuf = buf;
                outbytesleft = sizeof(buf);
                ret = iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
            }
            if (ret != (size_t)-1) {
                oss.write(buf, outbuf - buf);
                s2 = oss.str();
                ec.clear();
                return true;
            }
            return false;
        }

        void Convert::close()
        {
            if (ctx_)
                delete (iconv_t *)ctx_; 
                ctx_ = NULL;
            }
        }

    } // namespace string
} // namespace framework
