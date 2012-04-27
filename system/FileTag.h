// Version.cpp

#include <boost/asio/streambuf.hpp>
#include <boost/regex.hpp>

#include <fstream>

namespace framework
{
    namespace system
    {

        class FileFinder
        {
        public:
            FileFinder(
                std::string const & file);

            bool find(
                std::string const & regex, 
                std::string & result);

            void read(
                std::string & result);

            void write(
                std::string const & result);

            bool fail() const;

            void close();

        private:
            std::fstream fs_;
            boost::asio::streambuf buf_;
            size_t pos_;
        };

        class FileTag
        { 
        public:
            FileTag(
                std::string const & file, 
                std::string const & tag);

        public:
            boost::system::error_code get(
                std::string const & item, 
                std::string & value);

            boost::system::error_code set(
                std::string const & item, 
                std::string const & value);

            boost::system::error_code get_all(
                std::vector<std::pair<std::string, std::string> > & values);

        private:
            std::string const file_;
            std::string const tag_;
        };

    } // namespace system
} // namespace boost
