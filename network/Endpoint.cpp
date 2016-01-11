// Endpoint.cpp

#include "framework/Framework.h"
#include "framework/network/Endpoint.h"
#include "framework/network/NetName.h"
#include "framework/string/Format.h"
#include "framework/string/Parse.h"
using namespace framework::system::logic_error;
using namespace framework::string;

using namespace boost::system;

namespace framework
{
    namespace network
    {

        Endpoint::Endpoint(
            std::string const & ip_str, 
            boost::uint16_t port)
            : protocol_(unspec_protocol)
            , port_(port)
        {
            from_string(ip_str);
        }

        Endpoint::Endpoint(
            ProtocolEnum protocol, 
            std::string const & ip_str, 
            boost::uint16_t port)
            : protocol_(protocol)
            , port_(port)
        {
            from_string(ip_str);
        }

        Endpoint::Endpoint(
            boost::asio::ip::udp::endpoint const & end_point)
            : protocol_(udp)
            , port_(end_point.port())
        {
            ip(end_point.address());
        }

        Endpoint::Endpoint(
            boost::asio::ip::tcp::endpoint const & end_point)
            : protocol_(tcp)
            , port_(end_point.port())
        {
            ip(end_point.address());
        }

        Endpoint::operator boost::asio::ip::udp::endpoint() const
        {
            assert(protocol() == unspec_protocol || protocol() == udp);
            return boost::asio::ip::udp::endpoint(ip(), port_);
        }

        Endpoint::operator boost::asio::ip::tcp::endpoint() const
        {
            assert(protocol() == unspec_protocol || protocol() == tcp);
            return boost::asio::ip::tcp::endpoint(ip(), port_);
        }

        std::string Endpoint::to_string() const
        {
            char const * proto_str = protocol() == tcp ? "(tcp)" : (protocol() == udp ? "(udp)" : "");
            char const * type_str = type() == nat ? "(nat)" : (type() == turn ? "(turn)" : (type() == reflex ? "(reflex)" : ""));
            return std::string() + proto_str + type_str + ip_str() + ":" + format(port_);
        }

        error_code Endpoint::from_string(
            std::string const & str)
        {
            std::string::size_type beg = 0;
            ProtocolEnum proto = unspec_protocol;
            if (str.compare(beg, 5, "(tcp)", 5) == 0) {
                proto = tcp;
                beg += 5;
            } else if (str.compare(beg, 5, "(udp)", 5) == 0) {
                proto = udp;
                beg += 5;
            }
            TypeEnum type = local;
            if (str.compare(beg, 5, "(nat)", 5) == 0) {
                type = nat;
                beg += 5;
            } else if (str.compare(beg, 5, "(turn)", 5) == 0) {
                type = turn;
                beg += 6;
            } else if (str.compare(beg, 5, "(reflex)", 5) == 0) {
                type = reflex;
                beg += 8;
            }
            if (str.size() == beg)
                return succeed;
            if (str.at(beg) == '[') { // ipv6 with port
                std::string::size_type p = str.find(']', beg + 1);
                if (p == std::string::npos)
                    return invalid_argument;
                error_code ec;
                boost::asio::ip::address_v6 addr = 
                    boost::asio::ip::address_v6::from_string(str.substr(beg + 1, p - beg - 1), ec);
                if (ec)
                    return ec;
                ++p; // skip ']'
                boost::uint16_t port = port_;
                if (str.size() > p) {
                    if (str.at(p) != ':')
                        return invalid_argument;
                    if (parse2(str.substr(p + 1), port))
                        return invalid_argument;
                }
                ip_v6(addr);
                port_ = port;
            } else {
                std::string::size_type p = str.rfind(':');
                if (p == std::string::npos) {
                    p = str.size();
                } else if (str.find(':', beg) != p) { // may be ipv6 address only
                    p = str.size();
                }
                error_code ec;
                boost::asio::ip::address addr;
                bool has_addr = false;
                if (p > beg) {
                    addr = boost::asio::ip::address::from_string(str.substr(beg, p - beg), ec);
                    if (ec)
                        return ec;
                    has_addr = true;
                }
                if (str.size() > p)
                    ++p; // skip ':'
                boost::uint16_t port = port_;
                if (p < str.size()) {
                    if (parse2(str.substr(p), port))
                        return invalid_argument;
                }
                if (has_addr) {
                    ip(addr);
                }
                port_ = port;
            }
            protocol_ = type | proto;
            return succeed;
        }

        boost::asio::ip::address Endpoint::ip() const
        {
            if (family_ == v4) {
                return boost::asio::ip::address_v4(ip_v4_);
            } else if (family_ == v6) {
                return boost::asio::ip::address_v6(ip_v6_);
            } else {
                return boost::asio::ip::address();
            }
        }

        std::string Endpoint::ip_str() const
        {
            if (family_ == v4) {
                return boost::asio::ip::address_v4(ip_v4_).to_string();
            } else if (family_ == v6) {
                return "[" + boost::asio::ip::address_v6(ip_v6_).to_string() + "]";
            } else {
                return std::string();
            }
        }

        void Endpoint::ip(
            std::string const & str)
        {
            if (str.empty())
                return;
            if (str.at(0) == '[') { // ipv6 with port
                if (str.at(str.size() - 1) != ']')
                    return;
                error_code ec;
                boost::asio::ip::address_v6 addr = 
                    boost::asio::ip::address_v6::from_string(str.substr(1, str.size() - 2), ec);
                if (!ec) {
                    ip_v6(addr);
                }
            } else if (str.find(':') != std::string::npos) {
                error_code ec;
                boost::asio::ip::address_v6 addr = 
                    boost::asio::ip::address_v6::from_string(str, ec);
                if (!ec) {
                    ip_v6(addr);
                }
            } else {
                error_code ec;
                boost::asio::ip::address_v4 addr = 
                    boost::asio::ip::address_v4::from_string(str, ec);
                if (!ec) {
                    ip_v4(addr);
                }
            }
        }

        void Endpoint::ip(
            boost::asio::ip::address const & h)
        {
            if (h.is_v4()) {
                family_ = v4;
                ip_v4_ = h.to_v4().to_ulong();
            } else {
                family_ = v6;
                ip_v6_ = h.to_v6().to_bytes();
            }
        }

        void Endpoint::ip_v4(
            boost::asio::ip::address_v4 const & h)
        {
            family_ = v4;
            ip_v4_ = h.to_ulong();
        }

        void Endpoint::ip_v6(
            boost::asio::ip::address_v6 const & h)
        {
            family_ = v6;
            ip_v6_ = h.to_bytes();
        }

    } // namespace network
} // namespace framework
