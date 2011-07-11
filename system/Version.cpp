// Version.cpp

#include "framework/Framework.h"
#include "framework/system/ErrorCode.h"
#include "framework/system/Version.h"
#include "framework/string/Format.h"
#include "framework/string/Parse.h"
#include "framework/string/StringToken.h"

#include <boost/asio/streambuf.hpp>
#include <boost/filesystem/operations.hpp>

#include <fstream>
#include <string.h>

using namespace framework::string;

namespace framework
{
    namespace system
    {

        Version::Version(
            boost::uint8_t major, 
            boost::uint8_t minor, 
            boost::uint16_t revision, 
            boost::uint32_t build)
        {
            boost::uint8_t * ch = (boost::uint8_t *)&value;
            ch[0] = major;
            ch[1] = minor;
            ch[2] = (boost::uint8_t)((revision >> 8) & 0x00ff);
            ch[3] = (boost::uint8_t)((revision      ) & 0x00ff);
            ch[4] = (boost::uint8_t)((build >> 24) & 0x000000ff);
            ch[5] = (boost::uint8_t)((build >> 16) & 0x000000ff);
            ch[6] = (boost::uint8_t)((build >>  8) & 0x000000ff);
            ch[7] = (boost::uint8_t)((build      ) & 0x0000000ff);
        }

        Version::Version(
            std::string const & str)
        {
            from_string(str);
        }

        boost::system::error_code Version::from_string(
            std::string const & str)
        {
            StringToken st(str, ".");
            std::string major_str;
            std::string minor_str;
            std::string revision_str;
            std::string build_str;
            boost::uint8_t major = 0;
            boost::uint8_t minor = 0;
            boost::uint16_t revision = 0;
            boost::uint16_t build = 0;
            boost::system::error_code ec;
            if (st.next_token(major_str, ec) || 
                st.next_token(minor_str, ec) || 
                (ec = parse2(major_str, major)) || 
                (ec = parse2(minor_str, minor))) {
                    return ec;
            }
            if (!st.next_token(revision_str, ec)) {
                ec = parse2(revision_str, revision);
            }
            if (!ec) {
                ec = parse2(st.remain(), build);
            }
            if (ec == framework::system::logic_error::no_more_item) {
                ec = boost::system::error_code();
            }
            if (!ec) {
                *this = Version(major, minor, revision, build);
            }
            return ec;
        }

        std::string Version::to_string() const
        {
            boost::uint8_t const * ch = (boost::uint8_t const *)&value;
            return format(ch[0]) 
                + "." + format(ch[1]) 
                + "." + format(
                    (((boost::uint16_t)ch[2]) << 8) 
                    | (boost::uint16_t)ch[3])
                + "." + format(
                    (((boost::uint32_t)ch[4]) << 24) 
                    | ((boost::uint32_t)ch[5] << 16)
                    | ((boost::uint32_t)ch[6] << 8)
                    | ((boost::uint32_t)ch[7]));
        }

        std::string Version::to_simple_string() const
        {
            boost::uint8_t const * ch = (boost::uint8_t const *)&value;
            return format(ch[0]) 
                + "." + format(ch[1]) 
                + "." + format(
                    (((boost::uint16_t)ch[2]) << 8) 
                    | (boost::uint16_t)ch[3]);
        }

        boost::system::error_code Version::get_version(
            std::string const & file, 
            std::string const & module, 
            std::string & version)
        {
            return file_version(file, module, true, version);
        }

        boost::system::error_code Version::set_version(
            std::string const & file, 
            std::string const & module, 
            std::string const & version)
        {
            std::string version2 = version;
            return file_version(file, module, false, version2);
        }

        boost::system::error_code Version::file_version(
            std::string const & file, 
            std::string const & module, 
            bool get_or_set, 
            std::string & version)
        {
            std::fstream fs(file.c_str(), std::ios::in | std::ios::out | std::ios::binary);
            if (fs.fail()) {
                return framework::system::last_system_error();
            }

            time_t last_write_time = boost::filesystem::last_write_time(file);

            std::string version_tag = "!" + module + "_version_tag";

            boost::asio::streambuf buf;
            size_t pos = 0;

            while (!fs.eof()) {
                boost::asio::mutable_buffer mbuf = buf.prepare(4096);
                fs.read(boost::asio::buffer_cast<char *>(mbuf), boost::asio::buffer_size(mbuf));
                buf.commit(fs.gcount());
                boost::asio::const_buffer cbuf = buf.data();
                char const * buf_head = (char const *)boost::asio::buffer_cast<char const *>(cbuf);
                size_t buf_size = boost::asio::buffer_size(cbuf);
                char const * p = (char const *)memchr(buf_head, version_tag[0], buf_size);
                while (p) {
                    size_t buf_left = buf_size - (p - buf_head);
                    if (strncmp(p, version_tag.c_str(), buf_left > version_tag.size() ? version_tag.size() : buf_left) == 0) {
                        break;
                    } else {
                        ++p;
                        --buf_left;
                        p = (char const *)memchr(p, version_tag[0], buf_left);
                    }
                }
                if (p) {
                    size_t buf_pos = (p - buf_head);
                    pos += buf_pos;
                    buf.consume(buf_pos);
                    if (buf_size - buf_pos >= version_tag.size()) {
                        fs.clear();
                        break;
                    }
                } else {
                    pos += buf_size;
                    buf.consume(buf_size);
                }
            }

            if (fs.eof()) {
                return framework::system::logic_error::item_not_exist;
            }

            size_t offset = pos + version_tag.size();

            if (get_or_set) {
                fs.seekg(offset);
                version.resize(32);
                fs.read(&version[0], 32);
                version.resize(strlen(version.c_str()));
            } else {
                fs.seekp(offset);
                fs.write(&version[0], version.size() + 1); // write terminate \0
            }

            fs.close();

            boost::filesystem::last_write_time(file, last_write_time);

            return boost::system::error_code();
        }

    } // namespace system
} // namespace boost
