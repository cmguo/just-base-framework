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
            , port_(0)
        {
        }

        Acceptor::~Acceptor()
        {
            if (closer_) {
                close();
            };
        }

        bool Acceptor::is_open()
        {
            return closer_ != NULL;
        }

        boost::system::error_code Acceptor::close(
            boost::system::error_code & ec)
        {
            if (closer_ == NULL) {
                ec = boost::asio::error::bad_descriptor;
            } else {
                (this->*closer_)(ec);
            }
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
