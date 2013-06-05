// cycle.h

#ifndef _FRAMEWORK_CONTAINER_DETAIL_CYCLE_ITERATOR_H_
#define _FRAMEWORK_CONTAINER_DETAIL_CYCLE_ITERATOR_H_

#include <boost/operators.hpp>

namespace framework
{
    namespace container
    {

        namespace detail
        {

            template<
                typename _Ty
            >
            class cycle_iterator;

            struct cycle_iterator_access
            {
                template <
                    typename _Ty
                >
                static _Ty * beg(
                    cycle_iterator<_Ty> const & i)
                {
                    return i.beg_;
                }

                template <
                    typename _Ty
                >
                static _Ty * cur(
                    cycle_iterator<_Ty> const & i)
                {
                    return i.cur_;
                }

                template <
                    typename _Ty
                >
                static _Ty * end(
                    cycle_iterator<_Ty> const & i)
                {
                    return i.end_;
                }
            };

            template<
                typename _Ty
            >
            class cycle_iterator
                : public boost::forward_iterator_helper<
                cycle_iterator<_Ty>, 
                _Ty
                >
            {
            public:
                cycle_iterator(
                    _Ty * beg, 
                    _Ty * cur, 
                    _Ty * end)
                    : beg_(beg)
                    , cur_(cur)
                    , end_(end)
                {
                }

                cycle_iterator(
                    cycle_iterator const & r)
                    : beg_(r.beg_)
                    , cur_(r.cur_)
                    , end_(r.end_)
                {
                }

                template<
                    typename _Ty1
                >
                cycle_iterator(
                    cycle_iterator<_Ty1> const & r, 
                    typename boost::enable_if<boost::is_convertible<_Ty1 *, _Ty *> >::type * = NULL)
                    : beg_(cycle_iterator_access::beg(r))
                    , cur_(cycle_iterator_access::cur(r))
                    , end_(cycle_iterator_access::end(r))
                {
                }

            public:
                _Ty & operator*() const
                {
                    return *cur_;
                }

                cycle_iterator & operator++()
                {
                    ++cur_;
                    if (cur_ == end_)
                        cur_ = beg_;
                    return *this;
                }

            public:
                friend bool operator==(
                    cycle_iterator const & l,
                    cycle_iterator const & r)
                {
                    assert(l.beg_ == r.beg_ && l.end_ == r.end_);
                    return l.cur_ == r.cur_;
                }

            private:
                friend struct cycle_iterator_access;

            private:
                _Ty * beg_;
                _Ty * cur_;
                _Ty * end_;
            };

        } // namespace detail

    } // namespace container
} // namespace framework

#endif // _FRAMEWORK_CONTAINER_DETAIL_CYCLE_ITERATOR_H_
