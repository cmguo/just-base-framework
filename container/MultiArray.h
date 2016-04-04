// MultiArray.h

#ifndef _FRAMEWORK_CONTAINER_MULTI_ARRAY_H_
#define _FRAMEWORK_CONTAINER_MULTI_ARRAY_H_

#include <framework/system/BitFields.h>

#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_pod.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/dynamic_bitset/dynamic_bitset.hpp>

#include <memory>

namespace framework {
namespace container {

namespace detail
{

    template <typename Elem, typename Bfs, typename Alloc, size_t BF_SHIFT = Bfs::BF_TYPE::BF_SHIFT>
    struct Array
        : Alloc::template rebind<Array<Elem, typename Bfs::BFS_CHILD, Alloc> >::other
    {
        typedef Elem elem_t;
        typedef Array array_t;
        typedef typename Bfs::BF_TYPE bitf_t;
        typedef typename bitf_t::BF_BASE_TYPE base_t;
        typedef typename bitf_t::BF_RESULT index_t;

        typedef Array<Elem, typename Bfs::BFS_CHILD, Alloc> item_t;
        typedef typename Alloc::template rebind<item_t>::other alloc_t;
        typedef typename alloc_t::pointer ptr_t;
        typedef typename alloc_t::const_pointer cptr_t;

        static index_t const ITEM_COUNT = bitf_t::BF_MAX;

        index_t live_;
        index_t full_;
        mutable index_t nget_;
        mutable index_t nset_;
        ptr_t array_[ITEM_COUNT];

        Array(int)
            : live_(0)
            , full_(0)
            , nget_(0)
            , nset_(0)
        {
            memset(array_, 0, sizeof(array_));
        }

        bool is_live() const
        {
            return live_ + full_;
        }

        bool is_full() const
        {
            return full_ == ITEM_COUNT;
        }


        template <bool C>
        class Iterator
            : public item_t::template Iterator<C>
        {
        public:
            typedef typename boost::mpl::if_c<C, 
                array_t const *, 
                array_t *
            >::type aptr_t;

            typedef typename item_t::template Iterator<C> super_t;

        public:
            Iterator(
                base_t n, 
                aptr_t p = NULL)
                : super_t(n, p ? p->at(bitf_t::get(n)) : ptr_t())
                , ptr_(p)
            {
            }

        protected:
            bool increment()
            {
                if (super_t::increment())
                    return true;
                index_t i = bitf_t::get(this->n_);
                if (i) {
                    if (ptr_)
                        super_t::set(ptr_->at(i));
                    return true;
                } else {
                    return false;
                }
            }

            bool decrement()
            {
                index_t i = bitf_t::get(this->n_);
                if (super_t::decrement())
                    return true;
                if (i--) {
                    if (ptr_)
                        super_t::set(ptr_->at(i));
                    return true;
                } else {
                    return false;
                }
            }

            bool update()
            {
                if (super_t::update())
                    return true;
                if (ptr_) {
                    index_t i = bitf_t::get(this->n_);
                    super_t::set(ptr_->at(i));
                    return true;
                } else {
                    return false;
                }
            }

            void set(
                aptr_t p)
            {
                ptr_ = p;
                if (ptr_) {
                    index_t i = bitf_t::get(this->n_);
                    super_t::set(ptr_->at(i));
                } else {
                    super_t::reset();
                }
            }

            void reset()
            {
                ptr_ = NULL;
                super_t::reset();
            }

        private:
            aptr_t ptr_;
        };

        void dump(
            base_t n, 
            char const * pre) const
        {
            std::cout << pre << "live: " << (int)live_ << std::endl;
            std::cout << pre << "full: " << (int)full_ << std::endl;
            pre -= 2;
            index_t m = bitf_t::get(n);
            if (m == 0) m = ITEM_COUNT;
            for (index_t i = 0; i < m; ++i) {
                std::cout << pre << "item " << (int)i << ": " << (void *)array_[i] << std::endl;
                if (array_[i])
                    array_[i]->dump(0, pre - 2);
            }
            if (bitf_t::get_sub(n)) {
                std::cout << pre << "item " << (int)m << ": " << (void *)array_[m] << std::endl;
                array_[m]->dump(n, pre - 2);
            }
        }

        void alloc(
            ptr_t & ptr)
        {
            if (!ptr) {
                ptr = alloc_t::allocate(1);
                alloc_t::construct(ptr, 0);
                ++live_;
            }
        }

        void free(
            ptr_t & ptr)
        {
            if (ptr) {
                --live_;
                alloc_t::destroy(ptr);
                alloc_t::deallocate(ptr, 1);
                ptr = ptr_t();
            }
        }

        elem_t const * get(
            base_t n) const
        {
            index_t i = bitf_t::get(n);
            ++nget_;
            return array_[i] ? (const_cast<cptr_t>(array_[i]))->get(n) : NULL;
        }
        
        elem_t * get(
            base_t n)
        {
            index_t i = bitf_t::get(n);
            this->alloc(array_[i]);
            ++nget_;
            return array_[i]->get(n);
        }

        bool set(
            base_t n, 
            elem_t const * elem)
        {
            index_t i = bitf_t::get(n);
            alloc(array_[i]);
            ++nset_;
            if (array_[i]->set(n, elem)) {
                ++full_;
                return is_full();
            } else {
                return false;
            }
        }

        ptr_t at(
            index_t i)
        {
            alloc(array_[i]);
            return array_[i];
        }

        cptr_t at(
            index_t i) const
        {
            return array_[i];
        }

        bool next_null(
            base_t & n) const
        {
            index_t i = bitf_t::get(n);
            if (!array_[i] || !array_[i]->next_null(n))
                return false;
            ++i;
            while (i < ITEM_COUNT && array_[i] && array_[i]->is_full())
                ++i;
            if (i < ITEM_COUNT) {
                n = bitf_t::set(n, i);
                array_[i]->next_null(n);
                return false;
            } else {
                n = bitf_t::set(n, 0);
                return true;
            }
        }

        void get_full_map(
            size_t l, 
            base_t f, 
            base_t t, 
            boost::dynamic_bitset<boost::uint32_t> & map) const
        {
            if (l == 0) {
                index_t i = bitf_t::get(f);
                index_t j = bitf_t::get_all(t) ? bitf_t::get(t) : ITEM_COUNT;
                if (this == NULL) {
                    map.resize(map.size() + j - i, false);
                } else if (is_full()) {
                    map.resize(map.size() + j - i, true);
                } else {
                    for (; i < j; ++i) {
                        map.push_back(array_[i] && array_[i]->is_full());
                    }
                }
                return;
            }
            --l;
            index_t i = bitf_t::get(f);
            index_t j = bitf_t::get_all(t) ? bitf_t::get(t) : ITEM_COUNT;
            if (i < j && bitf_t::get_sub(f)) {
                base_t m = bitf_t::inc(f);
                ptr_t ptr = (this == NULL) ? ptr_t() : array_[i];
                ptr->get_full_map(l, f, m, map);
                f = m;
                ++i;
            }
            for (; i < j; ++i) {
                ptr_t ptr = (this == NULL) ? ptr_t() : array_[i];
                ptr->get_full_map(l, f, f, map);
            }
            if (bitf_t::get_sub(t)) {
                ptr_t ptr = (this == NULL) ? ptr_t() : array_[i];
                ptr->get_full_map(l, f, t, map);
            }
        }

        base_t nget(
            size_t l, 
            base_t n) const
        {
            if (l == 0) {
                index_t n = nget_;
                nget_ = 0;
                return n;
            } else {
                index_t i = bitf_t::get(n);
                ptr_t ptr = array_[i];
                return (ptr == NULL) ? 0 : ptr->nget(l - 1, n);
            }
        }

        base_t nset(
            size_t l, 
            base_t n) const
        {
            if (l == 0) {
                index_t t = nset_;
                nset_ = 0;
                return t;
            } else {
                index_t i = bitf_t::get(n);
                ptr_t ptr = array_[i];
                return (ptr == NULL) ? 0 : ptr->nset(l - 1, n);
            }
        }

        void release(
            base_t f, 
            base_t t)
        {
            index_t i = bitf_t::get(f);
            index_t j = bitf_t::get_all(t) ? bitf_t::get(t) : ITEM_COUNT;
            if (i < j && bitf_t::get_sub(f)) {
                base_t m = bitf_t::inc(f);
                ptr_t & ptr(array_[i]);
                if (ptr) {
                    if (ptr->is_full()) --full_;
                    ptr->release(f, m);
                    if (!ptr->is_live())
                        free(ptr);
                }
                f = m;
                ++i;
            }
            for (; i < j; ++i) {
                ptr_t & ptr(array_[i]);
                if (ptr) {
                    if (ptr->is_full()) --full_;
                    free(ptr);
                }
            }
            if (bitf_t::get_sub(t)) {
                ptr_t & ptr(array_[i]);
                if (ptr) {
                    if (ptr->is_full()) --full_;
                    ptr->release(f, t);
                }
            }
        }

        void limit(
            base_t n)
        {
            index_t i = bitf_t::get(n);
            if (bitf_t::get_sub(n)) {
                this->alloc(array_[i]);
                array_[i]->limit(n);
                if (array_[i]->is_full())
                    ++full_;
                ++i;
            }
            full_ += ITEM_COUNT - i;
        }

        void unlimit(
            base_t n)
        {
            index_t i = bitf_t::get(n);
            if (bitf_t::get_sub(n)) {
                assert(array_[i]);
                if (array_[i]->is_full())
                    --full_;
                array_[i]->unlimit(n);
                if (!array_[i]->is_live())
                    free(array_[i]);
                ++i;
            }
            full_ -= ITEM_COUNT - i;
        }
    };

