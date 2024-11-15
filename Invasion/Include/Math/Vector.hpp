#pragma once

#include <array>
#include <cassert>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <shared_mutex>
#include <ranges>
#include <stdexcept>
#include <type_traits>

namespace Invasion::Math
{
    template <typename T>
    concept Arithmetic = std::is_arithmetic_v<T>;

    template <Arithmetic T, size_t N>
    class Vector;

    template <Arithmetic T, size_t N>
    std::ostream& operator<<(std::ostream& os, const Vector<T, N>& vec);

    template <Arithmetic T1, Arithmetic T2, size_t N>
    auto operator+(const Vector<T1, N>& lhs, const Vector<T2, N>& rhs);

    template <Arithmetic T1, Arithmetic T2, size_t N>
    auto operator-(const Vector<T1, N>& lhs, const Vector<T2, N>& rhs);

    template <Arithmetic T, size_t N>
    class Vector 
    {
        static_assert(N > 0, "Vector must have at least one dimension.");

    public:
        
        using value_type = T;
        using size_type = size_t;
        using iterator = typename std::array<T, N>::iterator;
        using const_iterator = typename std::array<T, N>::const_iterator;

        Vector() = default;

		explicit Vector(T scalar)
		{
			std::unique_lock lock(*mutex);
			std::fill(data.begin(), data.end(), scalar);
		}

        Vector(std::initializer_list<T> list) 
        {
            assert(list.size() == N && "Initializer list size must match vector dimensions.");

            std::unique_lock lock(*mutex);
            std::copy(list.begin(), list.end(), data.begin());
        }

        template <typename U, size_t M> requires (std::is_arithmetic_v<U> && M <= N)
        Vector(const Vector<U, M>& other)
        {
            std::shared_lock lock_other(*other.mutex);
            std::unique_lock lock(*mutex);

            std::copy(other.begin(), other.end(), data.begin());
        }

		template <typename U, size_t M> requires (std::is_arithmetic_v<U>&& M <= N)
        Vector(Vector<U, M>&& other) noexcept
        {
			std::shared_lock lock_other(*other.mutex);
			std::unique_lock lock(*mutex);

			std::move(other.begin(), other.end(), data.begin());
        }

        //Vector(const Vector& other) = default;

        template <Arithmetic U, size_t M> requires (M <= N)
        Vector& operator=(const Vector<U, M>& other)
        {
            std::scoped_lock lock(*mutex, *other.mutex);
            std::copy(other.begin(), other.end(), data.begin());

            return *this;
        }

        template <Arithmetic U, size_t M> requires (M <= N)
        Vector& operator=(Vector<U, M>&& other) noexcept
        {
            std::scoped_lock lock(*mutex, *other.mutex);
            std::move(other.begin(), other.end(), data.begin());

            return *this;
        }

        T& operator[](size_t index) 
        {
            assert(index < N && "Index out of bounds.");
            std::shared_lock lock(*mutex);

            return data[index];
        }

        const T& operator[](size_t index) const 
        {
            assert(index < N && "Index out of bounds.");
            std::shared_lock lock(*mutex);

            return data[index];
        }

        iterator begin() 
        {
            std::shared_lock lock(*mutex);
            return data.begin();
        }

        const_iterator begin() const 
        {
            std::shared_lock lock(*mutex);
            return data.begin();
        }

        iterator end() 
        {
            std::shared_lock lock(*mutex);
            return data.end();
        }

        const_iterator end() const 
        {
            std::shared_lock lock(*mutex);
            return data.end();
        }

        Vector operator+(const Vector& other) const 
        {
            std::shared_lock lock(*mutex);
            std::shared_lock lock_other(*other.mutex);

            Vector result;

            for (size_t i = 0; i < N; ++i) 
                result.data[i] = data[i] + other.data[i];
            
            return result;
        }

        Vector operator-(const Vector& other) const
        {
            std::shared_lock lock(*mutex);
            std::shared_lock lock_other(*other.mutex);

            Vector result;

            for (size_t i = 0; i < N; ++i) 
                result.data[i] = data[i] - other.data[i];
            
            return result;
        }

