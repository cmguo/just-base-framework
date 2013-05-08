// SafeCycle.h

#ifndef _FRAMEWORK_CONTAINER_SAFE_CYCLE_H_
#define _FRAMEWORK_CONTAINER_SAFE_CYCLE_H_

#include "framework/container/detail/cycle_iterator.h"

namespace framework
{
    namespace container
    {

        template <
            typename _Ty
        >
        class SafeCycle
        {
        public:
            typedef _Ty value_type;

            typedef detail::cycle_iterator<value_type> iterator;

            typedef detail::cycle_iterator<value_type const> const_iterator;

        public:
            SafeCycle()
                : capacity_(0)
                , datas_(NULL)
                , write_index_(0)
                , read_index_(0)
            {
            }

            SafeCycle(
                size_t capacity)
                : capacity_(capacity + 1)
                , datas_(new value_type[capacity + 1])
                , write_index_(0)
                , read_index_(0)
            {
            }

            ~SafeCycle()
            {
                if (datas_)
                    delete datas_;
            }

        public:
            bool push(
                value_type const & t)
            {
                assert(!full());
                if (full()) {
                    return false;
                }
                datas_[write_index_] = t;
                size_t idx = ++write_index_;
                if (idx == capacity_) {
                    idx = 0;
                }
                write_index_ = idx;
                return true;
            }

            bool pop()
            {
                assert(!empty());
                if (empty()) {
                    return false;
                }
                size_t idx = ++read_index_;
                if (idx == capacity_) {
                    idx = 0;
                }
                read_index_ = idx;
                return true;
            }

            bool pop(
                size_t n)
            {
                assert(size() >= n);
                if (size() < n) {
                    return false;
                }
                size_t idx = read_index_ + n;
                if (idx >= capacity_) {
                    idx -= capacity_;
                }
                read_index_ = idx;
                return true;
            }

            void reserve(
                size_t n)
            {
                assert(size() <= n);
                if (size() <= n) {
                    return;
                }
                value_type * datas = new value_type[++n];
                std::copy(begin(), end(), datas);
                delete datas_;
                datas_ = datas;
                write_index_ = size();
                read_index_ = 0;
                capacity_ = n;
            }

            void clear()
            {
                write_index_ = read_index_ = 0;
            }

        public:
            size_t capacity() const
            {
                return capacity_ - 1;
            }

            size_t size() const
            {
                return read_index_ <= write_index_ 
                    ? write_index_ - read_index_ 
                    : write_index_ + capacity_ - read_index_;
            }

            bool empty() const
            {
                return read_index_ == write_index_;
            }

            bool full() const
            {
                return write_index_ + 1 == read_index_
                    || (write_index_ + 1 == capacity_ && read_index_ == 0);
            }

        public:
            value_type const & front() const
            {
                assert(!empty());
                return datas_[read_index_];
            }

            value_type & front()
            {
                assert(!empty());
                return datas_[read_index_];
            }

            value_type const & back() const
            {
                assert(!empty());
                size_t index = write_index_ == 0 ? capacity_ - 1 : write_index_ - 1;
                return datas_[index];
            }

            value_type & back()
            {
                assert(!empty());
                size_t index = write_index_ == 0 ? capacity_ - 1 : write_index_ - 1;
                return datas_[index];
            }

            value_type const & operator[](size_t index) const
            {
                assert(index < size());
                index += read_index_;
                if (index >= capacity_)
                    index -= capacity_;
                return datas_[index];
            }

            value_type & operator[](size_t index)
            {
                assert(index < size());
                index += read_index_;
                if (index >= capacity_)
                    index -= capacity_;
                return datas_[index];
            }

        public:
            iterator begin()
            {
                return iterator(datas_, datas_ + read_index_, datas_ + capacity_);
            }

            iterator end()
            {
                return iterator(datas_, datas_ + write_index_, datas_ + capacity_);
            }

            const_iterator begin() const
            {
                return const_iterator(datas_, datas_ + read_index_, datas_ + capacity_);
            }

            const_iterator end() const
            {
                return const_iterator(datas_, datas_ + write_index_, datas_ + capacity_);
            }

        private:
            size_t capacity_;
            value_type * datas_;
            volatile size_t write_index_;
            volatile size_t read_index_;
        };

    } // namespace container
} // namespace framework

#endif // _FRAMEWORK_CONTAINER_SAFE_CYCLE_H_
