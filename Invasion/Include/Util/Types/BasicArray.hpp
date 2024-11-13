#pragma once

#include <vector>
#include <array>
#include <mutex>
#include <shared_mutex>
#include <algorithm>
#include <stdexcept>
#include <initializer_list>
#include <functional>
#include <type_traits>
#include "Util/AtomicIterator.hpp"

namespace Invasion::Util::Types
{
    template <typename Container>
    concept VectorContainer = requires(Container c)
    {
        typename Container::value_type;
        typename Container::allocator_type;
            requires std::same_as<Container, std::vector<typename Container::value_type, typename Container::allocator_type>>;
    };

    template <typename Container>
    concept ArrayContainer = requires(Container c)
    {
        typename Container::value_type;
        requires std::same_as<Container, std::array<typename Container::value_type, std::tuple_size<Container>::value>>;
    };

    template <typename T, typename Container>
    class BasicArray;

    template <typename T, typename Alloc> requires VectorContainer<std::vector<T, Alloc>>
    class BasicArray<T, std::vector<T, Alloc>>
    {

    public:

        using ContainerType = std::vector<T, Alloc>;
        using Iterator = typename ContainerType::iterator;
        using ConstIterator = typename ContainerType::const_iterator;

        explicit BasicArray(size_t size = 0) : data(size) {}

        BasicArray(std::initializer_list<T> list)
        {
            std::unique_lock lock(*mutex);
            data = list;
        }

        BasicArray(const BasicArray& other)
        {
            std::shared_lock lock(*other.mutex);
            data = other.data;
        }

        BasicArray(BasicArray&& other) noexcept
        {
            std::unique_lock lock(*other.mutex);
            data = std::move(other.data);
        }

        BasicArray& operator=(const BasicArray& other)
        {
            if (this != &other)
            {
                std::scoped_lock lock(*mutex, *other.mutex);
                data = other.data;
            }

            return *this;
        }

        BasicArray& operator=(BasicArray&& other) noexcept
        {
            if (this != &other)
            {
                std::scoped_lock lock(*mutex, *other.mutex);
                data = std::move(other.data);
            }

            return *this;
        }

        BasicArray& operator=(std::initializer_list<T> list)
        {
            std::unique_lock lock(*mutex);

            data = list;

            return *this;
        }

        BasicArray operator+(const BasicArray& other) const
        {
            BasicArray result(*this);

            result += other;

            return result;
        }

        BasicArray operator+(const T& value) const
        {
            BasicArray result(*this);

            result += value;

            return result;
        }

        BasicArray operator-(const BasicArray& other) const
        {
            BasicArray result(*this);

            result -= other;

            return result;
        }

        BasicArray operator-(const T& value) const
        {
            BasicArray result(*this);

            result -= value;

            return result;
        }

        BasicArray& operator+=(const BasicArray& other)
        {
            std::shared_lock lock_other(*other.mutex);
            std::unique_lock lock(*mutex);
            data.insert(data.end(), other.data.begin(), other.data.end());

            return *this;
        }

        BasicArray& operator+=(const T& value)
        {
            std::unique_lock lock(*mutex);
            data.push_back(value);

            return *this;
        }

        BasicArray& operator|=(T&& value)
        {
            std::unique_lock lock(*mutex);
            data.push_back(std::move(value));

            return *this;
        }

        BasicArray& operator-=(const BasicArray& other)
        {
            std::shared_lock lock_other(*other.mutex);
            std::unique_lock lock(*mutex);

            for (const auto& element : other.data)
                data.erase(std::remove(data.begin(), data.end(), element), data.end());
            
            return *this;
        }

        BasicArray& operator-=(const T& value)
        {
            std::unique_lock lock(*mutex);
            data.erase(std::remove(data.begin(), data.end(), value), data.end());

            return *this;
        }

        const T& operator[](size_t index) const
        {
            std::shared_lock lock(*mutex);
            if (index >= data.size())
                throw std::out_of_range("Index out of range");
            return data[index];
        }