		Vector operator*(const Vector& other) const
		{
			std::shared_lock lock(*mutex);
			std::shared_lock lock_other(*other.mutex);

			Vector result;

			for (size_t i = 0; i < N; ++i)
				result.data[i] = data[i] * other.data[i];

			return result;
		}

        Vector operator*(T scalar) const 
        {
            std::shared_lock lock(*mutex);

            Vector result;

            for (size_t i = 0; i < N; ++i) 
                result.data[i] = data[i] * scalar;
            
            return result;
        }

		Vector operator/(const Vector& other) const
		{
			std::shared_lock lock(*mutex);
			std::shared_lock lock_other(*other.mutex);

			Vector result;

			for (size_t i = 0; i < N; ++i)
				result.data[i] = data[i] / other.data[i];

			return result;
		}

        Vector operator/(T scalar) const 
        {
            std::shared_lock lock(*mutex);

            Vector result;

            for (size_t i = 0; i < N; ++i) 
                result.data[i] = data[i] / scalar;
            
            return result;
        }

        Vector& operator+=(const Vector& other) 
        {
            std::unique_lock lock(*mutex);
            std::shared_lock lock_other(*other.mutex);

            for (size_t i = 0; i < N; ++i) 
                data[i] += other.data[i];
            
            return *this;
        }

        Vector& operator-=(const Vector& other)
        {
            std::unique_lock lock(*mutex);
            std::shared_lock lock_other(*other.mutex);

            for (size_t i = 0; i < N; ++i) 
                data[i] -= other.data[i];
            
            return *this;
        }

        Vector& operator*=(const Vector& other)
        {
			std::unique_lock lock(*mutex);
			std::shared_lock lock_other(*other.mutex);

			for (size_t i = 0; i < N; ++i)
				data[i] *= other.data[i];

			return *this;
        }

        Vector& operator*=(T scalar) 
        {
            std::unique_lock lock(*mutex);

            for (size_t i = 0; i < N; ++i) 
                data[i] *= scalar;
            
            return *this;
        }

		Vector& operator/=(const Vector& other)
        {
			std::unique_lock lock(*mutex);
			std::shared_lock lock_other(*other.mutex);

			for (size_t i = 0; i < N; ++i)
				data[i] /= other.data[i];

			return *this;
		}

        Vector& operator/=(T scalar) 
        {
            std::unique_lock lock(*mutex);

            for (size_t i = 0; i < N; ++i) 
                data[i] /= scalar;
            
            return *this;
        }

        bool operator==(const Vector& other) const 
        {
            std::shared_lock lock(*mutex);
            std::shared_lock lock_other(*other.mutex);

            return data == other.data;
        }

        bool operator!=(const Vector& other) const 
        {
            return !(*this == other);
        }

        static T Dot(const Vector& a, const Vector& b) 
        {
            std::shared_lock lock_a(*a.mutex);
            std::shared_lock lock_b(*b.mutex);

            T result = 0;

            for (size_t i = 0; i < N; ++i) 
                result += a.data[i] * b.data[i];
            
            return result;
        }

        template <Arithmetic U>
        static auto Dot(const Vector<U, N>& a, const Vector<U, N>& b) 
        {
            using ReturnType = std::common_type_t<T, U>;

            std::shared_lock lock_a(*a.mutex);
            std::shared_lock lock_b(*b.mutex);

            ReturnType result = 0;

            for (size_t i = 0; i < N; ++i) 
                result += static_cast<ReturnType>(a.data[i]) * static_cast<ReturnType>(b.data[i]);
            
            return result;
        }

        static Vector Cross(const Vector& a, const Vector& b)
        {
            static_assert(N == 3, "Cross product is only defined for 3D vectors.");

            std::shared_lock lock_a(*a.mutex);
            std::shared_lock lock_b(*b.mutex);

            Vector result;

            result.data[0] = a.data[1] * b.data[2] - a.data[2] * b.data[1];
            result.data[1] = a.data[2] * b.data[0] - a.data[0] * b.data[2];
            result.data[2] = a.data[0] * b.data[1] - a.data[1] * b.data[0];

            return result;
        }