    template <typename Elem, typename Bfs, typename Alloc>
    struct Array<Elem, Bfs, Alloc, 0>
    {
        typedef Elem elem_t;
        typedef Array array_t;
        typedef typename Bfs::BF_TYPE bitf_t;
        typedef typename bitf_t::BF_BASE_TYPE base_t;
        typedef typename bitf_t::BF_RESULT index_t;

        typedef elem_t item_t;
        typedef typename Alloc::template rebind<item_t>::other alloc_t;
        typedef typename alloc_t::pointer ptr_t;
        typedef typename alloc_t::const_pointer cptr_t;

        static index_t const ITEM_COUNT = bitf_t::BF_MAX;

        index_t live_;
        index_t full_;
        mutable index_t nget_;
        mutable index_t nset_;
        item_t array_[ITEM_COUNT];

        Array(int)
            : live_(0)
            , full_(0)
            , nget_(0)
            , nset_(0)
        {
            if (boost::is_pod<elem_t>::value)
                memset(array_, 0, sizeof(array_));
        }

        bool is_live() const
        {
            return live_ + full_;
        }

        bool is_full() const
        {
            return full_ == ITEM_COUNT;
        }

        template <bool C>
        class Iterator
        {
        public:
            typedef typename boost::mpl::if_c<C, 
                array_t const *, 
                array_t *
            >::type aptr_t;

