#pragma once

#include <mutex>
#include <shared_mutex>
#include <memory>
#include <string>
#include <algorithm>
#include <iostream>
#include <type_traits>
#include <cassert>
#include <format>
#include "Util/AtomicIterator.hpp"

namespace Invasion::Util::Types
{
    template <typename T>
    concept CharType = std::same_as<T, char> ||
        std::same_as<T, wchar_t> ||
        std::same_as<T, char16_t> ||
        std::same_as<T, char32_t>;

    template <typename From, typename To>
    concept ConvertibleCharTypes = CharType<From> && CharType<To>;

    template <CharType T>
    class BasicString 
    {

    public:

        BasicString() = default;

        ~BasicString() = default;

        BasicString(const BasicString& other) noexcept 
        {
            std::shared_lock lock(*other.mutex);
            data = other.data;
            mutex = std::make_shared<std::shared_mutex>();
        }

        BasicString(BasicString&& other) noexcept 
        {
            std::unique_lock lock(*other.mutex);
            data = std::move(other.data);
            mutex = std::make_shared<std::shared_mutex>();
        }

        template <typename U> requires ConvertibleCharTypes<U, T>
        BasicString(const std::basic_string<U>& str) noexcept 
        {
            data = Convert<U, T>(str);
            mutex = std::make_shared<std::shared_mutex>();
        }

        template <typename U> requires ConvertibleCharTypes<U, T>
        BasicString(const U* str) noexcept 
        {
            data = Convert<U, T>(std::basic_string<U>(str));
            mutex = std::make_shared<std::shared_mutex>();
        }

		template <typename U> requires ConvertibleCharTypes<U, T>
        BasicString(const U& other) noexcept
        {
			data = Convert<U, T>(std::basic_string<U>(1, other));
			mutex = std::make_shared<std::shared_mutex>();
        }

        BasicString& operator=(BasicString&& other) noexcept 
        {
            if (this != &other) 
            {
                std::scoped_lock lock(*mutex, *other.mutex);
                data = std::move(other.data);
            }

            return *this;
        }

        template <typename U> requires ConvertibleCharTypes<U, T>
        BasicString& operator=(const BasicString<U>& input)
        {
            std::unique_lock lock(*mutex);

            data = Convert<U, T>(input);

            return *this;
        }

        template <typename U> requires ConvertibleCharTypes<U, T>
        BasicString& operator=(const std::basic_string<U>& input)
        {
            std::unique_lock lock(*mutex);

            data = Convert<U, T>(input);

            return *this;
        }

        template <typename U> requires ConvertibleCharTypes<U, T>
        BasicString& operator=(const U* str) 
        {
            std::unique_lock lock(*mutex);

            data = Convert<U, T>(std::basic_string<U>(str));

            return *this;
        }

        BasicString& operator=(const BasicString& other) noexcept 
        {
            if (this != &other) 
            {
                std::scoped_lock lock(*mutex, *other.mutex);
                data = other.data;
            }

            return *this;
        }

        template <typename U> requires ConvertibleCharTypes<U, T>
        bool operator==(const BasicString<U>& other) const
        {
            std::shared_lock lock(*mutex);

            return data == Convert<U, T>(other);
        }

        template <typename U> requires ConvertibleCharTypes<U, T>
        bool operator==(const std::basic_string<U>& other) const 
        {
            std::shared_lock lock(*mutex);

            return data == Convert<U, T>(other);
        }

        template <typename U> requires ConvertibleCharTypes<U, T>
        bool operator==(const U* other) const 
        {
            std::shared_lock lock(*mutex);

            return data == Convert<U, T>(std::basic_string<U>(other));
        }

        template <typename U> requires ConvertibleCharTypes<U, T>
        bool operator!=(const BasicString<U>& other) const 
        {
            return !(*this == other);
        }

        template <typename U> requires ConvertibleCharTypes<U, T>
        bool operator!=(const std::basic_string<U>& other) const 
        {
            return !(*this == other);
        }

        template <typename U> requires ConvertibleCharTypes<U, T>
        bool operator!=(const U* other) const 
        {
            return !(*this == other);
        }

