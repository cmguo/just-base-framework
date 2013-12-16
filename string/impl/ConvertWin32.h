// ConvertWin32.h

#include <windows.h>

namespace framework
{
    namespace string
    {

        bool Convert::open(
            std::string const & to, 
            std::string const & from, 
            boost::system::error_code & ec)
        {
            static std::map<std::string, DWORD> name_cp;
            if (name_cp.empty()) {
                name_cp["unicode"] = 0;
                name_cp["acp"] = CP_ACP;
                name_cp["tacp"] = CP_THREAD_ACP;
                name_cp["utf8"] = CP_UTF8;
                name_cp["utf-8"] = CP_UTF8;
                name_cp["gbk"] = 936;
            }
            DWORD to_cp = name_cp[to];
            DWORD from_cp = name_cp[from];
            if (to_cp && from_cp) {
                DWORD * dw = new DWORD[2];
                dw[0] = to_cp;
                dw[1] = from_cp;
                ctx_ = dw;
                ec.clear();
                return true;
            } else {
                ec = invalid_argument;
                return false;
            }
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
            DWORD * dw = (DWORD *)ctx_;
            std::basic_string<WCHAR> unicode;
            int n1 = MultiByteToWideChar(dw[1], 0, &s1[0], s1.length(), NULL, 0);
            if (n1 > 0) {
                unicode.reserve(n1);
                MultiByteToWideChar(dw[1], 0, &s1[0], s1.length(), &unicode[0], n1);
                int n2 = WideCharToMultiByte(dw[0], 0, &unicode[0], n1, NULL, 0, NULL, NULL);
                if (n2 > 0) {
                    s2.resize(n2);
                    WideCharToMultiByte(dw[0], 0, &unicode[0], n1, &s2[0], n2, NULL, NULL);
                    ec.clear();
                    return true;
                }
            }
            ec = framework::system::last_system_error();
            return false;
        }

        void Convert::close()
        {
            if (ctx_) {
                delete [] (DWORD *)ctx_; 
                ctx_ = NULL;
            }
        }

    } // namespace string
} // namespace framework
