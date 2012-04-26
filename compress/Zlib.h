// Zlib.h

#ifndef _FRAMEWORK_COMPRESS_ZLIB_H_
#define _FRAMEWORK_COMPRESS_ZLIB_H_

namespace framework
{
    namespace compress
    {
        class ZLib
        {
        public:
            static int gzunzip(
                unsigned char * compressed, size_t com_len, unsigned char *uncompressed, size_t *uncom_len);
        };
    }// namespace compress
}// namespace framework

#endif // _FRAMEWORK_COMPRESS_ZLIB_H_