        static T Distance(const Vector& a, const Vector& b) 
        {
            return std::sqrt(DistanceSquared(a, b));
        }

        static T DistanceSquared(const Vector& a, const Vector& b) 
        {
            std::shared_lock lock_a(*a.mutex);
            std::shared_lock lock_b(*b.mutex);

            T result = 0;

            for (size_t i = 0; i < N; ++i) 
            {
                T diff = a.data[i] - b.data[i];
                result += diff * diff;
            }

            return result;
        }

        Vector Normalize() const 
        {
            std::shared_lock lock(*mutex);

            T length = Length();
            Vector result;

            for (size_t i = 0; i < N; ++i) 
                result.data[i] = data[i] / length;
            
            return result;
        }

        T Length() const 
        {
            return std::sqrt(Dot(*this, *this));
        }

        T LengthSquared() const 
        {
            return Dot(*this, *this);
        }

        static Vector Lerp(const Vector& a, const Vector& b, T t) 
        {
            std::shared_lock lock_a(*a.mutex);
            std::shared_lock lock_b(*b.mutex);

            Vector result;

            for (size_t i = 0; i < N; ++i) 
                result.data[i] = a.data[i] + (b.data[i] - a.data[i]) * t;
            
            return result;
        }

        static Vector Slerp(const Vector& a, const Vector& b, T t)
        {
            static_assert(N == 3, "Slerp is only defined for 3D vectors.");

            std::shared_lock lock_a(*a.mutex);
            std::shared_lock lock_b(*b.mutex);

            T dot = Dot(a, b);

            dot = std::clamp(dot, static_cast<T>(-1), static_cast<T>(1));

            T theta = std::acos(dot);
            T sin_theta = std::sin(theta);

            if (sin_theta == 0) 
                return a;
            
            T factor_a = std::sin((1 - t) * theta) / sin_theta;
            T factor_b = std::sin(t * theta) / sin_theta;

            Vector result;

            for (size_t i = 0; i < N; ++i) 
                result.data[i] = a.data[i] * factor_a + b.data[i] * factor_b;
            
            return result;
        }

        Vector Reflect(const Vector& normal) const 
        {
            std::shared_lock lock_a(*mutex);
            std::shared_lock lock_n(*normal.mutex);

            return *this - normal * (2 * Dot(*this, normal));
        }

        Vector Project(const Vector& b) const 
        {
            std::shared_lock lock_a(*mutex);
            std::shared_lock lock_b(*b.mutex);

            T scalar = Dot(*this, b) / Dot(b, b);

            return b * scalar;
        }

        Vector Reject(const Vector& b) const 
        {
            return *this - Project(b);
        }

        static Vector Min(const Vector& a, const Vector& b) 
        {
            std::shared_lock lock_a(*a.mutex);
            std::shared_lock lock_b(*b.mutex);
            Vector result;

            for (size_t i = 0; i < N; ++i) 
                result.data[i] = std::min(a.data[i], b.data[i]);
            
            return result;
        }

        static Vector Max(const Vector& a, const Vector& b) 
        {
            std::shared_lock lock_a(*a.mutex);
            std::shared_lock lock_b(*b.mutex);
            Vector result;

            for (size_t i = 0; i < N; ++i) 
                result.data[i] = std::max(a.data[i], b.data[i]);
            
            return result;
        }

        static Vector Clamp(const Vector& a, const Vector& min, const Vector& max) 
        {
            std::shared_lock lock_a(*a.mutex);
            std::shared_lock lock_min(*min.mutex);
            std::shared_lock lock_max(*max.mutex);

            Vector result;

            for (size_t i = 0; i < N; ++i) 
                result.data[i] = std::clamp(a.data[i], min.data[i], max.data[i]);
            
            return result;
        }

        template <Arithmetic T1, Arithmetic T2, size_t N>
        auto Dot(const Vector<T1, N>& a, const Vector<T2, N>& b)
        {
            using ReturnType = std::common_type_t<T1, T2>;

            std::shared_lock lock_a(*a.mutex);
            std::shared_lock lock_b(*b.mutex);

            ReturnType result = 0;

            for (size_t i = 0; i < N; ++i)
                result += static_cast<ReturnType>(a.data[i]) * static_cast<ReturnType>(b.data[i]);

            return result;
        }

