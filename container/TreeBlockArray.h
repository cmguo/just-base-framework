// TreeBlockArray.h

#ifndef _TREE_BLOCK_ARRAY_H_
#define _TREE_BLOCK_ARRAY_H_

#include "framework/container/Ordered.h"

#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_pod.hpp>
#include <boost/iterator/iterator_facade.hpp>

#include <memory>

namespace framework {
    namespace container {

        namespace detail
        {

            template <typename Value>
                struct EmptyValue
                {
                    static Value const value;
                };

            template <typename Value>
                Value const EmptyValue<Value>::value = Value();

            template <typename Value, size_t Block>
                struct TreeBlockArrayItem
                    : OrderedHook<TreeBlockArrayItem<Value, Block> >::type
                {
                    boost::uint64_t index;
                    Value values[Block];

                    TreeBlockArrayItem(
                        boost::uint64_t i)
                        : index(i)
                    {
                        if (boost::is_pod<Value>::value)
                            memset(values, 0, sizeof(values));
                    }

                    struct Key
                    {
                        typedef boost::uint64_t value_type;
                        typedef boost::uint64_t result_type;

                        boost::uint64_t operator()(
                            TreeBlockArrayItem const & i) const
                        {
                            return i.index;
                        }
                    };
                };

            template <typename T, bool C>
                struct TreeBlockIteratorType
                {
                    typedef typename boost::mpl::if_c<C, 
                            typename T::const_iterator,
                            typename T::iterator
                                >::type type;
                };

            template <typename T, bool C>
                struct TreeBlockReferenceType
                {
                    typedef typename boost::mpl::if_c<C, T const &, T &>::type type;
                };

            template <typename Value, size_t Block, bool C>
                struct TreeBlockIndexExtractor
                {
                    typedef boost::uint64_t value_type;
                    typedef typename TreeBlockReferenceType<boost::uint64_t , C>::type reference_type;
                    typedef typename TreeBlockReferenceType<TreeBlockArrayItem<Value, Block>, C>::type item_reference_type;

                    static reference_type extract(
                        item_reference_type item, 
                        size_t index)
                    {
                        return item.index + index;
                    }
                };

            template <typename Value, size_t Block, bool C>
                struct TreeBlockValueExtractor
                {
                    typedef Value value_type;
                    typedef typename TreeBlockReferenceType<Value, C>::type reference_type;
                    typedef typename TreeBlockReferenceType<TreeBlockArrayItem<Value, Block>, C>::type item_reference_type;

                    static reference_type extract(
                        item_reference_type item, 
                        size_t index)
                    {
                        return item.values[index];
                    }
                };

            template <typename Value, size_t Block, template <typename, size_t, bool> class Extractor, bool C>
                class TreeBlockArrayIterator
                : public boost::iterator_facade<
                  TreeBlockArrayIterator<Value, Block, Extractor, C>, 
                  typename Extractor<Value, Block, C>::value_type, 
                  boost::random_access_traversal_tag, 
                  typename Extractor<Value, Block, C>::reference_type, 
                  size_t> 
            {
            public:
                typedef TreeBlockArrayItem<Value, Block> item_t;
                typedef Ordered<item_t, typename item_t::Key> ordered_t;
                typedef typename TreeBlockIteratorType<ordered_t, C>::type item_iter_t;
                typedef Extractor<Value, Block, C> extractor_t;
                typedef typename extractor_t::reference_type reference_type;
                typedef boost::uint64_t size_type;

                TreeBlockArrayIterator(
                    item_iter_t iter, 
                    size_t idx = 0)
                    : iter_(iter)
                    , idx_(idx)
                {
                }

                friend class TreeBlockArrayIterator<Value, Block, Extractor, !C>;

                TreeBlockArrayIterator(
                    TreeBlockArrayIterator<Value, Block, Extractor, !C> const & o)
                    : iter_(o.iter_)
                    , idx_(o.idx_)
                {
                }

            public:
                size_type index() const
                {
                    return iter_->index + idx_;
                }

                void increment()
                {
                    if (++idx_ == Block) {
                        ++iter_;
                        idx_ = 0;
                    }
                }

                void decrement()
                {
                    if (--idx_ == size_t(-1)) {
                        --iter_;
                        idx_ = Block - 1;
                    }
                }

                void advance(
                    boost::int64_t d)
                {
                    assert(false);
                }

                reference_type dereference() const
                {
                    return extractor_t::extract(*iter_, idx_);
                }

                template <bool C1>
                    bool equal(
                        TreeBlockArrayIterator<Value, Block, Extractor, C1> const & o) const
                    {
                        return iter_ == o.iter_ && idx_ == o.idx_;
                    }

            private:
                item_iter_t iter_;
                size_t idx_;
            };

        } // namespace detail

