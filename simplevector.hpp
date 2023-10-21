// GPL-3.0-only

#pragma once

#include <algorithm>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace simplecontainer {

// @TODO: Add unit tests
// @TODO: Add cmake script
// @TODO: Check capacity_, size_ and data_ usage when they can be 0, 0 and nullptr
// @TODO: Use C++20 Concepts instead of the tag dispatch mechanism in iterators
// @TODO: Add rbegin() and crbegin() to iterators
// @TODO: Move allocation strategy to service function from push()
// @TODO: Update code for exception in push()
// @TODO: Add erase() and clear()
// @TODO: Think about data()
// @TODO: Add Doxygen comments
// @TODO: Add Licence

// Storage: exceptions safe dynamical allocated block storage
// Vector: dynamical array with elements access

//------------------------------------------------------------------------------------------------//

template<class T> class SimpleVectorStorage {
protected:
    size_t capacity_ = 0; // Allocated number of elements (not bytes)
    size_t size_ = 0; // Current inserted number of elements (not bytes)
    T *data_ = nullptr;

    SimpleVectorStorage() = default;

    SimpleVectorStorage(const SimpleVectorStorage&) = delete;
    SimpleVectorStorage &operator=(const SimpleVectorStorage&) = delete;

    SimpleVectorStorage(size_t quantity) :
        capacity_(quantity),
        size_{0}, // Not necessary but prefer to use
        data_{static_cast<T*>(::operator new(sizeof(T) * quantity))} {}

    SimpleVectorStorage(SimpleVectorStorage&& other) noexcept {
        std::swap(capacity_, other.capacity_);
        std::swap(size_, other.size_);
        std::swap(data_, other.data_);
    }

    SimpleVectorStorage &operator=(SimpleVectorStorage&& other) noexcept {
        std::swap(capacity_, other.capacity_);
        std::swap(size_, other.size_);
        std::swap(data_, other.data_);
        return *this;
    }

    ~SimpleVectorStorage() {
        for (size_t i = 0; i < size_; ++i) {
            data_[i].~T();
        }
        ::operator delete(data_);
    }
}; // class SimpleVectorStorage

//------------------------------------------------------------------------------------------------//

