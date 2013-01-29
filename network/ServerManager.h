// ServerManager.h

#ifndef _FRAMEWORK_NETWORK_SERVER_MANAGER_H_
#define _FRAMEWORK_NETWORK_SERVER_MANAGER_H_

#include "framework/network/Acceptor.h"

#include <boost/asio/ip/tcp.hpp>
#include <boost/bind.hpp>

namespace framework
{
    namespace network
    {

        struct DefaultManager;

        template <
            typename Server, 
            typename Manager = DefaultManager
        >
        class ServerManager
        {
        public:
            ServerManager(
                boost::asio::io_service & io_svc) 
                : acceptor_(io_svc)
                , proxy_(NULL)
            {
            }

            boost::system::error_code start(
                framework::network::NetName & addr, 
                boost::system::error_code & ec)
            {
                if (!acceptor_.open<boost::asio::ip::tcp>(addr, ec)) {
                    proxy_ = create(this, (Manager *)NULL);
                    proxy_->async_accept(acceptor_, 
                        boost::bind(&ServerManager::handle_accept, this, _1));
                }
                return ec;
            }

            void start(
                framework::network::NetName & addr)
            {
                acceptor_.open<boost::asio::ip::tcp>(addr);
                proxy_ = create(this, (Manager *)NULL);
                proxy_->async_accept(acceptor_, 
                    boost::bind(&ServerManager::handle_accept, this, _1));
            }

            void stop()
            {
                boost::system::error_code ec;
                acceptor_.close(ec);
            }

        public:
            boost::asio::io_service & io_svc()
            {
                return acceptor_.get_io_service();
            }

        private:
            static Server * create(
                ServerManager * mgr, 
                ServerManager * mgr2)
            {
                return new Server(static_cast<Manager &>(*mgr));
            }

            static Server * create(
                ServerManager * mgr, 
                DefaultManager * mgr2)
            {
                return new Server(mgr->io_svc());
            }

        private:
            void handle_accept(
                boost::system::error_code const & ec)
            {
                if (!ec) {
                    proxy_->start();
                    proxy_ = create(this, (Manager *)NULL);
                    proxy_->async_accept(acceptor_, 
                        boost::bind(&ServerManager::handle_accept, this, _1));
                } else {
                    proxy_->on_error(ec);
                    delete proxy_;
                }
            }

        private:
            Acceptor acceptor_;
            Server * proxy_;
        };

    } // namespace network
} // namespace framework

#endif // _FRAMEWORK_NETWORK_SERVER_MANAGER_H_
// ServerManager.h

#ifndef _FRAMEWORK_NETWORK_SERVER_MANAGER_H_
#define _FRAMEWORK_NETWORK_SERVER_MANAGER_H_

#include "framework/network/Acceptor.h"

#include <boost/asio/ip/tcp.hpp>
#include <boost/bind.hpp>

namespace framework
{
    namespace network
    {

        struct DefaultManager;

        template <
            typename Server, 
            typename Manager = DefaultManager
        >
        class ServerManager
        {
        public:
            ServerManager(
                boost::asio::io_service & io_svc) 
                : acceptor_(io_svc)
                , proxy_(NULL)
            {
            }

            boost::system::error_code start(
                framework::network::NetName & addr, 
                boost::system::error_code & ec)
            {
                if (!acceptor_.open<boost::asio::ip::tcp>(addr, ec)) {
                    proxy_ = create(this, (Manager *)NULL);
                    proxy_->async_accept(acceptor_, 
                        boost::bind(&ServerManager::handle_accept, this, _1));
                }
                return ec;
            }

            void start(
                framework::network::NetName & addr)
            {
                acceptor_.open<boost::asio::ip::tcp>(addr);
                proxy_ = create(this, (Manager *)NULL);
                proxy_->async_accept(acceptor_, 
                    boost::bind(&ServerManager::handle_accept, this, _1));
            }

            void stop()
            {
                boost::system::error_code ec;
                acceptor_.close(ec);
            }

        public:
            boost::asio::io_service & io_svc()
            {
                return acceptor_.get_io_service();
            }

        private:
            static Server * create(
                ServerManager * mgr, 
                ServerManager * mgr2)
            {
                return new Server(static_cast<Manager &>(*mgr));
            }

            static Server * create(
                ServerManager * mgr, 
                DefaultManager * mgr2)
            {
                return new Server(mgr->io_svc());
            }

        private:
            void handle_accept(
                boost::system::error_code const & ec)
            {
                if (!ec) {
                    proxy_->start();
                    proxy_ = create(this, (Manager *)NULL);
                    proxy_->async_accept(acceptor_, 
                        boost::bind(&ServerManager::handle_accept, this, _1));
                } else {
                    proxy_->on_error(ec);
                    delete proxy_;
                }
            }

        private:
            Acceptor acceptor_;
            Server * proxy_;
        };

    } // namespace network
} // namespace framework

#endif // _FRAMEWORK_NETWORK_SERVER_MANAGER_H_
