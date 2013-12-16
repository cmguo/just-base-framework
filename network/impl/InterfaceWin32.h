// Interface.cpp

#include "framework/string/Uuid.h"

#include <boost/asio/io_service.hpp>

#include <windows.h>
#if (!defined WINRT) && (!defined WIN_PHONE)
#  include <iphlpapi.h>
#  pragma comment(lib, "Iphlpapi.lib")
#endif

namespace framework
{
    namespace network
    {

#if (defined WINRT) || (defined WIN_PHONE)
        
        boost::system::error_code enum_interface(
            std::vector<Interface> & interfaces)
        {
            char buf[1024];
            int len = ::getadapters(buf, sizeof(buf));
            if (len < 0) {
                return framework::system::last_system_error();
            }
            char const * p = buf;
            while (len) {
                adapter const * a = (adapter const *)p;
                p += a->len;
                len -= a->len;
                Interface inf;
                inf.flags = Interface::up;
                std::string name = framework::string::Uuid((framework::string::UUID const &)a->id).to_string();
                strncpy(inf.name, name.c_str(), sizeof(inf.name));
                boost::system::error_code ec;
                inf.addr = boost::asio::ip::address::from_string((char *)(a + 1), ec);
                memcpy(inf.hwaddr, a->id.Data4, sizeof(inf.hwaddr));
                if (!ec) {
                    interfaces.push_back(inf);
                }
            }
            return  boost::system::error_code();
        }

#else

        boost::system::error_code enum_interface(
            std::vector<Interface> & interfaces)
        {
            /* Declare and initialize variables */

            // It is possible for an adapter to have multiple
            // IPv4 addresses, gateways, and secondary WINS servers
            // assigned to the adapter. 
            //
            // Note that this sample code only prints out the 
            // first entry for the IP address/mask, and gateway, and
            // the primary and secondary WINS server for each adapter. 

            boost::asio::io_service io_svc; // for WSAStartup
            boost::system::error_code ec;

            PIP_ADAPTER_INFO pAdapterInfo;
            PIP_ADAPTER_INFO pAdapter = NULL;
            DWORD dwRetVal = 0;

            ULONG ulOutBufLen = sizeof (IP_ADAPTER_INFO);
            pAdapterInfo = (IP_ADAPTER_INFO *) malloc(sizeof (IP_ADAPTER_INFO));
            if (pAdapterInfo == NULL) {
                printf("Error allocating memory needed to call GetAdaptersinfo\n");
                return framework::system::last_system_error();
            }
            // Make an initial call to GetAdaptersInfo to get
            // the necessary size into the ulOutBufLen variable
            if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
                free(pAdapterInfo);
                pAdapterInfo = (IP_ADAPTER_INFO *) malloc(ulOutBufLen);
                if (pAdapterInfo == NULL) {
                    printf("Error allocating memory needed to call GetAdaptersinfo\n");
                    return framework::system::last_system_error();
                }
            }
            if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
                pAdapter = pAdapterInfo;
                while (pAdapter) {
                    Interface inf;
                    strncpy(inf.name, pAdapter->AdapterName, sizeof(inf.name));
                    memcpy(inf.hwaddr, pAdapter->Address, sizeof(inf.hwaddr));
                    inf.index = pAdapter->Index;
                    inf.flags |= Interface::up;
                    switch (pAdapter->Type) {
                        case MIB_IF_TYPE_PPP:
                            inf.flags |= Interface::pointtopoint;
                            break;
                        case MIB_IF_TYPE_LOOPBACK:
                            inf.flags |= Interface::loopback;
                            break;
                    }
                    boost::system::error_code ec1;
                    inf.addr = boost::asio::ip::address::from_string(
                        pAdapter->IpAddressList.IpAddress.String, ec1);
                    inf.netmask = boost::asio::ip::address::from_string(
                        pAdapter->IpAddressList.IpMask.String, ec1);
                    interfaces.push_back(inf);
                    pAdapter = pAdapter->Next;
                }
            } else {
                printf("GetAdaptersInfo failed with error: %d\n", dwRetVal);
                ec = framework::system::last_system_error();
            }
            if (pAdapterInfo)
                free(pAdapterInfo);
            return ec;
        }

#endif

    }// namespace network
}// namespace framework
