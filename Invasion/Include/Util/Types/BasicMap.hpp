#pragma once

#include <iostream>
#include <map>
#include <unordered_map>
#include <shared_mutex>
#include <memory>
#include <initializer_list>
#include <functional>
#include <algorithm>
#include <concepts>
#include <ranges>
#include <type_traits>
#include "Util/AtomicIterator.hpp"

namespace Invasion::Util::Types
{
    template <typename Container, typename Key, typename Value>
    concept MapContainer = requires(Container c, Key k, Value v) 
    {
        { c.insert(std::pair<Key, Value>{k, v}) } -> std::same_as<std::pair<typename Container::iterator, bool>>;
        { c.erase(k) } -> std::same_as<size_t>;
        { c.find(k) } -> std::same_as<typename Container::iterator>;
        { c.begin() } -> std::same_as<typename Container::iterator>;
        { c.end() } -> std::same_as<typename Container::iterator>;
        { c.size() } -> std::convertible_to<size_t>;
        { c.empty() } -> std::convertible_to<bool>;
    };

    template <template <typename, typename> class Container, typename Key, typename Value>
    concept SupportedMapContainer =
        std::is_same_v<Container<Key, Value>, std::map<Key, Value>> ||
        std::is_same_v<Container<Key, Value>, std::unordered_map<Key, Value>>;

    template <typename Key, typename Value, template <typename, typename> class Container = std::map> requires SupportedMapContainer<Container, Key, Value>
    class BasicMap
    {

    public:
        
        using ContainerType = Container<Key, Value>;

        BasicMap() = default;

        BasicMap(std::initializer_list<std::pair<Key, Value>> list)
        {
            std::unique_lock lock(*mutex);
            std::ranges::for_each(list, [&](const auto& pair)
            {
                data.insert(pair);
            });
        }

        template <template <typename, typename> class U> requires SupportedMapContainer<U, Key, Value>
        BasicMap(const BasicMap<Key, Value, U>& other) 
        {
            std::shared_lock lock(*other.mutex);

            std::ranges::for_each(other.data, [&](const auto& pair) 
            {
                data.insert(pair);
            });
        }

        template <template <typename, typename> class U> requires MapContainer<U<Key, Value>, Key, Value>
        BasicMap(const U<Key, Value>& other) noexcept
        {
            std::unique_lock lock(*mutex);

            std::ranges::for_each(other, [&](const auto& pair)
            {
                data.insert(pair);
            });
        }

        BasicMap(BasicMap&& other) noexcept 
        {
            std::unique_lock lock(*other.mutex);
            data = std::move(other.data);
        }

        BasicMap& operator=(BasicMap&& other) noexcept
        {
            if (this == &other)
                return *this;

            std::unique_lock lock_this(*mutex, std::defer_lock);
            std::unique_lock lock_other(*other.mutex, std::defer_lock);
            std::lock(lock_this, lock_other);

            data = std::move(other.data);
            return *this;
        }

        BasicMap& operator=(std::initializer_list<std::pair<Key, Value>> list)
        {
            std::unique_lock lock(*mutex);

            data.clear();

            std::ranges::for_each(list, [&](const auto& pair) 
            {
                data.insert(pair);
            });

            return *this;
        }

        template <template <typename, typename> class U> requires SupportedMapContainer<U, Key, Value>
        BasicMap& operator=(const BasicMap<Key, Value, U>& other) 
        {
            if (this == &other)
                return *this;

            std::unique_lock lock_this(*mutex, std::defer_lock);
            std::shared_lock lock_other(*other.mutex, std::defer_lock);

            std::lock(lock_this, lock_other);

            data.clear();
            std::ranges::for_each(other.data, [&](const auto& pair) 
            {
                data.insert(pair);
            });

            return *this;
        }

        template <template <typename, typename> class U> requires MapContainer<U<Key, Value>, Key, Value>
        BasicMap& operator=(const U<Key, Value>& other) {
            std::unique_lock lock(*mutex);
            data.clear();

            std::ranges::for_each(other, [&](const auto& pair) 
            {
                data.insert(pair);
            });

            return *this;
        }

        bool operator==(const BasicMap& other) const 
        {
            std::shared_lock lock(*mutex);
            std::shared_lock lock_other(*other.mutex);

            return data == other.data;
        }

        bool operator!=(const BasicMap& other) const 
        {
            return !(*this == other);
        }

        Value& operator[](const Key& key) 
        {
            std::unique_lock lock(*mutex);

            return data[key];
        }

        const Value& operator[](const Key& key) const
        {
            std::shared_lock lock(*mutex);

            return data.at(key);
        }