        template <typename U> requires ConvertibleCharTypes<U, T>
        bool operator<(const BasicString<U>& other) const 
        {
            std::shared_lock lock(*mutex);

            return data < Convert<U, T>(other);
        }

        template <typename U> requires ConvertibleCharTypes<U, T>
        bool operator<(const std::basic_string<U>& other) const 
        {
            std::shared_lock lock(*mutex);

            return data < Convert<U, T>(other);
        }

        template <typename U> requires ConvertibleCharTypes<U, T>
        bool operator<(const U* other) const 
        {
            std::shared_lock lock(*mutex);

            return data < Convert<U, T>(std::basic_string<U>(other));
        }

        template <typename U> requires ConvertibleCharTypes<U, T>
        bool operator<=(const BasicString<U>& other) const
        {
            std::shared_lock lock(*mutex);

            return data <= Convert<U, T>(other);
        }

        template <typename U> requires ConvertibleCharTypes<U, T>
        bool operator<=(const std::basic_string<U>& other) const
        {
            std::shared_lock lock(*mutex);
            return data <= Convert<U, T>(other);
        }

        template <typename U> requires ConvertibleCharTypes<U, T>
        bool operator<=(const U* other) const 
        {
            std::shared_lock lock(*mutex);
            return data <= Convert<U, T>(std::basic_string<U>(other));
        }

        template <typename U> requires ConvertibleCharTypes<U, T>
        bool operator>(const BasicString<U>& other) const 
        {
            std::shared_lock lock(*mutex);
            return data > Convert<U, T>(other);
        }

        template <typename U> requires ConvertibleCharTypes<U, T>
        bool operator>(const std::basic_string<U>& other) const
        {
            std::shared_lock lock(*mutex);

            return data > Convert<U, T>(other);
        }

        template <typename U> requires ConvertibleCharTypes<U, T>
        bool operator>(const U* other) const 
        {
            std::shared_lock lock(*mutex);

            return data > Convert<U, T>(std::basic_string<U>(other));
        }

        template <typename U> requires ConvertibleCharTypes<U, T>
        bool operator>=(const BasicString<U>& other) const 
        {
            std::shared_lock lock(*mutex);

            return data >= Convert<U, T>(other);
        }

        template <typename U> requires ConvertibleCharTypes<U, T>
        bool operator>=(const std::basic_string<U>& other) const 
        {
            std::shared_lock lock(*mutex);

            return data >= Convert<U, T>(other);
        }

        template <typename U> requires ConvertibleCharTypes<U, T>
        bool operator>=(const U* other) const 
        {
            std::shared_lock lock(*mutex);

            return data >= Convert<U, T>(std::basic_string<U>(other));
        }

        template <typename U> requires ConvertibleCharTypes<U, T>
        [[nodiscard]] BasicString operator+(const BasicString<U>& other) const 
        {
            BasicString<T> otherConverted(other);

            std::scoped_lock lock(*mutex, *otherConverted.mutex);

            BasicString<T> result;

            result.data = this->data + otherConverted.data;

            return result;
        }

        template <typename U> requires ConvertibleCharTypes<U, T>
        [[nodiscard]] BasicString operator+(const std::basic_string<U>& other) const 
        {
            BasicString<T> otherConverted(other);

            std::scoped_lock lock(*mutex, *otherConverted.mutex);

            BasicString<T> result;

            result.data = this->data + otherConverted.data;

            return result;
        }

        template <typename U> requires ConvertibleCharTypes<U, T>
        [[nodiscard]] BasicString operator+(const U* other) const 
        {
            BasicString<T> otherConverted(other);

            std::scoped_lock lock(*mutex, *otherConverted.mutex);

            BasicString<T> result;

            result.data = this->data + otherConverted.data;

            return result;
        }

        template <typename U> requires ConvertibleCharTypes<U, T>
		[[nodiscard]] BasicString operator+(const U& other) const
		{
			BasicString<T> otherConverted(other);

			std::scoped_lock lock(*mutex, *otherConverted.mutex);

			BasicString<T> result;

			result.data = this->data + otherConverted.data;

			return result;
		}

