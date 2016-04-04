// SparseArray.h

#ifndef _SPARSE_ARRAY_H_
#define _SPARSE_ARRAY_H_

#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_pod.hpp>
#include <boost/iterator/iterator_facade.hpp>

#include <memory>

namespace framework {
    namespace container {

        namespace detail
        {

            template <typename Value>
                struct SparseArrayItem
                {
                    size_t index;
                    Value value;

                    SparseArrayItem(
                        size_t i, 
                        Value const & v)
                        : index(i)
                          , value(v)
                    {
                    }

                    friend bool operator==(
                        SparseArrayItem const & l, 
                        SparseArrayItem const & r)
                    {
                        return l.value == r.value;
                    }
                };

            template <typename T, bool C>
                struct PointerType
                {
                    typedef typename boost::mpl::if_c<C, T const *, T *>::type type;
                };

            template <typename T, bool C>
                struct ReferenceType
                {
                    typedef typename boost::mpl::if_c<C, T const &, T &>::type type;
                };

            template <typename Value, bool C>
                struct IndexExtractor
                {
                    typedef size_t value_type;
                    typedef typename ReferenceType<size_t, C>::type reference_type;
                    typedef typename ReferenceType<SparseArrayItem<Value>, C>::type item_reference_type;

                    static reference_type extract(
                        item_reference_type item)
                    {
                        return item.index;
                    }
                };

            template <typename Value, bool C>
                struct ValueExtractor
                {
                    typedef Value value_type;
                    typedef typename ReferenceType<Value, C>::type reference_type;
                    typedef typename ReferenceType<SparseArrayItem<Value>, C>::type item_reference_type;

                    static reference_type extract(
                        item_reference_type item)
                    {
                        return item.value;
                    }
                };

            template <typename Value, template <typename, bool> class Extractor, bool C>
                class SparseArrayIterator
                : public boost::iterator_facade<
                  SparseArrayIterator<Value, Extractor, C>, 
                  typename Extractor<Value, C>::value_type, 
                  boost::random_access_traversal_tag, 
                  typename Extractor<Value, C>::reference_type, 
                  size_t> 
            {
            public:
                typedef typename PointerType<SparseArrayItem<Value>, C>::type item_ptr_t;;
                typedef Extractor<Value, C> extractor_t;
                typedef typename extractor_t::reference_type reference_type;
                typedef size_t size_type;

                SparseArrayIterator(
                    item_ptr_t ptr)
                    : ptr_(ptr)
                {
                }

                friend class SparseArrayIterator<Value, Extractor, !C>;

                SparseArrayIterator(
                    SparseArrayIterator<Value, Extractor, !C> const & o)
                    : ptr_(o.ptr_)
                {
                }

            public:
                size_type index() const
                {
                    return ptr_->index;
                }

                void increment()
                {
                    ++ptr_;
                }

                void decrement()
                {
                    --ptr_;
                }

                void advance(
                    boost::int64_t d)
                {
                    ptr_ += d;
                }

                reference_type dereference() const
                {
                    return extractor_t::extract(*ptr_);
                }

                template <bool C1>
                    bool equal(
                        SparseArrayIterator<Value, Extractor, C1> const & o) const
                    {
                        return ptr_ == o.ptr_;
                    }

            private:
                item_ptr_t ptr_;
            };

        } // namespace detail