        T& operator[](size_t index)
        {
            std::unique_lock lock(*mutex);
            if (index >= data.size())
                throw std::out_of_range("Index out of range");
            return data[index];
        }

        template <typename Func> requires std::invocable<Func, T&>
        void ForEach(Func&& func)
        {
            std::unique_lock lock(*mutex);
            for (auto& element : data)
                func(element);
        }

        template <typename Func> requires std::invocable<Func, const T&>
        void ForEach(Func&& func) const
        {
            std::shared_lock lock(*mutex);
            for (const auto& element : data)
                func(element);
        }

		template <typename Func> requires std::invocable<Func, T&>
		void ForEach(Func&& func, std::function<bool(const T&)> condition)
		{
			std::unique_lock lock(*mutex);

			for (auto& element : data)
			{
				if (condition(element))
					func(element);
			}
		}

        size_t Length() const noexcept
        {
            std::shared_lock lock(*mutex);
            return data.size();
        }

		bool Contains(const T& value) const
		{
			std::shared_lock lock(*mutex);

			return std::find(data.begin(), data.end(), value) != data.end();
		}

        void Resize(size_t size)
        {
            std::unique_lock lock(*mutex);
            data.resize(size);
        }

        bool IsEmpty() const noexcept
        {
            std::shared_lock lock(*mutex);
            return data.empty();
        }

        AtomicIterator<Iterator> begin() noexcept
        {
            return AtomicIterator<Iterator>(data.begin(), mutex);
        }

        AtomicIterator<Iterator> end() noexcept
        {
            return AtomicIterator<Iterator>(data.end(), mutex);
        }

        AtomicIterator<ConstIterator> cbegin() const noexcept
        {
            return AtomicIterator<ConstIterator>(data.cbegin(), mutex);
        }

        AtomicIterator<ConstIterator> cend() const noexcept
        {
            return AtomicIterator<ConstIterator>(data.cend(), mutex);
        }

        void Clear()
        {
            std::unique_lock lock(*mutex);
            data.clear();
        }

		operator ContainerType() const
		{
			std::shared_lock lock(*mutex);
			return data;
		}

		operator T* () noexcept
		{
			std::shared_lock lock(*mutex);
			return data.data();
		}

    private:

        mutable std::shared_ptr<std::shared_mutex> mutex = std::make_shared<std::shared_mutex>();
        ContainerType data;

    };

    template <typename T, std::size_t N> requires ArrayContainer<std::array<T, N>>
    class BasicArray<T, std::array<T, N>>
    {
    public:

        using ContainerType = std::array<T, N>;
        using Iterator = typename ContainerType::iterator;
        using ConstIterator = typename ContainerType::const_iterator;

        BasicArray() = default;

        BasicArray(std::initializer_list<T> list)
        {
            std::unique_lock lock(*mutex);
            if (list.size() > N)
                throw std::runtime_error("Initializer list size is greater than array size");
            std::copy(list.begin(), list.end(), data.begin());
        }

        BasicArray(const BasicArray& other)
        {
            std::shared_lock lock(*other.mutex);
            data = other.data;
        }

        BasicArray(BasicArray&& other) noexcept
        {
            std::unique_lock lock(*other.mutex);
            data = std::move(other.data);
        }

        BasicArray& operator=(const BasicArray& other)
        {
            if (this != &other)
            {
                std::scoped_lock lock(*mutex, *other.mutex);
                data = other.data;
            }

            return *this;
        }

        BasicArray& operator=(BasicArray&& other) noexcept
        {
            if (this != &other)
            {
                std::scoped_lock lock(*mutex, *other.mutex);
                data = std::move(other.data);
            }

            return *this;
        }

        BasicArray& operator=(std::initializer_list<T> list)
        {
            std::unique_lock lock(*mutex);
            if (list.size() > N)
                throw std::runtime_error("Initializer list size is greater than array size");
            std::copy(list.begin(), list.end(), data.begin());
            return *this;
        }

        const T& operator[](size_t index) const
        {
            std::shared_lock lock(*mutex);
            if (index >= N)
                throw std::out_of_range("Index out of range");
            return data[index];
        }