        template <typename U> requires ConvertibleCharTypes<U, T>
        BasicString& operator+=(const BasicString<U>& other) 
        {
            BasicString<T> otherConverted(other);

            std::scoped_lock lock(*mutex, *otherConverted.mutex);
            this->data += otherConverted.data;

            return *this;
        }

        template <typename U> requires ConvertibleCharTypes<U, T>
        BasicString& operator+=(const std::basic_string<U>& other) 
        {
            BasicString<T> otherConverted(other);

            std::scoped_lock lock(*mutex, *otherConverted.mutex);
            this->data += otherConverted.data;

            return *this;
        }

        template <typename U> requires ConvertibleCharTypes<U, T>
        BasicString& operator+=(const U* other) 
        {
            BasicString<T> otherConverted(other);

            std::scoped_lock lock(*mutex, *otherConverted.mutex);
            this->data += otherConverted.data;

            return *this;
        }

        template <typename U> requires ConvertibleCharTypes<U, T>
		BasicString& operator+=(const U& other)
		{
			BasicString<T> otherConverted(other);

			std::scoped_lock lock(*mutex, *otherConverted.mutex);
			this->data += otherConverted.data;

			return *this;
		}

        template <typename U> requires ConvertibleCharTypes<U, T>
        [[nodiscard]] BasicString operator-(const BasicString<U>& other) const 
        {
            BasicString<T> otherConverted(other);
            std::scoped_lock lock(*mutex, *otherConverted.mutex);
            BasicString<T> result = *this;
            size_t pos = result.data.find(otherConverted.data);

            while (pos != std::basic_string<T>::npos) 
            {
                result.data.erase(pos, otherConverted.data.length());
                pos = result.data.find(otherConverted.data, pos);
            }

            return result;
        }

        template <typename U> requires ConvertibleCharTypes<U, T>
        [[nodiscard]] BasicString operator-(const std::basic_string<U>& other) const 
        {
            BasicString<T> otherConverted(other);
            std::scoped_lock lock(*mutex, *otherConverted.mutex);
            BasicString<T> result = *this;
            size_t pos = result.data.find(otherConverted.data);

            while (pos != std::basic_string<T>::npos) 
            {
                result.data.erase(pos, otherConverted.data.length());
                pos = result.data.find(otherConverted.data, pos);
            }

            return result;
        }

        template <typename U> requires ConvertibleCharTypes<U, T>
        [[nodiscard]] BasicString operator-(const U* other) const 
        {
            BasicString<T> otherConverted(other);
            std::scoped_lock lock(*mutex, *otherConverted.mutex);
            BasicString<T> result = *this;
            size_t pos = result.data.find(otherConverted.data);

            while (pos != std::basic_string<T>::npos) 
            {
                result.data.erase(pos, otherConverted.data.length());
                pos = result.data.find(otherConverted.data, pos);
            }

            return result;
        }

		template <typename U> requires ConvertibleCharTypes<U, T>
		[[nodiscard]] BasicString operator-(const U& other) const
		{
			BasicString<T> otherConverted(other);
			std::scoped_lock lock(*mutex, *otherConverted.mutex);
			BasicString<T> result = *this;
			size_t pos = result.data.find(otherConverted.data);

			while (pos != std::basic_string<T>::npos)
			{
				result.data.erase(pos, otherConverted.data.length());
				pos = result.data.find(otherConverted.data, pos);
			}

			return result;
		}

        template <typename U> requires ConvertibleCharTypes<U, T>
        BasicString& operator-=(const BasicString<U>& other) 
        {
            BasicString<T> otherConverted(other);
            std::scoped_lock lock(*mutex, *otherConverted.mutex);
            size_t pos = data.find(otherConverted.data);

            while (pos != std::basic_string<T>::npos) 
            {
                data.erase(pos, otherConverted.data.length());
                pos = data.find(otherConverted.data, pos);
            }

            return *this;
        }

        template <typename U> requires ConvertibleCharTypes<U, T>
        BasicString& operator-=(const std::basic_string<U>& other) 
        {
            BasicString<T> otherConverted(other);
            std::scoped_lock lock(*mutex, *otherConverted.mutex);
            size_t pos = data.find(otherConverted.data);

            while (pos != std::basic_string<T>::npos) 
            {
                data.erase(pos, otherConverted.data.length());
                pos = data.find(otherConverted.data, pos);
            }

            return *this;
        }

