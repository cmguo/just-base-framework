// Digest.h

#ifndef _FRAMEWORK_STRING_DIGEST_H_
#define _FRAMEWORK_STRING_DIGEST_H_

#include <boost/array.hpp>

namespace framework
{
    namespace string
    {

        template <size_t N>
        class Digest
        {
        public:
            static size_t const output_size = N; // in bytes

            typedef boost::array<boost::uint8_t, output_size> bytes_type;

        public:
            bytes_type to_bytes() const;

            void from_bytes(
                bytes_type const & str);

        public:
            std::string to_string() const;

            boost::system::error_code from_string(
                std::string const & str);

            friend bool operator==(
                Digest const & l, 
                Digest const & r)
            {
                return l.bytes_ == r.bytes_;
            }

            friend bool operator!=(
                Digest const & l, 
                Digest const & r)
            {
                return !(l == r);
            }

        protected:
            bytes_type bytes_;
        };

    } // namespace string
} // namespace framework

#endif // _FRAMEWORK_STRING_DIGEST_H_
