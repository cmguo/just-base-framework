// Digest.cpp

#ifndef _FRAMEWORK_STRING_DIGEST_HPP_
#define _FRAMEWORK_STRING_DIGEST_HPP_

#include "framework/string/Digest.h"
#include "framework/string/Base16.h"
#include "framework/system/LogicError.h"

namespace framework
{
    namespace string
    {

        template <size_t N>
        std::string Digest<N>::to_string() const
        {
            char const * bytes = (char const *)bytes_.elems;
            return Base16::encode(std::string(bytes, N));
        }

        template <size_t N>
        boost::system::error_code Digest<N>::from_string(
            std::string const & str)
        {
            using namespace framework::system;
            std::string md5 = Base16::decode(str);
            if (md5.size() == N) {
                memcpy((char *)bytes_.elems, md5.c_str(), N);
                return logic_error::succeed;
            } else {
                return logic_error::invalid_argument;
            }
        }

        template <size_t N>
        typename Digest<N>::bytes_type Digest<N>::to_bytes() const
        {
            return bytes_;
        }

        template <size_t N>
        void Digest<N>::from_bytes(
            bytes_type const & bytes)
        {
            bytes_ = bytes;
        }

    } // namespace string
} // namespace framework

#endif // _FRAMEWORK_STRING_DIGEST_HPP_
