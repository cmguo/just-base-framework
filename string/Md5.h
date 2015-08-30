// Md5.h

#ifndef _FRAMEWORK_STRING_MD5_H_
#define _FRAMEWORK_STRING_MD5_H_

#include <boost/array.hpp>

namespace framework
{
    namespace string
    {

        namespace detail
        {
            struct Md5Ctx;
        }

        class Md5Sum
        {
        public:
            static size_t const output_size = 16; // in bytes

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
                Md5Sum const & l, 
                Md5Sum const & r)
            {
                return l.bytes_ == r.bytes_;
            }

            friend bool operator!=(
                Md5Sum const & l, 
                Md5Sum const & r)
            {
                return !(l == r);
            }

        protected:
            bytes_type bytes_;
        };

        class Md5
            : public Md5Sum
        {
        public:
            static size_t const block_size = 64; // in bytes

        public:
            Md5(
                boost::uint32_t rand = 0);

            ~Md5();

        public:
            void init(
                boost::uint32_t rand = 0);

            void update(
                boost::uint8_t const * buf, 
                size_t len);

            void final();

            boost::uint8_t const * digest() const;

        public:
            static bytes_type apply(
                boost::uint8_t const * buf, 
                size_t len);

        protected:
            detail::Md5Ctx * ctx_;
        };

        inline Md5::bytes_type md5(
            boost::uint8_t const * buf, 
            size_t len)
        {
            return Md5::apply(buf, len);
        }

        inline Md5::bytes_type md5(
            std::string const & data)
        {
            return Md5::apply((boost::uint8_t const *)data.c_str(), data.size());
        }

        inline Md5::bytes_type md5(
            std::vector<boost::uint8_t> const & data)
        {
            return Md5::apply(data.empty() ? NULL : &data.at(0), data.size());
        }

        template <
            size_t N
        >
        inline Md5::bytes_type md5(
            boost::uint8_t const (& data)[N])
        {
            return Md5::apply(data, N);
        }

        template <
            size_t N
        >
        inline Md5::bytes_type md5(
            boost::array<boost::uint8_t, N> const & data)
        {
            return Md5::apply(data.data(), data.size());
        }

    } // namespace string
} // namespace framework

#endif // _FRAMEWORK_STRING_MD5_H_
