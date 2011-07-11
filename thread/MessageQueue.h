// MessageQueue.h

#ifndef _FRAMEOWRK_THREAD_MESSAGE_QUEUE_H__
#define _FRAMEOWRK_THREAD_MESSAGE_QUEUE_H__

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

namespace framework 
{
    namespace thread
    {

        template <
            typename _Ty
        >
        class MessageQueue
        {
        public:
            typedef _Ty value_type;

        public:
            MessageQueue(
                size_t max_size = size_t(-1))
                : max_size_(max_size)
            {
            }

            ~MessageQueue()
            {
            }

        public:
            template<
                typename DurationType
            >
            bool timed_pop(
                value_type & t, 
                DurationType const & duration)
            {
                boost::mutex::scoped_lock lc(mutex_);
                if (empty()) {
                    cond_.timed_wait(lc, duration);
                }
                if (empty()) {
                    return false;
                }
                t = deq_.front();
                if (size() == max_size_)
                    cond_.notify_all();	
                deq_.pop_front();
                return true;
            }

            template<
                typename DurationType
            >
            bool timed_push(
                value_type const & t, 
                DurationType const & duration)
            {
                boost::mutex::scoped_lock lc(mutex_);
                if (deq_.size() >= max_size_) {
                    cond_.timed_wait(lc, duration);
                }
                if (deq_.size() >= max_size_) {
                    return false;
                }
                deq_.push_back(t);
                if(deq_.size() == 1) {
                    cond_.notify_all();	
                }
                return true;
            }

            bool pop(
                value_type & t)
            {
                boost::mutex::scoped_lock lc(mutex_);
                if (empty()) {
                    cond_.wait(lc);
                }
                if (empty()) {
                    return false;
                }
                t = deq_.front();
                if (size() == max_size_)
                    cond_.notify_all();	
                deq_.pop_front();
                return true;
            }

            bool push(
                value_type const & t)
            {
                boost::mutex::scoped_lock lc(mutex_);
                if (deq_.size() >= max_size_) {
                    cond_.wait(lc);
                }
                if (deq_.size() >= max_size_) {
                    return false;
                }
                deq_.push_back(t);
                if(deq_.size() == 1) {
                    cond_.notify_all();	
                }
                return true;
            }

        public:
            bool empty() const
            {
                return deq_.empty();
            }

            std::size_t size() const
            {
                return deq_.size();
            }

        private:
            std::deque<_Ty> deq_;
            boost::mutex mutex_;
            boost::condition_variable cond_;
            size_t max_size_;
        };

    } // namespace thread
} // namespace framework 

#endif // _FRAMEOWRK_THREAD_MESSAGE_QUEUE_H__