        template <template <typename, typename> class U> requires SupportedMapContainer<U, Key, Value>
        [[nodiscard]] BasicMap operator+(const BasicMap<Key, Value, U>& other) const 
        {
            BasicMap result = *this;

            result += other;

            return result;
        }

        template <template <typename, typename> class U> requires MapContainer<U<Key, Value>, Key, Value>
        [[nodiscard]] BasicMap operator+(const U<Key, Value>& other) const 
        {
            BasicMap result = *this;

            result += other;

            return result;
        }

        [[nodiscard]] BasicMap operator+(std::initializer_list<std::pair<Key, Value>> other) const 
        {
            BasicMap result = *this;

            result += other;

            return result;
        }

        [[nodiscard]] BasicMap operator+(const std::pair<Key, Value>& other) const 
        {
            BasicMap result = *this;

            result += other;

            return result;
        }

        template <template <typename, typename> class U> requires MapContainer<U<Key, Value>, Key, Value>
        BasicMap& operator+=(const BasicMap<Key, Value, U>& other) 
        {
            std::unique_lock lock(*mutex);
            std::shared_lock lock_other(*other.mutex);

            std::ranges::for_each(other.data, [&](const auto& pair)
            {
                data.insert(pair);
            });

            return *this;
        }

        template <template <typename, typename> class U> requires MapContainer<U<Key, Value>, Key, Value>
        BasicMap& operator+=(const U<Key, Value>& other)
        {
            std::unique_lock lock(*mutex);

            std::ranges::for_each(other, [&](const auto& pair) 
            {
                data.insert(pair);
            });

            return *this;
        }

        BasicMap& operator+=(std::initializer_list<std::pair<Key, Value>> other) 
        {
            std::unique_lock lock(*mutex);
            std::ranges::for_each(other, [&](const auto& pair) 
            {
                data.insert(pair);
            });

            return *this;
        }

        BasicMap& operator+=(const std::pair<Key, Value>& other)
        {
            std::unique_lock lock(*mutex);

            data.insert(other);

            return *this;
        }

        BasicMap& operator|=(const std::pair<Key, Value>& other)
        {
            std::unique_lock lock(*mutex);

            data.insert(std::move(other));

            return *this;
        }

        template <template <typename, typename> class U> requires SupportedMapContainer<U, Key, Value>
        [[nodiscard]] BasicMap operator-(const BasicMap<Key, Value, U>& other) const 
        {
            BasicMap result = *this;

            result -= other;

            return result;
        }

        template <template <typename, typename> class U> requires MapContainer<U<Key, Value>, Key, Value>
        [[nodiscard]] BasicMap operator-(const U<Key, Value>& other) const 
        {
            BasicMap result = *this;

            result -= other;

            return result;
        }

        [[nodiscard]] BasicMap operator-(std::initializer_list<Key> other) const
        {
            BasicMap result = *this;

            result -= other;

            return result;
        }

        [[nodiscard]] BasicMap operator-(const Key& other) const 
        {
            BasicMap result = *this;

            result -= other;

            return result;
        }

        template <template <typename, typename> class U> requires MapContainer<U<Key, Value>, Key, Value>
        BasicMap& operator-=(const BasicMap<Key, Value, U>& other) 
        {
            std::unique_lock lock(*mutex);
            std::shared_lock lock_other(*other.mutex);

            std::ranges::for_each(other.data, [&](const auto& pair) 
            {
                data.erase(pair.first);
            });

            return *this;
        }

        template <template <typename, typename> class U> requires MapContainer<U<Key, Value>, Key, Value>
        BasicMap& operator-=(const U<Key, Value>& other) 
        {
            std::unique_lock lock(*mutex);

            std::ranges::for_each(other, [&](const auto& pair) 
            {
                data.erase(pair.first);
            });

            return *this;
        }

        BasicMap& operator-=(std::initializer_list<Key> other) 
        {
            std::unique_lock lock(*mutex);

            std::ranges::for_each(other, [&](const auto& key) 
            {
                data.erase(key);
            });

            return *this;
        }

        BasicMap& operator-=(const Key& other) 
        {
            std::unique_lock lock(*mutex);

            data.erase(other);

            return *this;
        }

        [[nodiscard]] bool IsEmpty() const 
        {
            std::shared_lock lock(*mutex);

            return data.empty();
        }

        [[nodiscard]] size_t Length() const 
        {
            std::shared_lock lock(*mutex);

            return data.size();
        }

        void Clear() 
        {
            std::unique_lock lock(*mutex);

            data.clear();
        }

        [[nodiscard]] bool Contains(const Key& key) const 
        {
            std::shared_lock lock(*mutex);

            return data.find(key) != data.end();
        }

