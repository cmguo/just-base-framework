// Version.cpp

#include "framework/Framework.h"
#include "framework/system/ErrorCode.h"
#include "framework/system/Version.h"
#include "framework/string/Format.h"
#include "framework/string/Parse.h"
#include "framework/string/StringToken.h"

#include <boost/asio/streambuf.hpp>
#include <boost/regex.hpp>
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

        class FileFinder
        {
        public:
            FileFinder(
                std::string const & file)
                : fs_(file.c_str(), std::ios::in | std::ios::out | std::ios::binary)
                , pos_(0)
            {
            }

            bool find(
                std::string const & regex, 
                std::string & result)
            {
                boost::regex expr(regex);
                while (!fs_.eof()) {
                    boost::asio::mutable_buffer mbuf = buf_.prepare(4096 - buf_.size());
                    fs_.read(boost::asio::buffer_cast<char *>(mbuf), boost::asio::buffer_size(mbuf));
                    buf_.commit(fs_.gcount());
                    boost::asio::const_buffer cbuf = buf_.data();
                    char const * buf_head = (char const *)boost::asio::buffer_cast<char const *>(cbuf);
                    size_t buf_size = boost::asio::buffer_size(cbuf);
                    boost::match_results<char const *> match_results;
                    if (boost::regex_search(buf_head, buf_head + buf_.size(), match_results, expr,
                        boost::match_default | boost::match_partial)) {
                            if (match_results[0].matched) {
                                // Full match. We're done.
                                result = match_results[0].str();
                                size_t buf_pos = match_results[0].second - buf_head;
                                pos_ += buf_pos;
                                buf_.consume(buf_pos);
                                return true;
                            } else {
                                // Partial match. Next search needs to start from beginning of match.
                                size_t buf_pos = match_results[0].first - buf_head;
                                pos_ += buf_pos;
                                buf_.consume(buf_pos);
                            }
                    } else {
                        // No match. Next search can start with the new data.
                        pos_ += buf_size;
                        buf_.consume(buf_size);
                    }
                }
                return false;
            }

            void read(
                std::string & result)
            {
                size_t size = result.size();
                if (buf_.size() < size) {
                    boost::asio::mutable_buffer mbuf = buf_.prepare(4096 - buf_.size());
                    fs_.read(boost::asio::buffer_cast<char *>(mbuf), boost::asio::buffer_size(mbuf));
                    buf_.commit(fs_.gcount());
                }
                if (buf_.size() < size) {
                    size = buf_.size();
                }
                result.resize(size);
                memcpy(&result[0], boost::asio::buffer_cast<char const *>(buf_.data()), size);
                buf_.consume(size);
                pos_ += size;
            }

            void write(
                std::string const & result)
            {
                size_t size = result.size();
                fs_.seekp(pos_);
                fs_.write(&result[0], size);
                pos_ += size;
                if (buf_.size() < size) {
                    size = buf_.size();
                }
                buf_.consume(size);
            }

            bool fail() const
            {
                return fs_.fail();
            }

            void close()
            {
                return fs_.close();
            }

        private:
            std::fstream fs_;
            boost::asio::streambuf buf_;
            size_t pos_;
        };

        boost::system::error_code Version::get_version(
            std::string const & file, 
            std::string const & module, 
            std::string & version)
        {
            FileFinder fs(file);
            if (fs.fail()) {
                return framework::system::last_system_error();
            }

            std::string version_tag = "!" + module + "_version_tag";
            std::string result;
            if (!fs.find(version_tag, result)) {
                return framework::system::logic_error::item_not_exist;
            }
            version.resize(32);
            fs.read(version);
            version.resize(::strlen(version.c_str()));
            fs.close();

            return boost::system::error_code();
        }

        boost::system::error_code Version::set_version(
            std::string const & file, 
            std::string const & module, 
            std::string const & version)
        {
            FileFinder fs(file);
            if (fs.fail()) {
                return framework::system::last_system_error();
            }

            time_t last_write_time = boost::filesystem::last_write_time(file);

            std::string version_tag = "!" + module + "_version_tag";
            std::string result;
            if (!fs.find(version_tag, result)) {
                return framework::system::logic_error::item_not_exist;
            }
            fs.write(version + '\0'); // write terminate \0
            fs.close();

            boost::filesystem::last_write_time(file, last_write_time);

            return boost::system::error_code();
        }

        boost::system::error_code Version::get_version(
            std::string const & file, 
            std::vector<std::pair<std::string, std::string> > & module_versions)
        {
            FileFinder fs(file);
            if (fs.fail()) {
                return framework::system::last_system_error();
            }

            std::string version_tag = "!\\w+_version_tag";
            std::string result;
            while (fs.find(version_tag, result)) {
                std::string version;
                version.resize(32);
                fs.read(version);
                version.resize(::strlen(version.c_str()));
                module_versions.push_back(std::make_pair(result.substr(1, result.size() - 13), version));
            }

            fs.close();

            return boost::system::error_code();
        }

    } // namespace system
} // namespace boost