        template <typename U> requires ConvertibleCharTypes<U, T>
        BasicString& operator-=(const U* other)
        {
            BasicString<T> otherConverted(other);
            std::scoped_lock lock(*mutex, *otherConverted.mutex);
            size_t pos = data.find(otherConverted.data);

            while (pos != std::basic_string<T>::npos)
            {
                data.erase(pos, otherConverted.data.length());
                pos = data.find(otherConverted.data, pos);
            }

            return *this;
        }

		template <typename U> requires ConvertibleCharTypes<U, T>
		BasicString& operator-=(const U& other)
		{
			BasicString<T> otherConverted(other);
			std::scoped_lock lock(*mutex, *otherConverted.mutex);

			size_t pos = data.find(otherConverted.data);

			while (pos != std::basic_string<T>::npos)
			{
				data.erase(pos, otherConverted.data.length());
				pos = data.find(otherConverted.data, pos);
			}

			return *this;
		}

        T& operator[](size_t index) 
        {
            std::shared_lock lock(*mutex);

            return data[index];
        }

        const T& operator[](size_t index) const 
        {
            std::shared_lock lock(*mutex);

            return data.at(index);
        }

        template <typename F, typename L> requires ConvertibleCharTypes<F, T> && ConvertibleCharTypes<L, T>
        void FindAndReplace(const BasicString<F>& find, const BasicString<L>& replace) 
        {
            BasicString<T> findConverted(find);
            BasicString<T> replaceConverted(replace);
            std::scoped_lock lock(*mutex, *findConverted.mutex, *replaceConverted.mutex);
            size_t pos = 0;

            while ((pos = data.find(findConverted.data, pos)) != std::basic_string<T>::npos) 
            {
                data.replace(pos, findConverted.data.length(), replaceConverted.data);
                pos += replaceConverted.data.length();
            }
        }

        template <typename F, typename L> requires ConvertibleCharTypes<F, T> && ConvertibleCharTypes<L, T>
        void FindAndReplace(const std::basic_string<F>& find, const std::basic_string<L>& replace) 
        {
            BasicString<T> findConverted(find);
            BasicString<T> replaceConverted(replace);
            std::scoped_lock lock(*mutex, *findConverted.mutex, *replaceConverted.mutex);
            size_t pos = 0;

            while ((pos = data.find(findConverted.data, pos)) != std::basic_string<T>::npos) 
            {
                data.replace(pos, findConverted.data.length(), replaceConverted.data);

                pos += replaceConverted.data.length();
            }
        }

        template <typename F, typename L> requires ConvertibleCharTypes<F, T> && ConvertibleCharTypes<L, T>
        void FindAndReplace(const F* find, const L* replace) 
        {
            BasicString<T> findConverted(find);
            BasicString<T> replaceConverted(replace);
            std::scoped_lock lock(*mutex, *findConverted.mutex, *replaceConverted.mutex);

            size_t pos = 0;

            while ((pos = data.find(findConverted.data, pos)) != std::basic_string<T>::npos) 
            {
                data.replace(pos, findConverted.data.length(), replaceConverted.data);
                pos += replaceConverted.data.length();
            }
        }

        void ToUpper() 
        {
            std::unique_lock lock(*mutex);
            std::ranges::transform(data, data.begin(), [](T ch) -> T 
            {
                if constexpr (std::is_same_v<T, char>) 
                    return static_cast<T>(std::toupper(static_cast<unsigned char>(ch)));
                else 
                    return static_cast<T>(std::toupper(ch));
            });
        }

        void ToLower() 
        {
            std::unique_lock lock(*mutex);
            std::ranges::transform(data, data.begin(), [](T ch) -> T 
            {
                if constexpr (std::is_same_v<T, char>) 
                    return static_cast<T>(std::tolower(static_cast<unsigned char>(ch)));
                else 
                    return static_cast<T>(std::tolower(ch));
            });
        }