        template <typename C = ContainerType> requires (std::is_same_v<C, std::map<Key, Value>> || std::is_same_v<C, std::unordered_map<Key, Value>>)
        bool Contains(const Value& value) const 
        {
            std::shared_lock lock(*mutex);
            return std::ranges::find_if(data, [&](const auto& pair) { return pair.second == value; }) != data.end();
        }

        void ForEach(const std::function<void(const Key&, Value&)>& func) 
        {
            std::unique_lock lock(*mutex);

            std::ranges::for_each(data, [&](auto& pair) 
            {
                func(pair.first, pair.second);
            });
        }

        void ForEach(const std::function<void(const Key&, const Value&)>& func) const 
        {
            std::shared_lock lock(*mutex);

            std::ranges::for_each(data, [&](const auto& pair) 
            {
                func(pair.first, pair.second);
            });
        }

        void ForEach(const std::function<void(Value&)>& func) 
        {
            std::unique_lock lock(*mutex);

            std::ranges::for_each(data, [&](auto& pair) 
            {
                func(pair.second);
            });
        }

        void ForEach(const std::function<void(const Value&)>& func) const 
        {
            std::shared_lock lock(*mutex);

            std::ranges::for_each(data, [&](const auto& pair)
            {
                func(pair.second);
            });
        }

        void ForEach(const std::function<void(const Key&)>& func) 
        {
            std::unique_lock lock(*mutex);

            std::ranges::for_each(data, [&](auto& pair)
            {
                func(pair.first);
            });
        }

        void ForEach(const std::function<void()>& func) 
        {
            std::unique_lock lock(*mutex);

            std::ranges::for_each(data, [&](auto&)
            {
                func();
            });
        }

        void ForEach(const std::function<void(const Key&, Value&)>& func, const std::function<bool(const Key&, Value&)>& condition) 
        {
            std::unique_lock lock(*mutex);

            std::ranges::for_each(data, [&](auto& pair) 
            {
                if (condition(pair.first, pair.second)) 
                    func(pair.first, pair.second);
            });
        }

        void ForEach(const std::function<void(const Key&, const Value&)>& func, const std::function<bool(const Key&, const Value&)>& condition) const
        {
            std::shared_lock lock(*mutex);

            std::ranges::for_each(data, [&](const auto& pair) 
            {
                if (condition(pair.first, pair.second)) 
                    func(pair.first, pair.second);
            });
        }

        [[nodiscard]] AtomicIterator<typename ContainerType::iterator> begin() noexcept 
        {
            std::shared_lock lock(*mutex);

            return AtomicIterator<typename ContainerType::iterator>(data.begin(), mutex);
        }

        [[nodiscard]] AtomicIterator<typename ContainerType::iterator> end() noexcept 
        {
            std::shared_lock lock(*mutex);

            return AtomicIterator<typename ContainerType::iterator>(data.end(), mutex);
        }

        [[nodiscard]] AtomicIterator<typename ContainerType::const_iterator> cbegin() const noexcept 
        {
            std::shared_lock lock(*mutex);

            return AtomicIterator<typename ContainerType::const_iterator>(data.cbegin(), mutex);
        }

        [[nodiscard]] AtomicIterator<typename ContainerType::const_iterator> cend() const noexcept 
        {
            std::shared_lock lock(*mutex);

            return AtomicIterator<typename ContainerType::const_iterator>(data.cend(), mutex);
        }

        [[nodiscard]] AtomicIterator<typename ContainerType::iterator> find() const noexcept
        {
            std::shared_lock lock(*mutex);

            return AtomicIterator<typename ContainerType::iterator>(data.find(), mutex);
        }

        [[nodiscard]] AtomicIterator<typename ContainerType::iterator> find(const Key& key) noexcept
        {
            std::unique_lock lock(*mutex);
            return AtomicIterator<typename ContainerType::iterator>(data.find(key), mutex);
        }

        [[nodiscard]] AtomicIterator<typename ContainerType::const_iterator> find(const Key& key) const noexcept
        {
            std::shared_lock lock(*mutex);
            return AtomicIterator<typename ContainerType::const_iterator>(data.find(key), mutex);
        }

    private:

        ContainerType data;
        std::shared_ptr<std::shared_mutex> mutex = std::make_shared<std::shared_mutex>();

    };

    template <typename Key, typename Value, template <typename, typename> class Container>
    BasicMap(std::initializer_list<std::pair<Key, Value>>, Container<Key, Value>) -> BasicMap<Key, Value, Container>;

    template <typename Key, typename Value>
    BasicMap(std::initializer_list<std::pair<Key, Value>>) -> BasicMap<Key, Value>;
}