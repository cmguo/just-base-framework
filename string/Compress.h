// Compress.h

#ifndef _FRAMEWORK_STRING_COMPRESS_H_
#define _FRAMEWORK_STRING_COMPRESS_H_

namespace framework
{
    namespace string
    {

        boost::uint32_t compress(void *in, boost::uint32_t in_len, void *out);
        boost::uint32_t decompress (void *in, boost::uint32_t in_len, void *out);

    } // namespace string
} // namespace framework

#endif // _FRAMEWORK_STRING_COMPRESS_H_
