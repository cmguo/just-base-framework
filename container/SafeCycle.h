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
            SafeCycle(
                size_t capacity)
                : m_capacity(capacity + 1)
                , m_datas(new value_type[capacity + 1])
                , m_write_index(0)
                , m_read_index(0)
            {

            }

            ~SafeCycle()
            {
                if (m_datas)
                    delete m_datas;
            }

        public:
            bool push(
                value_type const & t)
            {
                assert(!full());
                if (full()) {
                    return false;
                }
                m_datas[m_write_index] = t;
                ++m_write_index;
                return true;
            }

            bool pop()
            {
                assert(!empty());
                if (empty()) {
                    return false;
                }
                ++ m_read_index;
                if (m_read_index == m_capacity) {
                    m_read_index = 0;
                }
                return true;
            }

            void clear()
            {
                m_write_index = m_read_index = 0;
            }

        public:
            size_t capacity() const
            {
                return m_capacity - 1;
            }

            size_t size() const
            {
                return m_read_index <= m_write_index 
                    ? m_write_index - m_read_index 
                    : m_write_index + m_capacity - m_read_index;
            }

            bool empty() const
            {
                return m_read_index == m_write_index;
            }

            bool full() const
            {
                return m_write_index + 1 == m_read_index
                    || (m_write_index + 1 == m_capacity && m_read_index == 0);
            }

        public:
            value_type const & front() const
            {
                assert(!empty());
                return m_datas[m_read_index];
            }

            value_type & front()
            {
                assert(!empty());
                return m_datas[m_read_index];
            }

            value_type const & back() const
            {
                assert(!empty());
                size_t index = m_write_index == 0 ? m_capacity - 1 : m_write_index - 1;
                return m_datas[index];
            }

            value_type & back()
            {
                assert(!empty());
                size_t index = m_write_index == 0 ? m_capacity - 1 : m_write_index - 1;
                return m_datas[index];
            }

            value_type const & operator[](size_t index) const
            {
                assert(index < size());
                index += m_read_index;
                if (index >= m_capacity)
                    index -= m_capacity;
                return m_datas[index];
            }

            value_type & operator[](size_t index)
            {
                assert(index < size());
                index += m_read_index;
                if (index >= m_capacity)
                    index -= m_capacity;
                return m_datas[index];
            }

        public:
            iterator begin()
            {
                return iterator(m_datas, m_datas + m_read_index, m_datas + m_capacity);
            }

            iterator end()
            {
                return iterator(m_datas, m_datas + m_write_index, m_datas + m_capacity);
            }

            const_iterator begin() const
            {
                return const_iterator(m_datas, m_datas + m_read_index, m_datas + m_capacity);
            }

            const_iterator end() const
            {
                return const_iterator(m_datas, m_datas + m_write_index, m_datas + m_capacity);
            }

        private:
            size_t m_capacity;

            value_type * m_datas;

            volatile size_t m_write_index;

            volatile size_t m_read_index;
        };

    } // namespace container
} // namespace framework

#endif // _FRAMEWORK_CONTAINER_SAFE_CYCLE_H_
