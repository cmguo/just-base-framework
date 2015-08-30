// Md5.cpp

#include "framework/Framework.h"
#include "framework/string/Md5.h"
#include "framework/string/Base16.h"
#include "framework/system/LogicError.h"
using namespace framework::system::logic_error;

using namespace boost::system;

namespace framework
{
    namespace string
    {

        // include to namespace framework::string::detail
        namespace detail
        {
#include "framework/string/detail/md5.h"
            struct Md5Ctx
                : MD5_CTX
            {
            };
        }

        Md5::Md5(
            boost::uint32_t rand)
        {
            ctx_ = new detail::Md5Ctx;
            init(rand);
        }

        Md5::~Md5()
        {
            if (ctx_)
                delete ctx_;
        }

        void Md5::init(
            boost::uint32_t rand)
        {
            detail::MD5Init(ctx_, rand);
        }

        void Md5::update(
            boost::uint8_t const * buf, 
            size_t len)
        {
            detail::MD5Update(ctx_, (unsigned char *)buf, len);
        }

        void Md5::final()
        {
            detail::MD5Final(ctx_);
            memcpy(bytes_.elems, ctx_->digest, 16);
        }

        boost::uint8_t const * Md5::digest() const
        {
            return bytes_.elems;
        }

        Md5::bytes_type Md5::apply(
            boost::uint8_t const * buf, 
            size_t len)
        {
            Md5 sha;
            sha.update(buf, len);
            sha.final();
            return sha.to_bytes();
        }

        std::string Md5Sum::to_string() const
        {
            char const * bytes = (char const *)bytes_.elems;
            return Base16::encode(std::string(bytes, 16));
        }

        error_code Md5Sum::from_string(
            std::string const & str)
        {
            std::string md5 = Base16::decode(str);
            if (md5.size() == 16) {
                memcpy((char *)bytes_.elems, md5.c_str(), 16);
                return succeed;
            } else {
                return invalid_argument;
            }
        }

        Md5::bytes_type Md5Sum::to_bytes() const
        {
            return bytes_;
        }

        void Md5Sum::from_bytes(
            bytes_type const & bytes)
        {
            bytes_ = bytes;
        }

    } // namespace string
} // namespace framework

