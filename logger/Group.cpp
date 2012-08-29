// Group.cpp

#include "framework/Framework.h"
#include "framework/logger/Group.h"
#include "framework/logger/Logger.h"
#include "framework/logger/Stream.h"
#include "framework/logger/Manager.h"

#include <stdio.h>

namespace framework
{
    namespace logger
    {

        struct Group::StreamNode
        {
            StreamNode(
                Stream * stream)
                : stream(stream)
                , next(NULL)
            {
            }

            ~StreamNode()
            {
            }

            Stream * stream;            /// ������ָ��
            StreamNode * next;           /// ��һ���ڵ�
        };

        Group::Group(
            Manager & logmgr)
            : next_(NULL)
            , manager_(logmgr)
            , streams_(NULL)

        {
            level_ = All;
        }

        Group::~Group()
        {
            while (StreamNode * s = streams_) {
                streams_ = streams_->next;
                delete s;
            }
        }

        /// ���մӴ�С��˳�򣬲���һ����
        void Group::add_stream(
            Stream * ls)
        {
            level_ = ls->level_ > level_ ? ls->level_ : level_;

            StreamNode ** p = &streams_;

            while (*p && (*p)->stream->level_ > ls->level_) {
                p = &(*p)->next;
            }

            StreamNode * sn = new StreamNode(ls);
            sn->stream = ls;
            sn->next = *p;

            // mb()

            *p = sn;
        }

        /// ɾ��һ����
        void Group::del_stream(
            Stream * ls)
        {
            StreamNode **p = &streams_;

            while (*p && (*p)->stream != ls) {
                p = &(*p)->next;
            }

            StreamNode * sn = NULL;

            if (*p) {
                sn = *p;
                *p = (*p)->next;
            }

            sn->next = NULL;
            sn->stream = NULL;
            delete sn;
        }

        /// �����־
        void Group::log(
            Module const & module, 
            size_t level, 
            Record const & record)
        {
            Context & ctx = manager_.context();
            ctx.reset(module, (LevelEnum)level);
            size_t len = record.format_message(ctx.msg_buf(), ctx.msg_len());
            ctx.msg_len(len);
            size_t count = mi_other;
            char const * buf = record.other_buffer(len);
            if (buf) {
                ctx[mi_other].buf = buf;
                ctx[mi_other].len = len;
                count = mi_max;
            }

            StreamNode * cur = streams_;
            while (cur && cur->stream->level_ >= level) {
                cur->stream->write(ctx.buffers(), count);
                cur = cur->next;
            }
        }

    } // namespace logger
} // namespace framework
