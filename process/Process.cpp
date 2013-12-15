// Process.cpp

#include "framework/Framework.h"
#include "framework/process/Process.h"
#include "framework/process/ProcessEnum.h"
#include "framework/process/ProcessStat.h"
#include "framework/process/Error.h"
using namespace framework::process::error;
#include "framework/logger/Logger.h"
#include "framework/logger/StreamRecord.h"
#include "framework/system/ErrorCode.h"
using namespace framework::system;

using namespace boost::filesystem;
using namespace boost::system;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("framework.process.Process", framework::logger::Warn);

namespace framework
{
    namespace process
    {

        namespace detail
        {
            size_t format(
                char * buf, 
                boost::uint64_t t)
            {
                char * p = buf;
                while (t > 0) {
                    *p++ = '0' + (t & 0x0000000f);
                    t >>= 4;
                }
                return p - buf;
            }

            boost::uint64_t parse(
                char * buf, 
                size_t size)
            {
                boost::uint64_t t = 0;
                for (size_t i = size - 1; i != (size_t)-1; --i) {
                    t <<= 4;
                    t |= (boost::uint64_t)(buf[i] - '0');
                }
                return t;
            }
        }

    }
}

#ifdef BOOST_WINDOWS_API
#  include "framework/process/impl/ProcessWin32.h"
#else
#  include "framework/process/impl/ProcessLinux.h"
#endif

namespace framework
{
    namespace process
    {

        Process::Process()
            : data_(NULL)
        {
        }

        Process::~Process()
        {
            if (is_open()) {
                error_code ec;
                close(ec);
            }
        }

        boost::system::error_code Process::open(
            path const & bin_file, 
            CreateParamter const & paramter, 
            boost::system::error_code & ec)
        {
            if (open(bin_file, ec)) {
                create(bin_file, paramter, ec);
            }
            return ec;
        }

        error_code Process::create(
            path const & bin_file, 
            boost::system::error_code & ec)
        {
            return create(bin_file, CreateParamter(), ec);
        }

        bool Process::is_open()
        {
            return !(data_ == NULL);
        }

        error_code Process::close(
            error_code & ec)
        {
            if (is_open()) {
                detach(ec);
            } else {
                ec = error_code();
            }
            if (data_) {
                delete data_;
                data_ = NULL;
            }
            return ec;
        }

        int Process::id() const
        {
            return data_ ? data_->pid : 0;
        }

        int Process::parent_id() const
        {
            return data_ ? data_->ppid : 0;
        }

        Process & self()
        {
            static Process p;
            if (!p.is_open()) {
                error_code ec;
                p.open(this_process::id(), ec);
                assert(!ec);
            }
            return p;
        }

    } // namespace process
} // namespace framework