		template <typename U> requires ConvertibleCharTypes<U, T>
		bool Contains(const BasicString<U>& other) const
		{
			BasicString<T> otherConverted(other);
			std::scoped_lock lock(*mutex, *otherConverted.mutex);

			return data.find(otherConverted.data) != std::basic_string<T>::npos;
		}

		template <typename U> requires ConvertibleCharTypes<U, T>
        bool Contains(const std::basic_string<U>& other) const
        {
			BasicString<T> otherConverted(other);
			std::scoped_lock lock(*mutex, *otherConverted.mutex);

			return data.find(otherConverted.data) != std::basic_string<T>::npos;
        }

		template <typename U> requires ConvertibleCharTypes<U, T>
        bool Contains(const U* other) const
        {
            BasicString<T> otherConverted(other);
            std::scoped_lock lock(*mutex, *otherConverted.mutex);

            return data.find(otherConverted.data) != std::basic_string<T>::npos;
        }

		template <typename U> requires ConvertibleCharTypes<U, T>
		BasicString<U> SubString(size_t start, size_t length) const
		{
			std::shared_lock lock(*mutex);

			return BasicString(data.substr(start, length));
		}

        template <typename U> requires ConvertibleCharTypes<U, T>
		BasicString<U> SubString(size_t start) const
		{
			std::shared_lock lock(*mutex);

			return BasicString(data.substr(start));
		}

        [[nodiscard]] AtomicIterator<typename std::basic_string<T>::iterator> begin() noexcept
        {
            std::shared_lock lock(*mutex);
            return AtomicIterator<typename std::basic_string<T>::iterator>(data.begin(), mutex);
        }

        [[nodiscard]] AtomicIterator<typename std::basic_string<T>::iterator> end() noexcept
        {
            std::shared_lock lock(*mutex);
            return AtomicIterator<typename std::basic_string<T>::iterator>(data.end(), mutex);
        }

        [[nodiscard]] AtomicIterator<typename std::basic_string<T>::const_iterator> cbegin() const noexcept 
        {
            std::shared_lock lock(*mutex);
            return AtomicIterator<typename std::basic_string<T>::const_iterator>(data.cbegin(), mutex);
        }

        [[nodiscard]] AtomicIterator<typename std::basic_string<T>::const_iterator> cend() const noexcept 
        {
            std::shared_lock lock(*mutex);
            return AtomicIterator<typename std::basic_string<T>::const_iterator>(data.cend(), mutex);
        }

        [[nodiscard]] AtomicIterator<typename std::basic_string<T>::iterator> find() const noexcept
        {
			std::shared_lock lock(*mutex);
			return AtomicIterator<typename std::basic_string<T>::iterator>(data.find(), mutex);
        }

		template <typename U> requires ConvertibleCharTypes<U, T>
		[[nodiscard]] AtomicIterator<typename std::basic_string<T>::iterator> find(const BasicString<U>& str) const noexcept
		{
			BasicString<T> strConverted(str);

			std::shared_lock lock(*mutex);

			return AtomicIterator<typename std::basic_string<T>::iterator>(data.find(strConverted.data), mutex);
		}

        [[nodiscard]] size_t find(char c, size_t input) const noexcept
        {
            std::shared_lock lock(*mutex);

            return data.find(c, input);
        }

        [[nodiscard]] size_t Length() const 
        {
            std::shared_lock lock(*mutex);
            return data.length();
        }

        [[nodiscard]] bool IsEmpty() const
        {
            std::shared_lock lock(*mutex);
            return data.empty();
        }

        void Clear() 
        {
            std::unique_lock lock(*mutex);
            data.clear();
        }

        template <typename U> requires ConvertibleCharTypes<T, U>
        operator std::basic_string<U>() const 
        {
            std::shared_lock lock(*mutex);

            return Convert<T, U>(data);
        }

		static const size_t NullPosition = std::basic_string<T>::npos;

    private:
        