        template <typename Value, typename Alloc = std::allocator<Value> >
            class SparseArray
            : Alloc::template rebind<detail::SparseArrayItem<Value> >::other
            {
            public:
                typedef Value value_t;
                typedef size_t size_type;
                typedef ptrdiff_t difference_type;
                typedef value_t & reference;
                typedef value_t const & const_reference;
                typedef value_t * pointer;
                typedef value_t const * const_pointer;

            private:
                typedef detail::SparseArrayItem<value_t> item_t;
                typedef typename Alloc::template rebind<item_t>::other alloc_t;

            public:
                typedef detail::SparseArrayIterator<Value, detail::IndexExtractor, true> index_iterator;
                typedef detail::SparseArrayIterator<Value, detail::ValueExtractor, true> value_const_iterator;
                typedef detail::SparseArrayIterator<Value, detail::ValueExtractor, false> value_iterator;

            public:
                SparseArray(
                    size_t init_capacity = 4)
                    : capacity_(init_capacity)
                    , size_(0)
                    , dirty_(0)
                {
                    if (capacity_ <= 4) {
                        items_ = (item_t *)qbuf_;
                    } else {
                        items_ = alloc_t::allocate(init_capacity);
                    }
                }

                ~SparseArray()
                {
                    for (size_t i = 0; i < size_; ++i) {
                        alloc_t::destroy(items_ + i);
                    }
                    size_ = 0;
                    dirty_ = 0;
                    if (capacity_ <= 4) {
                    } else {
                        alloc_t::deallocate(items_, capacity_);
                    }
                    items_ = NULL;
                }

            public:
                size_type size() const
                {
                    return size_ - dirty_;
                }

                index_iterator index_begin() const
                {
                    return index_iterator(&items_[0]);
                }

                index_iterator index_end() const
                {
                    return index_iterator(&items_[size_]);
                }

                value_iterator value_begin()
                {
                    return value_iterator(&items_[0]);
                }

                value_const_iterator value_begin() const
                {
                    return value_const_iterator(&items_[0]);
                }

                value_const_iterator value_cbegin() const
                {
                    return value_begin();
                }

                value_iterator value_end()
                {
                    return value_iterator(&items_[size_]);
                }

                value_const_iterator value_end() const
                {
                    return value_const_iterator(&items_[size_]);
                }

                value_const_iterator value_cend() const
                {
                    return value_end();
                }

            public:
                void reset(
                    size_type pos)
                {
                    int i = search(pos);
                    if (i >= 0) {
                        if (!(items_[i].value == EMPTY)) {
                            items_[i].value = EMPTY;
                            ++dirty_;
                        }
                    }
                }

                value_t & at(
                    size_type index)
                {
                    return alloc(index).value;
                }

                value_t const & at(
                    size_type index) const
                {
                    ptrdiff_t i = search(index);
                    return i >= 0 ? (value_t const &)items_[i].value : EMPTY;
                }

                value_t & operator[](
                    size_type index)
                {
                    return at(index);
                }

                value_t const & operator[](
                    size_type index) const
                {
                    return at(index);
                }

                void set(
                    size_type index, 
                    value_t const & value)
                {
                    assert(!(value == EMPTY));
                    alloc(index).value = value;
                }

                void dump(
                    std::ostream & os)
                {
                    os << "capacity: " << capacity_ << std::endl;
                    os << "size: " << size_ << std::endl;
                    os << "dirty: " << dirty_ << std::endl;
                    for (size_t i = 0; i < size_; ++i) {
                        os << "items[" << i << "] index: " << items_[i].index << ", value: " << items_[i].value << std::endl;
                    }
                }

            private:
                void gc()
                {
                    item_t * end = items_ + size_;
                    item_t * ptr = std::remove(items_, end, item_t(0, EMPTY));
                    for (; ptr < end; ++ptr) {
                        alloc_t::destroy(ptr);
                        --size_;
                    }
                    dirty_ = 0;
                }

                ptrdiff_t search(
                    size_t index) const
                {
                    ptrdiff_t lo = 0;
                    ptrdiff_t hi = (ptrdiff_t)size_ - 1;
                    while (lo <= hi) {
                        ptrdiff_t mid = (lo + hi) >> 1;
                        size_t midv = items_[mid].index;
                        if (midv < index)
                            lo = mid + 1;
                        else if (midv > index)
                            hi = mid - 1;
                        else
                            return mid;
                    }
                    return ~lo;
                }

                void insert(
                    size_t pos, 
                    item_t const & item)
                {
                    if (size_ + 1 <= capacity_) {
                        item_t * end = items_ + size_;
                        alloc_t::construct(end, item);
                        std::copy_backward(items_ + pos, end, end + 1);
                        items_[pos] = item;
                    } else {
                        size_t capacity = capacity_ < 4 ? 8 : capacity_ * 2;
                        item_t * items = alloc_t::allocate(capacity);
                        item_t const * from = items_;
                        item_t const * mid = items_ + pos;
                        item_t const * end = items_ + size_;
                        item_t * to = items;
                        for (; from < mid; ++from)
                            alloc_t::construct(to++, *from);
                        alloc_t::construct(to++, item);
                        for (; from < end; ++from)
                            alloc_t::construct(to++, *from);
                        items_ = items;
                        capacity_ = capacity;
                    }
                    ++size_;
                }

                item_t & alloc(
                    size_t index)
                {
                    ptrdiff_t i = search(index);
                    if (i < 0) {
                        i = ~i;
                        if (i < (ptrdiff_t)size_ && items_[i].value == EMPTY) {
                            items_[i].index = index;
                        } else {
                            if (dirty_ && size_ >= capacity_) {
                                gc();
                                i = ~search(index);
                            }
                            insert((size_t)i, item_t(index, EMPTY));
                        }
                    }
                    return items_[i];
                }

            private:
                size_t capacity_;
                size_t size_;
                size_t dirty_;
                item_t * items_;
                char qbuf_[sizeof(item_t) * 4];
                static value_t const EMPTY;
            };

        template <typename Value, typename Alloc>
            Value const SparseArray<Value, Alloc>::EMPTY = Value();

    } // namespace container
} // namespace framework 

#endif // _SPARSE_ARRAY_H_