        template <Arithmetic T1, Arithmetic T2>
        auto Cross(const Vector<T1, 3>& a, const Vector<T2, 3>& b)
        {
            using ReturnType = std::common_type_t<T1, T2>;

            std::shared_lock lock_a(*a.mutex);
            std::shared_lock lock_b(*b.mutex);

            Vector<ReturnType, 3> result;

            result.data[0] = static_cast<ReturnType>(a.data[1]) * static_cast<ReturnType>(b.data[2]) - static_cast<ReturnType>(a.data[2]) * static_cast<ReturnType>(b.data[1]);
            result.data[1] = static_cast<ReturnType>(a.data[2]) * static_cast<ReturnType>(b.data[0]) - static_cast<ReturnType>(a.data[0]) * static_cast<ReturnType>(b.data[2]);
            result.data[2] = static_cast<ReturnType>(a.data[0]) * static_cast<ReturnType>(b.data[1]) - static_cast<ReturnType>(a.data[1]) * static_cast<ReturnType>(b.data[0]);

            return result;
        }

        Vector Abs() const 
        {
            std::shared_lock lock(*mutex);
            Vector result;

            for (size_t i = 0; i < N; ++i) 
                result.data[i] = std::abs(data[i]);
            
            return result;
        }

        Vector Floor() const 
        {
            std::shared_lock lock(*mutex);
            Vector result;

            for (size_t i = 0; i < N; ++i) 
                result.data[i] = std::floor(data[i]);
            
            return result;
        }

        Vector Ceil() const 
        {
            std::shared_lock lock(*mutex);
            Vector result;

            for (size_t i = 0; i < N; ++i) 
                result.data[i] = std::ceil(data[i]);
            
            return result;
        }

        Vector Round() const
        {
            std::shared_lock lock(*mutex);
            Vector result;

            for (size_t i = 0; i < N; ++i) 
                result.data[i] = std::round(data[i]);
            
            return result;
        }

        Vector Fract() const
        {
            std::shared_lock lock(*mutex);
            Vector result;

            for (size_t i = 0; i < N; ++i) 
                result.data[i] = data[i] - std::floor(data[i]);
            
            return result;
        }

        Vector Mod(const Vector& other) const 
        {
            std::shared_lock lock_a(*mutex);
            std::shared_lock lock_b(other.mutex);

            Vector result;

            for (size_t i = 0; i < N; ++i) 
                result.data[i] = std::fmod(data[i], other.data[i]);
            
            return result;
        }

        Vector Mod(T scalar) const
        {
            std::shared_lock lock(*mutex);
            Vector result;

            for (size_t i = 0; i < N; ++i) 
                result.data[i] = std::fmod(data[i], scalar);
            
            return result;
        }

        Vector Sin() const 
        {
            std::shared_lock lock(*mutex);
            Vector result;

            for (size_t i = 0; i < N; ++i) 
                result.data[i] = std::sin(data[i]);
            
            return result;
        }

        Vector Cos() const 
        {
            std::shared_lock lock(*mutex);
            Vector result;

            for (size_t i = 0; i < N; ++i) 
                result.data[i] = std::cos(data[i]);
            
            return result;
        }

        Vector Tan() const 
        {
            std::shared_lock lock(*mutex);
            Vector result;

            for (size_t i = 0; i < N; ++i) 
                result.data[i] = std::tan(data[i]);
            
            return result;
        }

        Vector Asin() const
        {
            std::shared_lock lock(*mutex);
            Vector result;
            for (size_t i = 0; i < N; ++i) 
                result.data[i] = std::asin(data[i]);
            
            return result;
        }

        Vector Acos() const 
        {
            std::shared_lock lock(*mutex);
            Vector result;

            for (size_t i = 0; i < N; ++i) 
                result.data[i] = std::acos(data[i]);
            
            return result;
        }