        template <typename From, typename To> requires ConvertibleCharTypes<From, To>
        std::basic_string<To> Convert(const std::basic_string<From>& from) const
        {
            static_assert(
                std::is_same_v<From, char> || std::is_same_v<From, wchar_t> ||
                std::is_same_v<From, char16_t> || std::is_same_v<From, char32_t>,
                "Convert only supports char, wchar_t, char16_t, and char32_t types."
                );

            static_assert(
                std::is_same_v<To, char> || std::is_same_v<To, wchar_t> ||
                std::is_same_v<To, char16_t> || std::is_same_v<To, char32_t>,
                "Convert only supports char, wchar_t, char16_t, and char32_t types."
                );

            if constexpr (std::is_same_v<From, To>)
                return from;

            else if constexpr (std::is_same_v<From, char> && std::is_same_v<To, wchar_t>)
            {
                std::wstring result(from.size() + 1, L'\0');

                size_t convertedChars = 0;
                errno_t error = mbstowcs_s(&convertedChars, result.data(), result.size(), from.c_str(), from.size());

                if (error != 0)
                    throw std::runtime_error("Failed to convert string from char to wchar_t.");

                result.resize(convertedChars - 1);

                return result;
            }
            else if constexpr (std::is_same_v<From, wchar_t> && std::is_same_v<To, char>)
            {
                std::string result;
                size_t convertedChars = 0;

                size_t bufferSize = from.size() * MB_CUR_MAX + 1;

                result.resize(bufferSize);

                convertedChars = std::wcstombs(result.data(), from.c_str(), bufferSize - 1);

                if (convertedChars == static_cast<size_t>(-1))
                    throw std::runtime_error("Conversion failed.");

                result.resize(convertedChars);
                return result;
            }
            else
            {
                std::cout << std::format("Unsupported character conversion. Conversion between '{}' and '{}'.", typeid(From).name(), typeid(To).name()) << std::endl;
                throw std::runtime_error("Unsupported character conversion.");
            }
        }

        template <typename U>
        friend struct std::hash;

        template <typename U>
        friend struct std::equal_to;

        template <typename U, typename Char>
        friend struct std::formatter;

        std::shared_ptr<std::shared_mutex> mutex = std::make_shared<std::shared_mutex>();

        std::basic_string<T> data;
    };

    template <CharType T>
    std::basic_ostream<T>& operator<<(std::basic_ostream<T>& stream, const BasicString<T>& str) 
    {
        std::shared_lock lock(*str.mutex);

        stream << str.data;

        return stream;
    }

