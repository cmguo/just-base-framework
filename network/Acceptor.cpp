// Acceptor.cpp

#include "framework/Framework.h"
#include "framework/network/Acceptor.h"
#include "framework/network/Acceptor.hpp"

namespace framework
{
    namespace network
    {

        Acceptor::Acceptor(
            boost::asio::io_service & io_svc)
            : io_svc_(io_svc)
            , closer_(NULL)
        {
        }

        Acceptor::~Acceptor()
        {
            if (closer_) {
                close();
            };
        }

        boost::system::error_code Acceptor::close(
            boost::system::error_code & ec)
        {
            assert(closer_ != NULL);
            (this->*closer_)(ec);
            return ec;
        }

        void Acceptor::close()
        {
            boost::system::error_code ec;
            close(ec);
            boost::asio::detail::throw_error(ec);
        }

    } // namespace network
} // namespace framework
