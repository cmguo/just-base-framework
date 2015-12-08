// HumanNumber.hpp

#include <framework/system/HumanNumber.h>

#include <sstream>

namespace framework
{
    namespace system
    {

        template <
            typename _Ty
        >
        bool HumanNumber<_Ty>::from_string(
            std::string const & str)
        {
            std::istringstream iss(str);
            _Ty n;
            iss >> n;
            if (!iss || iss.rdbuf()->in_avail() > 2) {
                return false;
            }
            char const * cc = "kKmMgGtT";
            char c = 0, b = 0;
            if (iss >> c ) {
                iss >> b;
            }
            char const * p = strchr(cc, c);
            if (p) {
                c = char(p - cc) / 2 + 1;
            } else if (b) {
                return false;
            } else {
                b = c;
                c = 0;
            }
            if (b == 'B') {
                n_ = n << (c * 10);
            } else if (b == 0) {
                _Ty m[5] = {_Ty(1), _Ty(1000), _Ty(1000000), _Ty(1000000000), _Ty(1000000000000)};
                n_ = n * m[(int)c];
            } else {
                return false;
            }
            return true;
        }

        template <
            typename _Ty
        >
        std::string HumanNumber<_Ty>::to_string() const
        {
            if (n_ == 0) {
                return "0";
            }
            _Ty n = n_;
            std::ostringstream oss;
            if (n < _Ty(0)) {
                n = -n;
                oss << '-';
            }
            int c = 0;
            while ((n & 0x3ff) == 0) {
                n >>= 10;
                ++c;
            }
            char const * s = "KMGT";
            if (c) {
                oss << n;
                oss << s[(int)c -  1];
                oss << 'B';
                return oss.str();
            }
            while ((n % 1000) == 0) {
                n /= 1000;
                ++c;
            }
            oss << n;
            if (c) {
                oss << s[(int)c -  1];
            }
            return oss.str();
        }

    }
}