    /*<CharType T1, CharType T2>
    auto operator+(const BasicString<T1>& lhs, const BasicString<T2>& rhs) 
    {
        using ReturnType = std::conditional_t<std::is_same_v<T1, wchar_t> || std::is_same_v<T2, wchar_t>, wchar_t, char>;

        BasicString<ReturnType> lhsConverted(static_cast<std::basic_string<ReturnType>>(lhs));
        BasicString<ReturnType> rhsConverted(static_cast<std::basic_string<ReturnType>>(rhs));

        std::scoped_lock lock(*lhsConverted.mutex, *rhsConverted.mutex);

        BasicString<ReturnType> result;
        result.data = lhsConverted.data + rhsConverted.data;

        return result;
    }

    template <CharType T1, CharType T2>
    auto operator+(const BasicString<T1>& lhs, const std::basic_string<T2>& rhs) 
    {
        using ReturnType = std::conditional_t<std::is_same_v<T1, wchar_t> || std::is_same_v<T2, wchar_t>, wchar_t, char>;

        BasicString<ReturnType> lhsConverted(static_cast<std::basic_string<ReturnType>>(lhs));
        BasicString<ReturnType> rhsConverted(static_cast<std::basic_string<ReturnType>>(rhs));

        std::scoped_lock lock(*lhsConverted.mutex, *rhsConverted.mutex);

        BasicString<ReturnType> result;
        result.data = lhsConverted.data + rhsConverted.data;

        return result;
    }

    template <CharType T1, CharType T2>
    auto operator+(const BasicString<T1>& lhs, const T2* rhs) 
    {
        using ReturnType = std::conditional_t<std::is_same_v<T1, wchar_t> || std::is_same_v<T2, wchar_t>, wchar_t, char>;

        BasicString<ReturnType> lhsConverted(static_cast<std::basic_string<ReturnType>>(lhs));
        BasicString<ReturnType> rhsConverted(static_cast<std::basic_string<ReturnType>>(rhs));

        std::scoped_lock lock(*lhsConverted.mutex, *rhsConverted.mutex);

        BasicString<ReturnType> result;
        result.data = lhsConverted.data + rhsConverted.data;

        return result;
    }

    template <CharType T1, CharType T2>
    auto operator-(const BasicString<T1>& lhs, const BasicString<T2>& rhs) 
    {
        using ReturnType = std::conditional_t<std::is_same_v<T1, wchar_t> || std::is_same_v<T2, wchar_t>, wchar_t, char>;

        BasicString<ReturnType> lhsConverted(static_cast<std::basic_string<ReturnType>>(lhs));
        BasicString<ReturnType> rhsConverted(static_cast<std::basic_string<ReturnType>>(rhs));

        std::scoped_lock lock(*lhsConverted.mutex, *rhsConverted.mutex);

        BasicString<ReturnType> result = lhsConverted;
        size_t pos = 0;

        while ((pos = result.data.find(rhsConverted.data, pos)) != std::basic_string<ReturnType>::npos) 
            result.data.erase(pos, rhsConverted.data.length());

        return result;
    }

    template <CharType T1, CharType T2>
    auto operator-(const BasicString<T1>& lhs, const std::basic_string<T2>& rhs) {
        using ReturnType = std::conditional_t<std::is_same_v<T1, wchar_t> || std::is_same_v<T2, wchar_t>, wchar_t, char>;

        BasicString<ReturnType> lhsConverted(static_cast<std::basic_string<ReturnType>>(lhs));
        BasicString<ReturnType> rhsConverted(static_cast<std::basic_string<ReturnType>>(rhs));

        std::scoped_lock lock(*lhsConverted.mutex, *rhsConverted.mutex);

        BasicString<ReturnType> result = lhsConverted;
        size_t pos = 0;

        while ((pos = result.data.find(rhsConverted.data, pos)) != std::basic_string<ReturnType>::npos) 
            result.data.erase(pos, rhsConverted.data.length());
        
        return result;
    }

    template <CharType T1, CharType T2>
    auto operator-(const BasicString<T1>& lhs, const T2* rhs)
    {
        using ReturnType = std::conditional_t<std::is_same_v<T1, wchar_t> || std::is_same_v<T2, wchar_t>, wchar_t, char>;

        BasicString<ReturnType> lhsConverted(static_cast<std::basic_string<ReturnType>>(lhs));
        BasicString<ReturnType> rhsConverted(static_cast<std::basic_string<ReturnType>>(rhs));

        std::scoped_lock lock(*lhsConverted.mutex, *rhsConverted.mutex);

        BasicString<ReturnType> result = lhsConverted;
        size_t pos = 0;
        while ((pos = result.data.find(rhsConverted.data, pos)) != std::basic_string<ReturnType>::npos) 
            result.data.erase(pos, rhsConverted.data.length());
        
        return result;
    }*/
}

namespace std
{
	template <typename T>
	struct hash<Invasion::Util::Types::BasicString<T>>
	{
		size_t operator()(const Invasion::Util::Types::BasicString<T>& str) const
		{
			std::shared_lock<std::shared_mutex> lock(*str.mutex);
			return std::hash<std::basic_string<T>>()(str.data);
		}
	};

	template <typename T>
	struct equal_to<Invasion::Util::Types::BasicString<T>>
	{
		bool operator()(const Invasion::Util::Types::BasicString<T>& lhs, const Invasion::Util::Types::BasicString<T>& rhs) const
		{
			return lhs.data == rhs.data;
		}
	};

    template <typename T, typename Char>
    struct formatter<Invasion::Util::Types::BasicString<T>, Char> : formatter<std::basic_string<T>, Char>
    {
        using formatter<std::basic_string<T>, Char>::formatter;

        template <typename FormatContext>
        auto format(const Invasion::Util::Types::BasicString<T>& str, FormatContext& ctx) const noexcept
        {
            std::shared_lock lock(*str.mutex);

            return formatter<std::basic_string<T>, Char>::format(str.data, ctx);
        }
    };
}