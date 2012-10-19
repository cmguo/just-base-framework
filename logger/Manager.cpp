// Manager.cpp

#include "framework/Framework.h"
#include "framework/logger/Manager.h"
#include "framework/logger/Group.h"
#include "framework/logger/Stream.h"
#include "framework/logger/Logger.h"
#include "framework/logger/Context.h"
#include "framework/logger/ConsoleStream.h"
#include "framework/logger/FileStream.h"
#include "framework/logger/UdpStream.h"
#include "framework/configure/Config.h"
#include "framework/string/StringToken.h"
using namespace framework::configure;
using namespace framework::string;

namespace framework
{
    namespace logger
    {

        Manager & global_logger()
        {
            static Manager mgr;
            return mgr;
        }

        Manager::Manager()
            : streams_( NULL )
            , groups_( NULL )
            , modules_( NULL )
            , id_format_("[%t]")
            , defalut_level_(Info)
        {
            group_null_ = new Group(*this);
        }

        Manager::~Manager()
        {
            while (Module * lm = modules_) {
                modules_ = modules_->next_;
                delete lm;
            }

            while (Group * lg = groups_) {
                groups_ = groups_->next_;
                delete lg;
            }

            while (Stream * ils = streams_) {
                streams_ = streams_->next_;
                delete ils;
            }
        }

        /// 从配置文件读取每个输出流，创建输出流队列
        void Manager::load_config( 
            framework::configure::Config & conf )
        {
            config_ = &conf;

            ConfigModule & logger_mgr_config = 
                conf.register_module( "framework.logger.Manager" );

            /// 进程ID、线程ID打印标识
            logger_mgr_config << CONFIG_PARAM_NAME_RDWR("id_format", id_format_);
            logger_mgr_config << CONFIG_PARAM_NAME_RDWR("defalut_group", defalut_group_);
            logger_mgr_config << CONFIG_PARAM_NAME_RDWR("defalut_level", defalut_level_);

            // 初始化流链表，先于日志模块初始化
            init_streams(conf);

            // 初始化日志链表
            init_groups(conf);
        }

        /// 注册模块
        Module & Manager::register_module(
            char const * name, 
            size_t level)
        {
            boost::mutex::scoped_lock lk(mutex_);

            for (Module * m = modules_; m; m = m->next_) {
                if (m->name_ == name || strcmp(m->name_, name) == 0)
                    return *m;
            }

            /* 模块的 log_level 的配置顺序是：
                1、直接配置
                    a、覆盖参数指定 --<name.log_level>=xx
                    b、配置文件     [name]log_level=xx
                    c、默认参数指定 ++<name.log_level>=xx
                2、继承配置（每级继承又包括上面的a、b、c）
                3、程序注册等级，使用 FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL
                4、全局默认 defalut_level
             */

            std::string log_group = defalut_group_;
            size_t log_level = level == None ? defalut_level_ : level;
            if (config_) {
                config_->register_module(name) 
                    << CONFIG_PARAM_NOACC(log_group)
                    << CONFIG_PARAM_NOACC(log_level);
            }

            Group * group = group_null_;
            for (Group * p = groups_; p; p = p->next_) {
                if (p->name_ == log_group) {
                    group = p;
                    break;
                }
            }

            Module * m = new Module(group, name, log_level);

            m->next_ = modules_;
            modules_ = m;

            return *m;
        }

        ///.添加一个流到指定日志
        bool Manager::add_stream(
            Stream & s)
        {
            boost::mutex::scoped_lock lk(mutex_);

            for (Group * g = groups_; g; g = g->next_) {
                g->add_stream(&s);
            }
            return true;
        }

        ///.添加一个流到指定组
        bool Manager::add_stream(
            Stream & s, 
            std::string const & group)
        {
            boost::mutex::scoped_lock lk(mutex_);

            for (Group * g = groups_; g; g = g->next_) {
                if (g->name_ == group) {
                    g->add_stream(&s);
                    return true;
                }
            }
            return false;
        }

        /// 删除一个流
        bool Manager::del_stream(
            Stream & s)
        {
            boost::mutex::scoped_lock lk(mutex_);

            for (Group * g = groups_; g; g = g->next_) {
                g->del_stream(&s);
            }
            return true;
        }

        /// 初始化输出流链表
        void Manager::init_streams(
            framework::configure::Config & conf)
        {
            std::set<std::string> streams;
            conf.profile().get_section_pattern("framework.logger.Stream.", streams);

            for (std::set<std::string>::iterator it = streams.begin() ; it != streams.end(); ++it) {
                ConfigModule & cm = conf.register_module(*it);
                Stream * s = Stream::create(cm);
                if (s == NULL) {
                    continue;
                }
                s->name_ = (*it).substr(sizeof("framework.logger.Stream"));
                /// 加入流链表头部
                s->next_ = streams_;
                streams_ = s;
            }
        }

        /// 初始化日志链表
        void Manager::init_groups(
            framework::configure::Config & conf)
        {
            std::set<std::string> groups;
            conf.profile().get_section_pattern("framework.logger.Group.", groups);

            // 一定要有一个名字为空的Group，不指定Group名字的Module使用这个
            bool has_empty = false;

            for (std::set<std::string>::iterator it = groups.begin() ; it != groups.end(); ++it) {

                Group * g = new Group(*this);
                g->name_ = (*it).substr(sizeof("framework.logger.Group"));

                ConfigModule & cm = conf.register_module(*it);
                std::string logstreams;
                cm << CONFIG_PARAM_NAME_RDONLY("level", g->level_);
                cm << CONFIG_PARAM_RDONLY(logstreams);

                std::string stream;
                boost::system::error_code ec;
                StringToken st(logstreams, ";");
                st.next_token(stream, ec);
                while (!st.next_token(stream, ec)) {
                    Stream * s = streams_;
                    while ( s ) {
                        if ( s->name_ == stream ) {
                            g->add_stream(s);
                            break;
                        }
                        s = s->next_;
                    }
                }

                if (g->name_.empty())
                    has_empty = true;

                /// 插入日志链头部
                g->next_ = groups_;
                groups_ = g;
            }

            if (!has_empty) {
                Group * g = new Group(*this);
                Stream * s = streams_;
                for (Stream * s = streams_; s; s = s->next_) {
                    g->add_stream(s);
                }

                /// 插入日志链头部
                g->next_ = groups_;
                groups_ = g;
            }
        }

        Context & Manager::context()
        {
            Context * ctx = thread_ctx_.get();
            if (ctx == NULL) {
                ctx = new Context(id_format_.c_str(), time_.time_str());
                thread_ctx_.reset(ctx);
            }
            time_.update(mutex_);
            return *ctx;
        }

        Manager & global_logger_mgr()
        {
            static Manager slog_mgr_;
            return slog_mgr_;
        }

    } // namespace logger
} // namespace framework