            typedef typename boost::mpl::if_c<C, 
                elem_t const *, 
                elem_t *
            >::type eptr_t;

            Iterator(
                base_t n)
                : n_(n)
                , ptr_(NULL)
            {
            }

            Iterator(
                base_t n, 
                aptr_t p)
                : n_(n)
                , ptr_(p)
            {
            }

        public:
            base_t index() const
            {
                return n_;
            }

            eptr_t elem() const
            {
                return ptr_ ? ptr_->get(n_) : NULL;
            }

        protected:
            bool increment()
            {
                return bitf_t::get(++n_) != 0;
            }

            bool decrement()
            {
                return bitf_t::get(n_--) != 0;
            }

            bool update()
            {
                return ptr_;
            }

            void set(
                aptr_t ptr)
            {
                ptr_ = ptr;
            }

            void clear()
            {
                ptr_ = NULL;
            }

        protected:
            base_t n_;

        private:
            aptr_t ptr_;
        };

        void dump(
            base_t n, 
            char const * pre) const
        {
            std::cout << pre << "live: " << (int)live_ << std::endl;
            std::cout << pre << "full: " << (int)full_ << std::endl;
            pre -= 2;
            index_t m = bitf_t::get(n);
            if (m == 0) m = ITEM_COUNT;
            for (index_t i = 0; i < m; ++i) {
                std::cout << pre << "item " << (int)i << ": " << array_[i] << std::endl;
            }
        }

        elem_t const * get(
            base_t n) const
        {
            index_t i = bitf_t::get(n);
            ++nget_;
            return &array_[i];
        }
        
        elem_t * get(
            base_t n)
        {
            index_t i = bitf_t::get(n);
            ++nget_;
            return &array_[i];
        }

        bool set(
            base_t n, 
            elem_t const * elem)
        {
            index_t i = bitf_t::get(n);
            if (array_[i] == elem_t()) {
                ++live_;
                ++full_;
            }
            array_[i] = *elem;
            ++nset_;
            return is_full();
        }