        template <typename Value, size_t Block, typename Alloc = std::allocator<Value> >
            class TreeBlockArray
            : Alloc::template rebind<detail::TreeBlockArrayItem<Value, Block> >::other
            {
            public:
                typedef Value value_t;
                typedef boost::uint64_t size_type;
                typedef ptrdiff_t difference_type;
                typedef value_t & reference;
                typedef value_t const & const_reference;
                typedef value_t * pointer;
                typedef value_t const * const_pointer;
                typedef TreeBlockArray array_t;

            private:
                static size_t const block_size = Block;
                typedef detail::TreeBlockArrayItem<value_t, block_size> item_t;
                typedef typename Alloc::template rebind<item_t>::other alloc_t;
                typedef Ordered<item_t, typename item_t::Key> ordered_t;

            public:
                typedef detail::TreeBlockArrayIterator<Value, Block, detail::TreeBlockIndexExtractor, true> index_iterator;
                typedef detail::TreeBlockArrayIterator<Value, Block, detail::TreeBlockValueExtractor, true> value_const_iterator;
                typedef detail::TreeBlockArrayIterator<Value, Block, detail::TreeBlockValueExtractor, false> value_iterator;

            public:
                TreeBlockArray()
                {
                }

                ~TreeBlockArray()
                {
                    while (!items_.empty()) {
                        item_t * item = const_cast<item_t *>(items_.first());
                        items_.erase(item);
                        alloc_t::destroy(item);
                        alloc_t::deallocate(item, 1);
                    }
                }

            public:
                size_type size() const
                {
                    return 0;
                }

                index_iterator index_begin() const
                {
                    return index_iterator(items_.begin());
                }

                index_iterator index_end() const
                {
                    return index_iterator(items_.end());
                }

                value_iterator value_begin()
                {
                    return value_iterator(items_.begin());
                }

                value_const_iterator value_begin() const
                {
                    return value_const_iterator(items_.begin());
                }

                value_const_iterator value_cbegin() const
                {
                    return value_begin();
                }

                value_iterator value_end()
                {
                    return value_iterator(items_.end());
                }

                value_const_iterator value_end() const
                {
                    return value_const_iterator(items_.end());
                }

                value_const_iterator value_cend() const
                {
                    return value_end();
                }

            public:
                void reset(
                    size_type pos)
                {
                    value_t const * ptr = const_cast<array_t const *>(this)->get(pos);
                    if (ptr) {
                        if (!(*ptr == EMPTY)) {
                            *ptr = EMPTY;
                        }
                    }
                }

                value_t & at(
                    size_type index)
                {
                    value_t * ptr = get(index);
                    return *ptr;
                }

                value_t const & at(
                    size_type index) const
                {
                    value_t const * ptr = get(index);
                    return ptr ? *ptr : EMPTY;
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
                    at(index) = value;
                }

                void dump(
                    std::ostream & os)
                {
                    for (item_t * item = items_.first(); item; item = items_.next(item)) {
                        for (size_t i = 0; i < block_size; ++i) {
                            os << "items[" << item->index + i << "] value: " << item->values[i] << std::endl;
                        }
                    }
                }


            ordered_t & items()
            {
                return items_;
            }

            private:
                value_t * get(
                    size_type index)
                {
                    size_type index2 = index / block_size * block_size;
                    typename ordered_t::iterator iter = items_.find(index2);
                    if (iter == items_.end()) {
                        iter = items_.insert(iter, new item_t(index2)).first;
                    }
                    item_t & item = const_cast<item_t &>(*iter);
                    return &item.values[index - index2];
                }

                value_t const * get(
                    size_type index) const
                {
                    size_type index2 = index / block_size * block_size;
                    typename ordered_t::const_iterator iter = items_.find(index2);
                    if (iter == items_.end()) {
                        return NULL;
                    }
                    return &iter->values[index - index2];
                }

            private:
                ordered_t items_;
                static value_t const EMPTY;
            };

        template <typename Value, size_t Block, typename Alloc>
            Value const TreeBlockArray<Value, Block, Alloc>::EMPTY = Value();

    } // namespace container
} // namespace framework 

#endif // _TREE_BLOCK_ARRAY_H_