        Vector Atan() const 
        {
            std::shared_lock lock(*mutex);
            Vector result;

            for (size_t i = 0; i < N; ++i) 
                result.data[i] = std::atan(data[i]);
            
            return result;
        }

        Vector Atan2(const Vector& y, const Vector& x) const 
        {
            static_assert(N == 3, "Atan2 is typically used with 3D vectors.");

            std::shared_lock lock_y(*y.mutex);
            std::shared_lock lock_x(*x.mutex);

            Vector result;

            for (size_t i = 0; i < N; ++i) 
                result.data[i] = std::atan2(y.data[i], x.data[i]);
            
            return result;
        }

        Vector Sinh() const 
        {
            std::shared_lock lock(*mutex);
            Vector result;

            for (size_t i = 0; i < N; ++i) 
                result.data[i] = std::sinh(data[i]);
            
            return result;
        }

        Vector Cosh() const 
        {
            std::shared_lock lock(*mutex);
            Vector result;

            for (size_t i = 0; i < N; ++i) 
                result.data[i] = std::cosh(data[i]);
            
            return result;
        }

        Vector Tanh() const 
        {
            std::shared_lock lock(*mutex);
            Vector result;

            for (size_t i = 0; i < N; ++i) 
                result.data[i] = std::tanh(data[i]);
            
            return result;
        }

        operator DirectX::XMFLOAT2() const requires (N == 2) 
        {
            return DirectX::XMFLOAT2(static_cast<float>(data[0]), static_cast<float>(data[1]));
        }

        operator DirectX::XMFLOAT3() const requires (N == 3) 
        {
            return DirectX::XMFLOAT3(static_cast<float>(data[0]), static_cast<float>(data[1]), static_cast<float>(data[2]));
        }

        operator DirectX::XMFLOAT4() const requires (N == 4) 
        {
            return DirectX::XMFLOAT4(static_cast<float>(data[0]), static_cast<float>(data[1]), static_cast<float>(data[2]), static_cast<float>(data[3]));
        }

        friend std::ostream& operator<< <T, N>(std::ostream& os, const Vector& vec);

        template <Arithmetic T1, Arithmetic T2, size_t X>
        friend auto operator+(const Vector& lhs, const Vector& rhs);

        template <Arithmetic T1, Arithmetic T2, size_t X>
        friend auto operator-(const Vector& lhs, const Vector& rhs);

    private:

        std::array<T, N> data{};
        mutable std::shared_ptr<std::shared_mutex> mutex = std::make_shared<std::shared_mutex>();

    };

    template <Arithmetic T, size_t N>
    std::ostream& operator<<(std::ostream& os, const Vector<T, N>& vec) 
    {
        std::shared_lock lock(*vec.mutex);

        os << "(";

        for (size_t i = 0; i < N; ++i) {
            os << vec.data[i];
            if (i != N - 1) 
                os << ", ";
        }

        os << ")";
        return os;
    }

    template <Arithmetic T1, Arithmetic T2, size_t N>
    auto operator+(const Vector<T1, N>& lhs, const Vector<T2, N>& rhs) 
    {
        using ReturnType = std::common_type_t<T1, T2>;

        Vector<ReturnType, N> result;

        std::shared_lock lock_lhs(*lhs.mutex);
        std::shared_lock lock_rhs(*rhs.mutex);

        for (size_t i = 0; i < N; ++i) 
            result.data[i] = static_cast<ReturnType>(lhs.data[i]) + static_cast<ReturnType>(rhs.data[i]);
        
        return result;
    }

    template <Arithmetic T1, Arithmetic T2, size_t N>
    auto operator-(const Vector<T1, N>& lhs, const Vector<T2, N>& rhs) 
    {
        using ReturnType = std::common_type_t<T1, T2>;

        Vector<ReturnType, N> result;

        std::shared_lock lock_lhs(*lhs.mutex);
        std::shared_lock lock_rhs(*rhs.mutex);

        for (size_t i = 0; i < N; ++i) 
            result.data[i] = static_cast<ReturnType>(lhs.data[i]) - static_cast<ReturnType>(rhs.data[i]);
        
        return result;
    }
};