template<class T> class SimpleVector final : private SimpleVectorStorage<T> {
    static_assert(std::is_nothrow_move_constructible<T>::value
               || std::is_copy_constructible<T>::value,
                  "T must be copy constructible or nothrow moveable.");

private:
    using SimpleVectorStorage<T>::capacity_;
    using SimpleVectorStorage<T>::size_;
    using SimpleVectorStorage<T>::data_;

private:
    class empty_obj final {};

    using CopyArg = std::conditional_t<std::is_copy_constructible_v<T>,
                                       const SimpleVector&,
                                       empty_obj>;

    template<class Arg> using CopyCtor = std::void_t<decltype(Arg(std::declval<Arg&>()))>;
    template<class Arg> using DefaultCtor = std::void_t<decltype(Arg())>;

private:
    template<class Arg> using UseMove = std::enable_if_t<std::is_nothrow_move_constructible_v<Arg>, 
                                                         int>;
    template<class Arg> using UseCopy = std::enable_if_t<!std::is_nothrow_move_constructible_v<Arg>,
                                                         int>;

    template<class Arg = T, UseMove<Arg> = 0>
    void move_or_copy(T *dest, T &src) {
        new (dest) T(src);
        return;
    }

    template<class Arg = T, UseCopy<Arg> = 0>
    void move_or_copy(T *dest, T &src) {
        new (dest) T(src);
        return;
    }

    void reallocate_size(size_t size) {
        size_t new_size = std::min(size, size_);
        SimpleVector new_simple_vector(size, empty_obj{});

        for (; new_simple_vector.size_ < new_size; ++new_simple_vector.size_) {
            move_or_copy(new_simple_vector.data_ + new_simple_vector.size_,
                         data_[new_simple_vector.size_]);
        }

        *this = std::move(new_simple_vector);
        return;
    }

    inline size_t get_new_allocation_size() {
        size_t new_size = capacity_;
        // Possible Allocation Strategy:
        // 1) capacity_ + std::max(capacity_, n_)
        // 2) 2 *  capacity_ + 1
        // 3) etc.
        // Prefer to use (1)
        const size_t N = 20;
        if(__builtin_add_overflow(capacity_, std::max(capacity_, N), &new_size)) {
            throw std::overflow_error("Too big values (Overflow Error).");
        }
        
        return new_size;
    }

private:
    SimpleVector(size_t size, empty_obj) : SimpleVectorStorage<T>{size} {}

public:
    static void swap(SimpleVector &first, SimpleVector &second) noexcept {
        std::swap(first.data_, second.data_);
        std::swap(first.size_, second.size_);
        std::swap(first.capacity_, second.capacity_);
        return;
    }

public:
    class iterator final {
        friend SimpleVector;
        T *ptr_;

        iterator(T *ptr) noexcept : ptr_(ptr) {}

    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = int;
        using difference_type = ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;

        iterator(const iterator &) = default;

        iterator &operator=(const iterator &) = default;

        T &operator*() const noexcept {
            return *ptr_;
        }

        iterator &operator++() noexcept {
            ++ptr_;
            return *this;
        }

        iterator operator++(int) noexcept {
            iterator copy{*this};
            ++ptr_;
            return copy;
        }

        iterator &operator--() noexcept {
            --ptr_;
            return *this;
        }

        iterator operator--(int) noexcept {
            iterator copy{*this};
            --ptr_;
            return copy;
        }

        bool operator==(const iterator &second) const noexcept = default;
    }; // class iterator

    class const_iterator final {
        iterator it_;

    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = int;
        using difference_type = ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;

        const_iterator(const iterator &it) noexcept : it_(it) {}

        const_iterator(const const_iterator &) = default;

        const_iterator &operator=(const const_iterator &) = default;

        const T &operator*() const noexcept {
            return *it_;
        }

        const_iterator &operator++() {
            ++it_;
            return *this;
        }

        const_iterator operator++(int) {
            return it_++;
        }

        const_iterator &operator--() {
            --it_;
            return *this;
        }

        const_iterator operator--(int) {
            return --it_;
        }

        bool operator==(const const_iterator &sd) const noexcept = default;
    }; // class const iterator

    iterator begin() noexcept {
        return iterator(data_);
    }

    iterator end() noexcept {
        return iterator(data_ + size_);
    }

    const_iterator begin() const noexcept {
        return const_iterator(data_);
    }

    const_iterator end() const noexcept {
        return const_iterator(data_ + size_);
    }

    const_iterator cbegin() const noexcept {
        return const_iterator(data_);
    }

    const_iterator cend() const noexcept {
        return const_iterator(data_ + size_);
    }

public:
    SimpleVector() = default;
    SimpleVector(SimpleVector&&) = default;
    SimpleVector &operator=(SimpleVector&&) = default;

    SimpleVector(CopyArg other) : SimpleVectorStorage<T>{other.size_} {
        for (; size_ < other.size_; ++size_) {
            new (data_ + size_) T(other.data_[size_]);
        }
    }

    SimpleVector &operator=(CopyArg other) {
        SimpleVectorStorage tmp(other);
        return *this = std::move(tmp);
    }

    template<class Arg = T, class = CopyCtor<Arg>>
    SimpleVector(std::initializer_list<T> init_list) : SimpleVectorStorage<T>{init_list.size()} {
        static_assert(std::is_same_v<Arg, T>, "Arg should be default type T.");

        for (auto it = init_list.begin(), end = init_list.end(); it != end; ++it, ++size_) {
            new (data_ + size_) T(*it);
        }
    }

    template<class Arg = T, class = CopyCtor<Arg>>
    SimpleVector(size_t size, const T& other) : SimpleVectorStorage<T>{size} {
        static_assert(std::is_same_v<Arg, T>, "Arg should be default type T.");

        for (; size_ < size; ++size_) {
            new (data_ + size_) T(other);
        }
    }

    template<class Arg = T, class = DefaultCtor<Arg>>
    SimpleVector(size_t size) : SimpleVectorStorage<T>{size} {
        static_assert(std::is_same_v<Arg, T>, "Arg should be default type T.");

        for (; size_ < size; ++size_) {
            new (data_ + size_) T{};
        }
    }

    ~SimpleVector() = default;

public:
    template<class Arg> void push(Arg&& value) {
        if (size_ == capacity_) {
            size_t new_size = capacity_;
            try {
                new_size = get_new_allocation_size();
            }
            catch (const std::overflow_error& e) {
                throw;
            }
            catch (...) {
                throw;
            }
            reallocate_size(new_size);
        }

        new (data_ + size_) T{std::forward<Arg>(value)};
        ++size_;
        return;
    }

    void pop() {
        if (size_ == 0) {
            throw std::invalid_argument("SimpleVector is empty.");
        }

        data_[size_ - 1].~T();
        --size_;
        return;
    }

    T &back() noexcept {
        return data_[size_ - 1];
    }

    const T &back() const noexcept {
        return data_[size_ - 1];
    }

    T &front() noexcept {
        return *data_;
    }

    const T &front() const noexcept {
        return *data_;
    }
    
    T &operator[](size_t id) noexcept {
        return data_[id];
    }

    const T &operator[](size_t id) const noexcept {
        return data_[id];
    }

    inline size_t capacity() const noexcept {
        return sizeof(T) * capacity_;
    }

    inline size_t size() const noexcept {
        return size_;
    }

    T& at(size_t id) {
        if (size_ == 0) {
            throw std::invalid_argument("SimpleVector is empty.");
        }
        if (id >= size_) {
            throw std::invalid_argument("SimpleVector bad index.");
        }
        
        return data_[id];
    }

    const T& at(size_t id) const {
        if (size_ == 0) {
            throw std::invalid_argument("SimpleVector is empty.");
        }
        if (id >= size_) {
            throw std::invalid_argument("SimpleVector bad index.");
        }
        
        return data_[id];
    }
    
    bool empty() const noexcept {
        if (size_ == 0) {
            return true;
        }
        return false;
    }
}; // class SimpleVector

//------------------------------------------------------------------------------------------------//

} // namespace simplecontainer