        bool next_null(
            base_t & n) const
        {
            index_t i = bitf_t::get(n);
            while (i < ITEM_COUNT && array_[i] != elem_t())
                ++i;
            if (i < ITEM_COUNT) {
                n = bitf_t::set(n, i);
                return false;
            } else {
                n = bitf_t::set(n, 0);
                return true;
            }
        }

        void get_full_map(
            size_t l, 
            base_t f, 
            base_t t, 
            boost::dynamic_bitset<boost::uint32_t> & map) const
        {
            assert(l == 0);
            index_t i = bitf_t::get(f);
            index_t j = bitf_t::get_all(t) ? bitf_t::get(t) : ITEM_COUNT;
            if (this == NULL) {
                map.resize(map.size() + j - i, false);
            } else if (is_full()) {
                map.resize(map.size() + j - i, true);
            } else {
                for (; i < j; ++i) {
                    map.push_back(array_[i] != elem_t());
                }
            }
        }

        index_t nget(
            size_t l, 
            base_t n) const
        {
            assert(l == 0);
            index_t t = nget_;
            nget_ = 0;
            return t;
        }

        index_t nset(
            size_t l, 
            base_t n) const
        {
            assert(l == 0);
            index_t t = nset_;
            nset_ = 0;
            return t;
        }

        void release(
            base_t f, 
            base_t t)
        {
            index_t i = bitf_t::get(f);
            index_t j = bitf_t::get_all(t) ? bitf_t::get(t) : ITEM_COUNT;
            for (; i < j; ++i) {
                if (array_[i] != item_t()) {
                    array_[i] = item_t();
                    --live_;
                    --full_;
                }
            }
        }

        void limit(
            base_t n)
        {
            index_t i = bitf_t::get(n);
            full_ += ITEM_COUNT - i;
        }