        T& operator[](size_t index)
        {
            std::unique_lock lock(*mutex);
            if (index >= N)
                throw std::out_of_range("Index out of range");
            return data[index];
        }

        template <typename Func> requires std::invocable<Func, T&>
        void ForEach(Func&& func)
        {
            std::unique_lock lock(*mutex);

            for (auto& element : data)
                func(element);
        }

        template <typename Func> requires std::invocable<Func, const T&>
        void ForEach(Func&& func) const
        {
            std::shared_lock lock(*mutex);

            for (const auto& element : data)
                func(element);
        }

        constexpr size_t Length() const noexcept
        {
            return N;
        }

        bool IsEmpty() const noexcept
        {
            std::shared_lock lock(*mutex);
            return data.empty();
        }

        AtomicIterator<Iterator> begin() noexcept
        {
            return AtomicIterator<Iterator>(data.begin(), mutex);
        }

        AtomicIterator<Iterator> end() noexcept
        {
            return AtomicIterator<Iterator>(data.end(), mutex);
        }

        AtomicIterator<ConstIterator> cbegin() const noexcept
        {
            return AtomicIterator<ConstIterator>(data.cbegin(), mutex);
        }

        AtomicIterator<ConstIterator> cend() const noexcept
        {
            return AtomicIterator<ConstIterator>(data.cend(), mutex);
        }

        void Clear()
        {
            std::unique_lock lock(*mutex);
            data.fill(T());
        }

		operator ContainerType() const
		{
			std::shared_lock lock(*mutex);
			return data;
		}

        operator T* () noexcept
        {
			std::shared_lock lock(*mutex);
			return data.data();
        }

    private:

        mutable std::shared_ptr<std::shared_mutex> mutex = std::make_shared<std::shared_mutex>();

        ContainerType data{};
    };
}

namespace std
{
    template <typename T, typename Container>
    struct hash<Invasion::Util::Types::BasicArray<T, Container>>
    {
        size_t operator()(const Invasion::Util::Types::BasicArray<T, Container>& array) const
        {
            size_t result = 0;

            array.ForEach([&result](const T& element)
            {
                result ^= std::hash<T>{}(element)+0x9e3779b9 + (result << 6) + (result >> 2);
            });

            return result;
        }
    };

    template <typename T, typename Container>
    struct equal_to<Invasion::Util::Types::BasicArray<T, Container>>
    {
        bool operator()(const Invasion::Util::Types::BasicArray<T, Container>& lhs, const Invasion::Util::Types::BasicArray<T, Container>& rhs) const
        {
            if (lhs.Length() != rhs.Length())
                return false;

            for (size_t i = 0; i < lhs.Length(); ++i)
            {
                if (lhs[i] != rhs[i])
                    return false;
            }

            return true;
        }
    };

    template <typename T, typename Container>
    struct less<Invasion::Util::Types::BasicArray<T, Container>>
    {
        bool operator()(const Invasion::Util::Types::BasicArray<T, Container>& lhs, const Invasion::Util::Types::BasicArray<T, Container>& rhs) const
        {
            size_t min_length = std::min(lhs.Length(), rhs.Length());

            for (size_t i = 0; i < min_length; ++i)
            {
                if (lhs[i] < rhs[i])
                    return true;
                if (rhs[i] < lhs[i])
                    return false;
            }

            return lhs.Length() < rhs.Length();
        }
    };

    template <typename T, typename Container>
    struct formatter<Invasion::Util::Types::BasicArray<T, Container>> : std::formatter<std::string>
    {
        template <typename FormatContext>
        auto format(const Invasion::Util::Types::BasicArray<T, Container>& array, FormatContext& ctx)
        {
            std::string result = "{";
            size_t len = array.Length();

            for (size_t i = 0; i < len; ++i)
            {
                result += std::format("{}", array[i]);
                if (i < len - 1)
                    result += ", ";
            }

            result += "}";

            return std::formatter<std::string>::format(result, ctx);
        }
    };
}