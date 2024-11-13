#pragma once

#include <vector>
#include <shared_mutex>
#include <mutex>
#include <thread>
#include <memory>

namespace Invasion::Util
{
    template <typename Iterator>
    class AtomicIterator 
    {

    public:

        using iterator_category = typename std::iterator_traits<Iterator>::iterator_category;
        using value_type = typename std::iterator_traits<Iterator>::value_type;
        using difference_type = typename std::iterator_traits<Iterator>::difference_type;
        using pointer = typename std::iterator_traits<Iterator>::pointer;
        using reference = typename std::iterator_traits<Iterator>::reference;

        AtomicIterator(Iterator it, std::shared_ptr<std::shared_mutex> mtx) : iter_(it), mutex_(mtx) {}

        AtomicIterator() = delete;

        AtomicIterator(const AtomicIterator& other) : iter_(other.iter_), mutex_(other.mutex_) {}

        AtomicIterator(AtomicIterator&& other) noexcept : iter_(std::move(other.iter_)), mutex_(std::move(other.mutex_)) {}

        AtomicIterator& operator=(const AtomicIterator& other)
        {
            if (this != &other)
            {
                iter_ = other.iter_;
                mutex_ = other.mutex_;
            }

            return *this;
        }

        AtomicIterator& operator=(AtomicIterator&& other) noexcept 
        {
            if (this != &other) 
            {
                iter_ = std::move(other.iter_);
                mutex_ = std::move(other.mutex_);
            }

            return *this;
        }

        reference operator*() const 
        {
            std::shared_lock lock(*mutex_);

            return *iter_;
        }

        pointer operator->() const 
        {
            std::shared_lock lock(*mutex_);

            return iter_.operator->();
        }

        AtomicIterator& operator++() 
        {
            std::unique_lock lock(*mutex_);

            ++iter_;

            return *this;
        }

        AtomicIterator operator++(int) 
        {
            std::unique_lock lock(*mutex_);

            AtomicIterator temp = *this;
            ++iter_;

            return temp;
        }

        bool operator==(const AtomicIterator& other) const 
        {
            if (mutex_ == other.mutex_) 
            {
                std::shared_lock lock(*mutex_);
                return iter_ == other.iter_;
            }
            else 
            {
                std::scoped_lock lock(*mutex_, *other.mutex_);
                return iter_ == other.iter_;
            }
        }

        bool operator!=(const AtomicIterator& other) const 
        {
            return !(*this == other);
        }

        void set(const value_type& value) 
        {
            std::unique_lock lock(*mutex_);
            *iter_ = value;
        }

    private:
        Iterator iter_;
        std::shared_ptr<std::shared_mutex> mutex_;
    };
}