// FileStream.cpp

#include "framework/Framework.h"
#include "framework/logger/FileStream.h"
#include "framework/filesystem/Path.h"
#include "framework/configure/Config.h"
using namespace framework::configure;

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>

#ifndef BOOST_WINDOWS_API
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <fcntl.h>
#  include <unistd.h>
#endif

namespace framework
{
    namespace logger
    {

        FileStream::FileStream()
            : time_(true)
            , app_(false)
            , daily_(false)
            , roll_(false)
            , size_(0)
        {
        }

        FileStream::~FileStream()
        {
            close();
        }

        void FileStream::load_config(
            ConfigModule & cm)
        {
            Stream::load_config(cm);

            cm << CONFIG_PARAM_NAME_RDWR("file", name_);
            cm << CONFIG_PARAM_NAME_RDWR("append", app_);
            cm << CONFIG_PARAM_NAME_RDWR("daily", daily_);
            cm << CONFIG_PARAM_NAME_RDWR("size", size_);
            cm << CONFIG_PARAM_NAME_RDWR("roll", roll_);

            if (name_[0] == '$' 
                && name_[1] == 'L' 
                && name_[2] == 'O' 
                && name_[3] == 'G') {
                    name_ = 
                        framework::filesystem::log_path().string() 
                        + name_.substr(4);
            }

            backup_file();
        }

        void FileStream::write( 
            buffers_t const & buffers)
        {
            if (check_file()) {
                boost::mutex::scoped_lock lk(mutex_);
                if (check_file()) {
                    time_.update();
                    backup_file();
                }
            }

            boost::system::error_code ec;
            file_.write_some(buffers, ec);
        }

        bool FileStream::open()
        {
            boost::system::error_code ec;
            return file_.open(name_, ec);
        }

        bool FileStream::is_open() const
        {
            return file_.is_open();
        }

        bool FileStream::close()
        {
            boost::system::error_code ec;
            return file_.close(ec);
        }

        bool FileStream::reopen()
        {
#ifdef BOOST_WINDOWS_API
            return false;
#else
            
            framework::filesystem::File file;
	    boost::system::error_code ec;
            file.open(name_, ec);
            file.swap(file_);
            return file_.is_open();
#endif
        }

        bool FileStream::seek(
            bool beg_or_end)
        {
            boost::system::error_code ec;
            return file_.seek(beg_or_end ? file_.beg : file_.end, 0, ec);
        }

        size_t FileStream::position()
        {
            boost::system::error_code ec;
            return (size_t)file_.tell(ec);
        }

        bool FileStream::check_file()
        {
            if (size_ > 0 && position() >= size_)
                return true;
            if (daily_ && time_.check())
                return true;
            return false;
        }

        bool FileStream::backup_file()
        {
            bool app = app_;
            if (is_open()) {
                if (roll_) {
                    seek(true);
                    return true;
                }
#ifdef BOOST_WINDOWS_API
                close();
#endif
                app = false;
            }
            if (!app) {
                boost::filesystem::path ph(name_);
                boost::filesystem::path ph2(ph.parent_path() / "log_bak");
                boost::filesystem::path ph3(ph2 / (time_.time_str() + ph.filename().string()));
                try {
                    boost::filesystem::create_directory(ph2);
                    if (boost::filesystem::exists(ph))
                        boost::filesystem::rename(ph, ph3);
                } catch (...) {
                }
            }
#ifdef BOOST_WINDOWS_API
            open();
#else
            reopen();
#endif
            if (app)
                seek(false);
            return true;
        }

    } // namespace logger
} // namespace framework
