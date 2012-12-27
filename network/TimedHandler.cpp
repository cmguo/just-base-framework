// TimedHandler.cpp

#include "framework/Framework.h"
#include "framework/network/TimedHandler.h"
#include "framework/network/BindHandler.h"

namespace framework
{
    namespace network
    {
        
        TimedHandler::TimedHandler(
            boost::asio::io_service & io_svc)
            : timer_(io_svc)
            , delay_(0)
            , canceler_(NULL)
        {
        }

        TimedHandler::TimedHandler(
            boost::asio::io_service & io_svc, 
            boost::uint32_t delay)
            : timer_(io_svc)
            , delay_(delay)
            , canceler_(NULL)
        {
        }

        TimedHandler::~TimedHandler()
        {
            if (canceler_)
                delete canceler_;
        }

    } // namespace network
} // namespace framework