        void unlimit(
            base_t n)
        {
            index_t i = bitf_t::get(n);
            full_ -= ITEM_COUNT - i;
        }
    };

} // namespace detail

template <
    size_t N0, 
    size_t N1 = 0, 
    size_t N2 = 0, 
    size_t N3 = 0, 
    size_t N4 = 0, 
    size_t N5 = 0, 
    size_t N6 = 0, 
    size_t N7 = 0
>
struct MultiArrayDimension
{
    typedef framework::system::BitFields<N0 + N1 + N2 + N3 + N4 + N5 + N6 + N7, 
        N0, N1, N2, N3, N4, N5, N6, N7> BFS_TYPE;
};

template <typename Elem, typename Dim, typename Alloc = std::allocator<Elem> >
class MultiArray
    : Alloc::template rebind<detail::Array<Elem, typename Dim::BFS_TYPE, Alloc> >::other
{
public:
    typedef Elem elem_t;
    typedef typename Dim::BFS_TYPE bfs_t;
    typedef detail::Array<Elem, bfs_t, Alloc> array_t;
    typedef typename Alloc::template rebind<array_t>::other alloc_t;
    typedef typename array_t::base_t size_type;

    static size_type const MAX_SIZE = bfs_t::BF_TYPE::BF_MAX_ALL;

    template <size_t L>
        struct bit_field
        {
            typedef typename bfs_t::template bit_field<L>::type type;
        };

    template <bool C>
        class Iterator
            : public boost::iterator_facade<
                Iterator<C>, elem_t, 
                boost::random_access_traversal_tag, 
                typename boost::mpl::if_c<C, 
                    elem_t const &, 
                    elem_t &
                >::type, 
                size_type> 
            , array_t::template Iterator<C>
        {
        public:
            typedef typename boost::mpl::if_c<C, 
                array_t const *, 
                array_t *
            >::type aptr_t;

            typedef typename array_t::template Iterator<C> super_t;
            typedef Iterator<C> iter_t;
            typedef typename super_t::eptr_t eptr_t;

            typedef typename boost::mpl::if_c<C, 
                elem_t const &, 
                elem_t &
            >::type eref_t;

            Iterator(
                size_type n, 
                aptr_t ptr)
                : super_t(n, ptr)
                , ptr_(ptr)
            {
            }

            Iterator(
                size_type n)
                : super_t(n)
            {
            }

            friend class Iterator<!C>;

            Iterator(
                Iterator<!C> const & o)
                : super_t(o.n_, o.ptr_)
                , ptr_(o.ptr_)
            {
            }

        public:
            size_type index() const
            {
                return super_t::index();
            }

            void increment()
            {
                super_t::increment();
            }

            void decrement()
            {
                super_t::decrement();
            }

            void advance(
                boost::int64_t d)
            {
                this->n_ += d;
                super_t::set(ptr_);
            }

            eref_t dereference() const
            {
                eptr_t eptr(super_t::elem());
                if (!eptr) {
                    const_cast<iter_t *>(this)->update();
                    eptr = super_t::elem();
                }
                return eptr ? *eptr : elem_;
            }

            template <bool C1>
            bool equal(
                Iterator<C1> const & o) const
            {
                return index() == o.index();
            }

        private:
            aptr_t ptr_;
        };

    typedef Iterator<false> iterator;
    typedef Iterator<true> const_iterator;

    struct PrintableIndex
    {
        PrintableIndex(
            size_type index)
            : index_(index)
        {
        }

        size_type index_;

        template <class C, class T>
        friend std::basic_ostream<C, T> & operator<<(
            std::basic_ostream<C, T> & os, 
            PrintableIndex const & index)
        {
            Dim::BFS_TYPE::print(os, index.index_);
            return os;
        }
    };

public:
    MultiArray()
        : size_(MAX_SIZE)
        , array_(NULL)
    {
        array_ = alloc_t::allocate(1);
        alloc_t::construct(array_, 0);
    }

    ~MultiArray()
    {
        alloc_t::destroy(array_);
        alloc_t::deallocate(array_, 1);
        array_ = NULL;
    }

public:
    bool is_full() const
    {
        return array_->is_full();
    }

    size_type size() const
    {
        return size_;
    }

    iterator begin()
    {
        return iterator(0, array_);
    }

    const_iterator begin() const
    {
        return const_iterator(0, array_);
    }

    const_iterator cbegin() const
    {
        return const_iterator(0, array_);
    }

    iterator end()
    {
        return iterator(size_, NULL);
    }

    const_iterator end() const
    {
        return const_iterator(size_, NULL);
    }

    const_iterator cend() const
    {
        return const_iterator(size_, NULL);
    }

    void reset(
        size_type pos)
    {
        assert(pos < size_);
        reset(pos, pos + 1);
    }

    void reset(
        size_type beg, 
        size_type end)
    {
        assert(beg <= end);
        assert(end <= size_);
        array_->release(beg, end);
    }

    void dump() const
    {
        char const * prefix = "----------------";
        std::cout << "size: " << (int)size_ << std::endl;
        array_->dump(size_, prefix + 16);
    }

    static PrintableIndex print(
        size_type index)
    {
        return PrintableIndex(index);
    }

    void resize(
        size_type size)
    {
        if (size < size_)
            array_->release(size, size_);
        if (size_ != MAX_SIZE)
            array_->unlimit(size_);
        size_ = size;
        if (size_ != MAX_SIZE)
            array_->limit(size_);
    }

    elem_t & at(
        size_type index)
    {
        assert(index < size_);
        return *array_->get(index);
    }

    elem_t const & at(
        size_type index) const
    {
        assert(index < size_);
        elem_t const * elem(array_->get(index));
        return elem ? *elem : elem_;
    }

    elem_t const & cat(
        size_type index) const
    {
        return at(index);
    }

    elem_t & operator[](
        size_type index)
    {
        return at(index);
    }

    elem_t const & operator[](
        size_type index) const
    {
        return at(index);
    }

    void set(
        size_type index, 
        elem_t const & elem)
    {
        assert(index < size_);
        assert(elem != elem_);
        array_->set(index, &elem);
    }

    size_type next_null(
        size_type n) const
    {
        array_->next_null(n);
        return n;
    }

    void get_full_map(
        size_t level, 
        size_type from, 
        size_type to, 
        boost::dynamic_bitset<boost::uint32_t> & map)
    {
        array_->get_full_map(level, from, to, map);
    }

    size_type nget(
        size_t level, 
        size_type index) const
    {
        return array_->nget(level, index);
    }

    size_type nset(
        size_t level, 
        size_type index) const
    {
        return array_->nset(level, index);
    }

private:
    template <bool C>
    friend class Iterator;

    size_type size_;
    array_t * array_;
    static elem_t const elem_;
};

template <typename Elem, typename Dim, typename Alloc>
Elem const MultiArray<Elem, Dim, Alloc>::elem_ = Elem();

} // namespace container
} // namespace framework 

#endif // _FRAMEWORK_CONTAINER_MULTI_ARRAY_H_
