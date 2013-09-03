// FileTag.cpp

#include "framework/Framework.h"
#include "framework/system/FileTag.h"
#include "framework/system/ErrorCode.h"

#include <boost/regex.hpp>
#include <boost/filesystem/operations.hpp>

#include <string.h>

namespace framework
{
    namespace system
    {

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

            void modify(
                std::string const & result)
            {
                size_t size = result.size();
                fs_.seekp(pos_ - size);
                fs_.write(&result[0], size);
                fs_.seekp(pos_ + buf_.size());
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

        FileTag::FileTag(
            std::string const & file, 
            std::string const & tag)
            : file_(file)
            , tag_(tag)
        {
        }

        boost::system::error_code FileTag::get(
            std::string const & item, 
            std::string & value)
        {
            FileFinder fs(file_);
            if (fs.fail()) {
                return framework::system::last_system_error();
            }

            std::string tag = "!" + item + "\\|" + tag_ + "\\|tag[^\\x00]*\\x00 *!";
            std::string result;
            if (!fs.find(tag, result)) {
                return framework::system::logic_error::item_not_exist;
            }
            value = result.c_str() + 1 + item.size() + 1 + tag_.size() + 4;

            fs.close();

            return boost::system::error_code();
        }

        boost::system::error_code FileTag::set(
            std::string const & item, 
            std::string & value)
        {
            FileFinder fs(file_);
            if (fs.fail()) {
                return framework::system::last_system_error();
            }

            time_t last_write_time = boost::filesystem::last_write_time(file_);

            std::string tag = "!" + item + "\\|" + tag_ + "\\|tag[^\\x00]*\\x00 *!";
            std::string result;
            if (!fs.find(tag, result)) {
                return framework::system::logic_error::item_not_exist;
            }
            if (1 + item.size() + 1 + tag_.size() + 4 + value.size() + 2 > result.size()) {
                value = value + " [BUFFER EXCEED]";
            } else {
                char * old = &result.at(0) + 1 + item.size() + 1 + tag_.size() + 4;
                std::string ovalue = old;
                memset(old, ' ', ovalue.size() + 1);
                strcpy(old, value.c_str());
                fs.modify(result);
                value = value + " [OK " + ovalue + "]";
            }

            fs.close();

            boost::filesystem::last_write_time(file_, last_write_time);

            return boost::system::error_code();
        }

        boost::system::error_code FileTag::get_all(
            std::map<std::string, std::string> & values)
        {
            FileFinder fs(file_);
            if (fs.fail()) {
                return framework::system::last_system_error();
            }

            bool all = values.empty();

            std::string tag = "!\\w+\\|" + tag_ + "\\|tag[^\\x00]*\\x00 *!";
            std::string result;
            while (fs.find(tag, result)) {
                std::string item = result.substr(1, result.find('|') - 1);
                std::map<std::string, std::string>::iterator iter = values.find(item);
                if (all && iter == values.end())
                    iter = values.insert(std::make_pair(item, "")).first;
                if (iter != values.end()) {
                    iter->second = result.c_str() + 1 + item.size() + 1 + tag_.size() + 4;
                }
            }

            fs.close();

            return boost::system::error_code();
        }

        boost::system::error_code FileTag::set_all(
            std::map<std::string, std::string> & values)
        {
            FileFinder fs(file_);
            if (fs.fail()) {
                return framework::system::last_system_error();
            }

            std::string tag = "!\\w+\\|" + tag_ + "\\|tag[^\\x00]*\\x00 *!";
            std::string result;
            while (fs.find(tag, result)) {
                std::string item = result.substr(1, result.find('|') - 1);
                std::map<std::string, std::string>::iterator iter = values.find(item);
                if (iter != values.end()) {
                    std::string & value = iter->second;
                    if (1 + item.size() + 1 + tag_.size() + 4 + value.size() + 2 > result.size()) {
                        value = value + " [BUFFER EXCEED]";
                    } else {
                        char * old = &result.at(0) + 1 + item.size() + 1 + tag_.size() + 4;
                        std::string ovalue = old;
                        memset(old, ' ', ovalue.size() + 1);
                        strcpy(old, value.c_str());
                        fs.modify(result);
                        value = value + " [OK " + ovalue + "]";
                    }
                }
            }

            fs.close();

            return boost::system::error_code();
        }

    } // namespace system
} // namespace boost
