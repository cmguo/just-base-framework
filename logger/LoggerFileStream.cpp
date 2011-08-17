// LoggerFileStream.cpp

#include "framework/Framework.h"
#include "framework/logger/LoggerFileStream.h"
#include "framework/filesystem/Path.h"

#include <stdio.h>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>

#ifndef BOOST_WINDOWS_API
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

namespace framework
{
    namespace logger
    {
        LoggerFileStream::LoggerFileStream( 
            std::string const & file, 
            size_t size, 
            bool app, 
            bool day, 
            bool roll )
            : m_log_app_( app )
            , m_log_day_( day )
            , m_log_roll_( roll )
            , m_log_size_( size )
            , m_log_file_( file )
        {
            if (m_log_file_[0] == '$' 
                && m_log_file_[1] == 'L' 
                && m_log_file_[2] == 'O' 
                && m_log_file_[3] == 'G') {
                    m_log_file_ = 
                        framework::filesystem::log_path().file_string() 
                        + m_log_file_.substr(4);
            }
            else
            {
                m_log_file_ = 
                    framework::filesystem::log_path().file_string() 
                    + "/" + m_log_file_;
            }

            time_t tt = time(NULL);
            struct tm lt;
            localtime_r(&tt, &lt);
            lt.tm_hour = lt.tm_min = lt.tm_sec = 0;
            mid_night_ = mktime(&lt);

            check_file();
        }

        void LoggerFileStream::write( 
            muti_buffer_t const * logmsgs, 
            size_t len )
        {
            if ( !len ) return;

            at_new_time();

#ifdef BOOST_WINDOWS_API
            for ( size_t iLoop = 0; iLoop < len; ++iLoop )
            {
                fwrite( logmsgs[iLoop].buffer, logmsgs[iLoop].len, 1, fd_ );
            }
            fflush(fd_);

#else
            ::writev( fd_, ( iovec *)logmsgs, len );
#endif
        }

#ifdef BOOST_WINDOWS_API
        bool LoggerFileStream::check_file()
        {
            if ( boost::filesystem::exists( m_log_file_ ) )
            {/// file exist
                errno_t ec = fopen_s( &fd_, m_log_file_.c_str(), "a" );
                if ( ec ) return false;

                if ( m_log_day_ )
                {
                    fclose( fd_ );
                    backup_file( m_log_file_.c_str() );
                    errno_t ec = fopen_s( &fd_, m_log_file_.c_str(), "w" );
                    if ( ec ) return false;
                }
                else
                {
                    if ( m_log_size_ )
                    {
                        fseek( fd_, 0, SEEK_END );
                        if ( ( size_t )ftell( fd_ ) > m_log_size_ )
                        {
                            if ( m_log_roll_ )
                            {
                                fclose( fd_ );
                                backup_file( m_log_file_.c_str() );
                                errno_t ec = fopen_s( &fd_, m_log_file_.c_str(), "a" );
                                if ( ec ) return false;
                                fseek( fd_, 0, SEEK_SET );
                            }
                            else if ( m_log_app_ )
                            {
                                fseek( fd_, 0, SEEK_END );
                            }
                            else
                            {
                                fclose( fd_ );
                                backup_file( m_log_file_.c_str() );
                                errno_t ec = fopen_s( &fd_, m_log_file_.c_str(), "w" );
                                if ( ec ) return false;
                            }
                        }
                        else
                        {
                            fseek( fd_, 0, SEEK_END );
                        }
                    }
                    else
                    {
                        fseek( fd_, 0, SEEK_END );
                    }
                }
            }
            else
            {// Create a file
                errno_t ec = fopen_s( &fd_, m_log_file_.c_str(), "w" );
                if ( ec ) return false;
            }
            return true;
        }
#else
        bool LoggerFileStream::check_file()
        {
            if ( boost::filesystem::exists( m_log_file_ ) )
            {/// file exist
                fd_ = ::open( m_log_file_.c_str(), O_APPEND | O_RDWR );
                if ( fd_ == -1 ) return false;

                if ( m_log_day_ )
                {/// create everyday
                    ::close( fd_ );
                    backup_file( m_log_file_.c_str() );
                    fd_ = ::open( m_log_file_.c_str(), O_CREAT | O_RDWR, 00666 );
                    if ( fd_ == -1 ) return false;
                }
                else 
                { 
                    if ( m_log_size_ )
                    {/// limit size
                        if ( ( size_t )::lseek( fd_, 0, SEEK_END ) > m_log_size_ )
                        {
                            if ( m_log_roll_ )
                            {/// 
                                ::close( fd_ );
                                backup_file( m_log_file_.c_str() );
                                fd_ = ::open( m_log_file_.c_str(), O_APPEND | O_RDWR );
                                if ( fd_ == -1 ) return false;
                                ::lseek( fd_, 0, SEEK_SET );
                            }
                            else if ( m_log_app_ )
                            {
                                ::lseek( fd_, 0, SEEK_END );
                            }
                            else
                            {
                                ::close( fd_ );
                                backup_file( m_log_file_.c_str() );
                                fd_ = ::open( m_log_file_.c_str(), O_CREAT | O_RDWR, 00666 );
                                if ( fd_ == -1 ) return false;
                            }
                        }
                        else
                        {
                            ::lseek( fd_, 0, SEEK_END );
                        }
                    }
                    else
                    {
                        ::lseek( fd_, 0, SEEK_END );
                    }
                }
            }
            else
            {// Create a file
                fd_ = ::open( m_log_file_.c_str(), O_CREAT | O_RDWR, 00666 );
                if ( fd_ == -1 ) return false;
            }
            return true;
        }
#endif

        bool LoggerFileStream::backup_file( char const * filename )
        {
            if (fd_) {
                m_log_app_ = false;
            }
            if (!m_log_app_) {
                std::string::size_type p = m_log_file_.rfind('/');
                char buf[40];
                ILoggerStream::time_str_now(buf, sizeof(buf), ".%Y-%m-%d %HH%MM%SS");
                if (p == std::string::npos) {
                    boost::filesystem::create_directory("log_bak");
                    if (boost::filesystem::exists(m_log_file_))
                        boost::filesystem::copy_file(m_log_file_, "log_bak/" + m_log_file_ + buf);
                } else {
                    boost::filesystem::create_directories((m_log_file_.substr(0, p) + "/log_bak").c_str());
                    if (boost::filesystem::exists(m_log_file_))
                        boost::filesystem::copy_file(m_log_file_, m_log_file_.substr(0, p) + "/log_bak" + m_log_file_.substr(p) + buf);
                }
            }
            return true;
        }

        void LoggerFileStream::at_new_time()
        {
            int t_diff = ( int )( ::time( NULL ) - mid_night_ );
            if ( t_diff >= 24 * 60 * 60 )
            {
                if ( t_diff >= 24 * 60 * 60 )
                {
#ifdef BOOST_WINDOWS_API
                    fclose( fd_ );
#else
                    ::close( fd_ );
#endif
                    check_file();
                    mid_night_ += 24 * 60 * 60;
                    t_diff -= 24 * 60 * 60;
                }
            }
        }

    } // namespace logger
} // namespace framework
