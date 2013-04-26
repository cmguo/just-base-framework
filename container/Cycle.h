// Cycle.h

#ifndef _FRAMEWORK_CONTAINER_CYCLE_BUFFER_H_
#define _FRAMEWORK_CONTAINER_CYCLE_BUFFER_H_

#include "framework/container/detail/cycle_iterator.h"

namespace framework
{
    namespace container
    {

        template <
            typename _Ty
        >
        class Cycle
        {
        public:
            typedef _Ty value_type;

            typedef detail::cycle_iterator<value_type> iterator;

            typedef detail::cycle_iterator<value_type const> const_iterator;

        public:
            Cycle(
                size_t capacity)
                : m_capacity(capacity)
                , m_datas(new value_type[capacity])
                , m_write_index(0)
                , m_read_index(0)
                , m_size(0)
            {

            }

            ~Cycle()
            {
                if (m_datas)
                    delete m_datas;
            }

        public:
            void push(
                value_type const & t)
            {
                assert(m_size < m_capacity);
                m_datas[m_write_index] = t;
                ++m_write_index;
                ++m_size;
                if (m_write_index == m_capacity) {
                    m_write_index = 0;
                }
            }

            void pop()
            {
                assert(m_size > 0);
                ++m_read_index;
                --m_size;
                if (m_read_index == m_capacity) {
                    m_read_index = 0;
                }
            }

        public:
            size_t capacity() const
            {
                return m_capacity;
            }

            size_t size() const
            {
                return m_size;
            }

            value_type const & front() const
            {
                assert(m_size > 0);
                return m_datas[m_read_index];
            }

            value_type & front()
            {
                assert(m_size > 0);
                return m_datas[m_read_index];
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

            volatile size_t m_size;
        };

    } // namespace container
} // namespace framework

#endif // _FRAMEWORK_CONTAINER_CYCLE_BUFFER_H_
