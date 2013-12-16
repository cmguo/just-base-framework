// ConvertSim.h

namespace framework
{
    namespace string
    {

        bool Convert::open(
            std::string const & to, 
            std::string const & from, 
            boost::system::error_code & ec)
        {
            ec = not_supported;
            return false;
        }

        bool Convert::convert(
            std::string const & s1, 
            std::string & s2, 
            boost::system::error_code & ec)
        {
            ec = not_supported;
            return false;
        }

        void Convert::close()
        {
        }

    } // namespace string
} // namespace framework
