// FormatRecord.cpp

#include "framework/Framework.h"
#include "framework/logger/FormatRecord.h"

namespace framework
{
    namespace logger
    {

        static char const chex[] = "0123456789ABCDEF";

        void FormatFormator::next_format()
        {
            assert(fmt_);
            for (char const * p = fmt_; *p; ++p) {
                if (*p == '%') {
                    char const * q = p + 1;
                    bool all_digit = *q != '%';
                    while ((*q < 'A' || *q > 'Z') && (*q < 'a' || *q > 'z') && *q != '%') {
                        all_digit = all_digit && (*q >='0' && *q <= '9');
                        ++q;
                    }
                    os_.write(fmt_, p - fmt_);
                    if (*q == '%' && !all_digit) { // 适应 %xx% 和 %N% 这样的形式，通过 all_digit 区分
                        os_.put('%');
                        p = q;
                        fmt_ = q + 1;
                    } else {
                        fmt_ = q + 1;
                        break;
                    }
                }
            }
        }

    } // namespace logger
} // namespace framework
