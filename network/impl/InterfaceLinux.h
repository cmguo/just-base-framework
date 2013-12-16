// InterfaceLinux.h

#include <boost/asio/ip/tcp.hpp>

#include <net/if.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef __FreeBSD__
#  define FRAMEWORK_NETWORK_HAVE_SOCKADDR_LEN
#  define ifr_ifindex ifr_index
#  define ifr_netmask ifr_addr
#  include <net/if_dl.h>
#endif
#ifdef __MACH__
#  define FRAMEWORK_NETWORK_HAVE_SOCKADDR_LEN
#  define ifr_netmask ifr_addr
#  include <net/if_dl.h>
#endif

namespace framework
{
    namespace network
    {

        boost::system::error_code enum_interface(
            std::vector<Interface> & interfaces)
        {
            // 通过/proc/net/dev枚举接口
            int fd = ::open("/proc/net/dev", O_RDONLY);
            if (fd >= 0) {
                char buf[256];
                size_t line = 0;
                char * ptr = buf;
                size_t len = 0;
                while (true) {
                    if (ptr == buf) {
                        len += ::read(fd, ptr + len, sizeof(buf) - len);
                        if (buf + len <= ptr)
                            break;
                    }
                    if (line < 2) {
                        char * linebreak = (char *)::memchr(ptr, '\n', len);
                        if (linebreak != NULL) {
                            ++line;
                            ++linebreak;
                            len -= (linebreak - ptr);
                            ptr = linebreak;
                        } else {
                            ptr = buf;
                            len = 0;
                        }
                    }
                    if (line == 2) {
                        char * colon = (char *)::memchr(ptr, ':', len);
                        if (colon != NULL) {
                            while (*ptr == ' ') {
                                ++ptr;
                                --len;
                            }
                            Interface inf;
                            ::memcpy(inf.name, ptr, colon - ptr);
                            inf.name[colon - ptr] = 0;
                            interfaces.push_back(inf);
                            --line;
                            ++colon;
                            len -= (colon - ptr);
                            ptr = colon;
                        } else {
                            ::memmove(buf, ptr, len);
                            ptr = buf;
                        }
                    }
                }
                ::close(fd);
            } else if ((fd = ::socket(AF_INET, SOCK_DGRAM, 0)) >= 0) {
                size_t count = 16;
                size_t last_len = 0;
                struct ifconf ifc;
                for (; ;) {
                    ifc.ifc_len = sizeof(struct ifreq) * count;
                    ifc.ifc_buf = (caddr_t)new char[ifc.ifc_len];
                    if (::ioctl(fd, SIOCGIFCONF, &ifc) < 0) {
                        if (errno != EINVAL || last_len != 0) {
                            delete [] (char *)ifc.ifc_buf;
                            return framework::system::last_system_error();
                        }
                    } else {
                        if ((size_t)ifc.ifc_len == last_len)
                            break;          /* success, len has not changed */
                        last_len = ifc.ifc_len;
                    }
                    count += 10;
                    delete [] (char *)ifc.ifc_buf;
                }

                char const * last_name = "";
#ifndef SIOCGIFHWADDR
                char const * hwaddr = NULL;
#endif
                for (char * ptr = ifc.ifc_buf; ptr < ifc.ifc_buf + ifc.ifc_len; ) {
                    struct ifreq * ifr = (struct ifreq *)ptr;
                    size_t len = 0;
#ifndef FRAMEWORK_NETWORK_HAVE_SOCKADDR_LEN
                    switch (ifr->ifr_addr.sa_family) {
                                case AF_INET6:
                                    len = sizeof(struct sockaddr_in6);
                                    break;
                                case AF_INET:
                                default:
                                    len = sizeof(struct sockaddr);
                                    break;
                    }
#else
                    len = std::max(sizeof(struct sockaddr), (size_t)ifr->ifr_addr.sa_len);
#endif
                    ptr += sizeof(ifr->ifr_name) + len; /* for next one in buffer */

#ifndef SIOCGIFHWADDR
                    /* assumes that AF_LINK precedes AF_INET or AF_INET6 */
                    if (ifr->ifr_addr.sa_family == AF_LINK) {
                        struct sockaddr_dl * sdl = (struct sockaddr_dl *) &ifr->ifr_addr;
                        hwaddr = sdl->sdl_data + sdl->sdl_nlen;
                        continue;
                    }
#endif    
                    if (ifr->ifr_addr.sa_family != AF_INET)
                        continue;

                    char * cptr = strchr(ifr->ifr_name, ':');
                    if (cptr != NULL)
                        continue;       /* already processed this interface */
                    if (strncmp(last_name, ifr->ifr_name, IFNAMSIZ) == 0) {
                        continue;       /* already processed this interface */
                    }
                    last_name = ifr->ifr_name;

                    Interface inf;
                    memcpy(inf.name, ifr->ifr_name, sizeof(inf.name));
                    inf.name[sizeof(inf.name) - 1] = '\0';
#ifndef SIOCGIFHWADDR
                    if (hwaddr != NULL) {
                        memcpy(inf.hwaddr, hwaddr, sizeof(inf.hwaddr));
                        hwaddr = NULL;
                    }
#endif    
                    interfaces.push_back(inf);
                } // for
                delete [] (char *)ifc.ifc_buf;
                ::close(fd);
            } else {
                return framework::system::last_system_error();
            }

            fd = socket (AF_INET, SOCK_DGRAM, 0);
            if (fd < 0)  {
                return framework::system::last_system_error();
            }

            for (size_t i = 0; i < interfaces.size(); ++i) {
                Interface & inf = interfaces[i];
                struct ifreq ifrcopy;
                ::memcpy(ifrcopy.ifr_name, inf.name, sizeof(ifrcopy.ifr_name));
                ifrcopy.ifr_name[sizeof(ifrcopy.ifr_name) - 1] = '\0';
#ifndef SIOCGIFINDEX
        inf.index = 0;
#else
                if (!(::ioctl(fd, SIOCGIFINDEX, (char *)&ifrcopy))) {
                    inf.index = ifrcopy.ifr_ifindex;
                }
#endif
                if (!(::ioctl (fd, SIOCGIFMETRIC, (char *)&ifrcopy))) {
                    inf.metric = ifrcopy.ifr_metric;
                }
                if (!(::ioctl (fd, SIOCGIFMTU, (char *)&ifrcopy))) {
                    inf.mtu = ifrcopy.ifr_mtu;
                }
                if (!(::ioctl (fd, SIOCGIFFLAGS, (char *)&ifrcopy))) {
                    inf.flags = (Interface::FlagEnum)ifrcopy.ifr_flags;
                }
#ifdef SIOCGIFHWADDR
                if (!(::ioctl (fd, SIOCGIFHWADDR, (char *)&ifrcopy))) {
                    memcpy(inf.hwaddr, ifrcopy.ifr_hwaddr.sa_data, sizeof(inf.hwaddr));
                }
#endif
                if (!(::ioctl (fd, SIOCGIFADDR, (char *)&ifrcopy))) {
                    boost::asio::ip::tcp::endpoint ep;
                    memcpy(ep.data(), &ifrcopy.ifr_addr, sizeof(ifrcopy.ifr_addr));
                    inf.addr = ep.address();
                }
                if (!(::ioctl (fd, SIOCGIFNETMASK, (char *)&ifrcopy))) {
                    boost::asio::ip::tcp::endpoint ep;
                    memcpy(ep.data(), &ifrcopy.ifr_netmask, sizeof(ifrcopy.ifr_addr));
                    inf.netmask = ep.address();
                }
            } // while
            ::close(fd);
            return boost::system::error_code();
        }

    }// namespace network
}// namespace framework
