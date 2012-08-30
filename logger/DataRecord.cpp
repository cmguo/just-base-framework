// DataRecord.cpp

#include "framework/Framework.h"
#include "framework/logger/DataRecord.h"
#include "framework/logger/Buffer.h"

namespace framework
{
    namespace logger
    {

        static char const chex[] = "0123456789ABCDEF";

        size_t DataRecord::format_message(
            Record const & base, 
            char * buf, 
            size_t len)
        {
            const size_t line_num_len = 4;
            const size_t line_num_len2 = line_num_len + 2;
            const size_t hex_len = 16 * 3 + 4;
            const size_t text_start = line_num_len2 + hex_len;
            const size_t text_len = 16 + 2;
            const size_t total_len = text_start + text_len + 1;

            DataRecord const & me = 
                static_cast<DataRecord const &>(base);

            char const * title = me.title_;
            unsigned char const * data = me.data_;
            size_t size = me.size_;

            Buffer sbuf(buf, len);
            std::ostream os(&sbuf);
            os << title << " [data: s" << size << " bytes]\n";
            sbuf.finish();

            char * p(buf + sbuf.size());
            char * e(buf + len);
            char * q = p;

            for (size_t i = 0; i < size; ++i, ++data) {
                if ((i % 16) == 0) {
                    // ÐÐºÅ
                    if (q + total_len > e)
                        break;
                    p = q;
                    q = p + text_start;
                    size_t n = i >> 4;
                    *p++ = '[';
                    for (size_t ii = line_num_len - 1; ii != (size_t)-1; --ii) {
                        *p++ = chex[n & 0x0F];
                        n >>= 4;
                    }
                    *p++ = ']';
                    *q++ = '|';
                }
                if ((i & 0x03) == 0) {
                    *p++ = ' ';
                }
                unsigned char c = *data;
                *p++ = chex[c >> 4];
                *p++ = chex[c & 0x0F];
                *p++ = ' ';
                *q++ = isgraph(c) ? c : '.';
                if ((i % 16) == 15) {
                    *q++ = '|';
                    *q++ = '\n';
                }
            }

            size = size & 0x0F;
            if (size != 0) {
                for (size_t i = size; i < 16; ++i) {
                    if ((i & 0x03) == 0) {
                        *p++ = ' ';
                    }
                    *p++ = ' ';
                    *p++ = ' ';
                    *p++ = ' ';
                    *q++ = ' ';
                }
                *q++ = '|';
                *q++ = '\n';
            }

            return q - buf;
        }

    } // namespace logger
} // namespace framework
