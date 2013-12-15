// Interface.cpp

#include "framework/Framework.h"
#include "framework/network/Interface.h"
#include "framework/system/ErrorCode.h"

#ifdef BOOST_WINDOWS_API
#include "framework/network/impl/InterfaceWin32.h"
#else
#include "framework/network/impl/InterfaceLinux.h"
#endif

namespace framework
{
    namespace network
    {

        std::string Interface::hard_addr_string(
            char splitter, 
            bool up_cast) const
        {
            char const * hex_chr = up_cast ? "0123456789ABCDEF" : "0123456789abcdef";
            std::string hex(17, splitter);
            char * p = &hex[0];
            for (size_t i = 0; i < sizeof(hwaddr); i++) {
                boost::uint8_t d = hwaddr[i];
                *p++ = hex_chr[d >> 4];
                *p++ = hex_chr[d & 0x0F];
                ++p;
            }
            return hex;
        }

    }// namespace network
}// namespace framework
