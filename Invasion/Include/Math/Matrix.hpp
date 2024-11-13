#pragma once

#include <DirectXMath.h>
#include "Math/Vector.hpp"

namespace Invasion::Math
{
    template <typename T, size_t R, size_t C>
    class Matrix
    {

    public:
        
        Matrix() : data_{} { }

        Matrix(const std::initializer_list<std::initializer_list<T>>& list)
        {
            std::unique_lock lock(mutex_);
            size_t i = 0;

            for (const auto& row : list)
            {
                if (i >= R)
                    throw std::out_of_range("Too many rows in initializer list");
                
                size_t j = 0;

                for (const auto& element : row)
                {
                    if (j >= C)
                        throw std::out_of_range("Too many columns in initializer list");
                    
                    data_[i][j] = element;
                    ++j;
                }
                if (j < C)
                {
                    for (; j < C; ++j)
                        data_[i][j] = T{};
                }
                ++i;
            }
            
            for (; i < R; ++i)
                data_[i].fill(T{});
        }

        Matrix(const Matrix& other)
        {
            std::shared_lock lock(other.mutex_);
            data_ = other.data_;
        }

        Matrix(Matrix&& other) noexcept
        {
            std::unique_lock lock(other.mutex_);
            data_ = std::move(other.data_);
        }

        Matrix& operator=(const Matrix& other)
        {
            if (this != &other)
            {
                std::unique_lock lock1(mutex_, std::defer_lock);
                std::shared_lock lock2(other.mutex_, std::defer_lock);
                std::lock(lock1, lock2);

                data_ = other.data_;
            }

            return *this;
        }

        Matrix& operator=(Matrix&& other) noexcept
        {
            if (this != &other)
            {
                std::unique_lock lock1(mutex_, std::defer_lock);
                std::unique_lock lock2(other.mutex_, std::defer_lock);
                std::lock(lock1, lock2);
                data_ = std::move(other.data_);
            }

            return *this;
        }

        Matrix operator+(const Matrix& other) const
        {
            Matrix result;

            std::shared_lock lock1(mutex_, std::defer_lock);
            std::shared_lock lock2(other.mutex_, std::defer_lock);

            std::lock(lock1, lock2);

            for (size_t i = 0; i < R; ++i)
            {
                for (size_t j = 0; j < C; ++j)
                    result.data_[i][j] = data_[i][j] + other.data_[i][j];
            }

            return result;
        }

        Matrix operator-(const Matrix& other) const
        {
            Matrix result;

            std::shared_lock lock1(mutex_, std::defer_lock);
            std::shared_lock lock2(other.mutex_, std::defer_lock);

            std::lock(lock1, lock2);

            for (size_t i = 0; i < R; ++i)
            {
                for (size_t j = 0; j < C; ++j)
                    result.data_[i][j] = data_[i][j] - other.data_[i][j];
            }

            return result;
        }

        Matrix operator*(T scalar) const
        {
            Matrix result;
            std::shared_lock lock(mutex_);

            for (size_t i = 0; i < R; ++i)
            {
                for (size_t j = 0; j < C; ++j)
                    result.data_[i][j] = data_[i][j] * scalar;
            }

            return result;
        }

        Matrix operator/(T scalar) const
        {
            if (scalar == T{})
                throw std::invalid_argument("Division by zero");
            
            Matrix result;
            std::shared_lock lock(mutex_);

            for (size_t i = 0; i < R; ++i)
            {
                for (size_t j = 0; j < C; ++j)
                    result.data_[i][j] = data_[i][j] / scalar;
            }

            return result;
        }

        Matrix& operator+=(const Matrix& other)
        {
            std::unique_lock lock1(mutex_, std::defer_lock);
            std::shared_lock lock2(other.mutex_, std::defer_lock);
            std::lock(lock1, lock2);

            for (size_t i = 0; i < R; ++i)
            {
                for (size_t j = 0; j < C; ++j)
                    data_[i][j] += other.data_[i][j];
            }

            return *this;
        }

        Matrix& operator-=(const Matrix& other)
        {
            std::unique_lock lock1(mutex_, std::defer_lock);
            std::shared_lock lock2(other.mutex_, std::defer_lock);
            std::lock(lock1, lock2);

            for (size_t i = 0; i < R; ++i)
            {
                for (size_t j = 0; j < C; ++j)
                    data_[i][j] -= other.data_[i][j];
            }

            return *this;
        }

		Matrix& operator*=(const Matrix& other)
		{
			std::unique_lock lock1(mutex_, std::defer_lock);
			std::shared_lock lock2(other.mutex_, std::defer_lock);
			std::lock(lock1, lock2);

			Matrix result;

			for (size_t i = 0; i < R; ++i)
			{
				for (size_t j = 0; j < C; ++j)
				{
					T sum = T{};
					for (size_t k = 0; k < C; ++k)
						sum += data_[i][k] * other.data_[k][j];
					result.data_[i][j] = sum;
				}
			}

			data_ = result.data_;

			return *this;
		}

        Matrix& operator*=(T scalar)
        {
            std::unique_lock lock(mutex_);

            for (size_t i = 0; i < R; ++i)
            {
                for (size_t j = 0; j < C; ++j)
                    data_[i][j] *= scalar;
            }

            return *this;
        }

        Matrix& operator/=(const Matrix& other)
		{
			std::unique_lock lock1(mutex_, std::defer_lock);
			std::shared_lock lock2(other.mutex_, std::defer_lock);
			std::lock(lock1, lock2);

			for (size_t i = 0; i < R; ++i)
			{
				for (size_t j = 0; j < C; ++j)
				{
					if (other.data_[i][j] == T{})
						throw std::invalid_argument("Division by zero");

					data_[i][j] /= other.data_[i][j];
				}
			}

			return *this;
		}

        Matrix& operator/=(T scalar)
        {
            if (scalar == T{})
                throw std::invalid_argument("Division by zero");
            
            std::unique_lock lock(mutex_);

            for (size_t i = 0; i < R; ++i)
            {
                for (size_t j = 0; j < C; ++j)
                    data_[i][j] /= scalar;
            }

            return *this;
        }

        bool operator==(const Matrix& other) const
        {
            std::shared_lock lock1(mutex_, std::defer_lock);
            std::shared_lock lock2(other.mutex_, std::defer_lock);

            std::lock(lock1, lock2);

            for (size_t i = 0; i < R; ++i)
            {
                for (size_t j = 0; j < C; ++j)
                {
                    if (data_[i][j] != other.data_[i][j])
                        return false;
                }
            }

            return true;
        }

        bool operator!=(const Matrix& other) const
        {
            return !(*this == other);
        }

        T& operator()(size_t row, size_t col)
        {
            if (row >= R || col >= C)
                throw std::out_of_range("Matrix indices out of range");
            
            std::unique_lock lock(mutex_);

            return data_[row][col];
        }

        const T& operator()(size_t row, size_t col) const
        {
            if (row >= R || col >= C)
                throw std::out_of_range("Matrix indices out of range");
            
            std::shared_lock lock(mutex_);

            return data_[row][col];
        }

        class RowProxy
        {

        public:

            RowProxy(Matrix& matrix, size_t row) : matrix_(matrix), row_(row) {}

            T& operator[](size_t col)
            {
                if (col >= C)
                    throw std::out_of_range("Column index out of range");
                
                std::unique_lock lock(matrix_.mutex_);

                return matrix_.data_[row_][col];
            }

        private:

            Matrix& matrix_;
            size_t row_;
        };

        class ConstRowProxy
        {

        public:

            ConstRowProxy(const Matrix& matrix, size_t row) : matrix_(matrix), row_(row) {}

            const T& operator[](size_t col) const
            {
                if (col >= C)
                {
                    throw std::out_of_range("Column index out of range");
                }
                std::shared_lock lock(matrix_.mutex_);
                return matrix_.data_[row_][col];
            }

        private:

            const Matrix& matrix_;
            size_t row_;
        };

        RowProxy operator[](size_t row)
        {
            if (row >= R)
                throw std::out_of_range("Row index out of range");
            
            return RowProxy(*this, row);
        }

        ConstRowProxy operator[](size_t row) const
        {
            if (row >= R)
                throw std::out_of_range("Row index out of range");
            
            return ConstRowProxy(*this, row);
        }

        Matrix<T, C, R> Transpose() const
        {
            Matrix<T, C, R> result;

            std::shared_lock lock(mutex_);
            std::unique_lock lockResult(result.mutex_);

            for (size_t i = 0; i < R; ++i)
            {
                for (size_t j = 0; j < C; ++j)
                    result.data_[j][i] = data_[i][j];
            }

            return result;
        }

        static Matrix Identity() requires(R == C)
        {
            Matrix result;
            std::unique_lock lock(result.mutex_);

            for (size_t i = 0; i < R; ++i)
            {
                for (size_t j = 0; j < C; ++j)
                    result.data_[i][j] = (i == j) ? T{ 1 } : T{ 0 };
            }

            return result;
        }

		static Matrix Zero()
		{
			return Matrix();
		}

		static Matrix Projection(T fov, T aspect, T nearPlane, T farPlane) requires(R == 4 && C == 4)
		{
			Matrix result;

			T tanHalfFov = std::tan(fov / 2);
			T zRange = nearPlane - farPlane;

			std::unique_lock lock(result.mutex_);

			result.data_[0][0] = 1 / (tanHalfFov * aspect);
			result.data_[1][1] = 1 / tanHalfFov;
			result.data_[2][2] = (-nearPlane - farPlane) / zRange;
			result.data_[2][3] = 2 * farPlane * nearPlane / zRange;
			result.data_[3][2] = 1;

			return result;
		}

		static Matrix Orthographic(T left, T right, T bottom, T top, T nearPlane, T farPlane) requires(R == 4 && C == 4)
		{
			Matrix result;

			T width = right - left;
			T height = top - bottom;
			T depth = farPlane - nearPlane;

			std::unique_lock lock(result.mutex_);

			result.data_[0][0] = 2 / width;
			result.data_[1][1] = 2 / height;
			result.data_[2][2] = -2 / depth;
			result.data_[3][0] = -(right + left) / width;
			result.data_[3][1] = -(top + bottom) / height;
			result.data_[3][2] = -(farPlane + nearPlane) / depth;
			result.data_[3][3] = 1;

			return result;
		}

		static Matrix LookAt(const Vector<T, 3>& eye, const Vector<T, 3>& target, const Vector<T, 3>& up) requires(R == 4 && C == 4)
		{
			Matrix result;

			Vector<T, 3> f = (target - eye).Normalize();
			Vector<T, 3> r = f.Cross(up).Normalize();
			Vector<T, 3> u = r.Cross(f);

			std::unique_lock lock(result.mutex_);

			result.data_[0][0] = r.x;
			result.data_[0][1] = r.y;
			result.data_[0][2] = r.z;
			result.data_[0][3] = -r.Dot(eye);
			result.data_[1][0] = u.x;
			result.data_[1][1] = u.y;
			result.data_[1][2] = u.z;
			result.data_[1][3] = -u.Dot(eye);
			result.data_[2][0] = -f.x;
			result.data_[2][1] = -f.y;
			result.data_[2][2] = -f.z;
			result.data_[2][3] = f.Dot(eye);
			result.data_[3][0] = 0;
			result.data_[3][1] = 0;
			result.data_[3][2] = 0;
			result.data_[3][3] = 1;

			return result;
		}

        static Matrix RotationX(T angle) requires(R == 4 && C == 4)
        {
			Matrix result;

			T cos = std::cos(angle);
			T sin = std::sin(angle);

			std::unique_lock lock(result.mutex_);

			result.data_[0][0] = 1;
			result.data_[1][1] = cos;
			result.data_[1][2] = -sin;
			result.data_[2][1] = sin;
			result.data_[2][2] = cos;
			result.data_[3][3] = 1;

			return result;
        }

		static Matrix RotationY(T angle) requires(R == 4 && C == 4)
        {
			Matrix result;

			T cos = std::cos(angle);
			T sin = std::sin(angle);

			std::unique_lock lock(result.mutex_);

			result.data_[0][0] = cos;
			result.data_[0][2] = sin;
			result.data_[1][1] = 1;
			result.data_[2][0] = -sin;
			result.data_[2][2] = cos;
			result.data_[3][3] = 1;

			return result;
        }

        static Matrix RotationZ(T angle) requires(R == 4 && C == 4)
        {
            Matrix result;

            T cos = std::cos(angle);
            T sin = std::sin(angle);

            std::unique_lock lock(result.mutex_);

            result.data_[0][0] = cos;
            result.data_[0][1] = -sin;
            result.data_[1][0] = sin;
            result.data_[1][1] = cos;
            result.data_[2][2] = 1;
            result.data_[3][3] = 1;

            return result;
        }

		static Matrix Translation(const Vector<T, 3>& translation) requires(R == 4 && C == 4)
		{
			Matrix result;

			std::unique_lock lock(result.mutex_);

			result.data_[0][0] = 1;
			result.data_[1][1] = 1;
			result.data_[2][2] = 1;
			result.data_[3][0] = translation[0];
            result.data_[3][1] = translation[1];
			result.data_[3][2] = translation[2];
			result.data_[3][3] = 1;

			return result;
		}

		static Matrix EulerRotation(const Vector<T, 3>& angles) requires(R == 4 && C == 4)
		{
			return RotationX(angles[0]) * RotationY(angles[1]) * RotationZ(angles[2]);
		}

        static Matrix Scale(const Vector<T, 3>& scale) requires(R == 4 && C == 4)
        {
			Matrix result;

			std::unique_lock lock(result.mutex_);

			result.data_[0][0] = scale[0];
			result.data_[1][1] = scale[1];
			result.data_[2][2] = scale[2];
			result.data_[3][3] = 1;

			return result;
        }

		static Matrix Transpose(const Matrix& matrix)
		{
			Matrix result;

			std::shared_lock lock(matrix.mutex_);

			for (size_t i = 0; i < R; ++i)
			{
				for (size_t j = 0; j < C; ++j)
					result.data_[j][i] = matrix.data_[i][j];
			}

			return result;
		}

		operator DirectX::XMMATRIX() const
		{
			std::shared_lock lock(mutex_);

			DirectX::XMMATRIX result;

			for (size_t i = 0; i < R; ++i)
			{
				for (size_t j = 0; j < C; ++j)
					result.r[i].m128_f32[j] = data_[i][j];
			}

			return result;
		}

    private:

        mutable std::shared_mutex mutex_;

        std::array<std::array<T, C>, R> data_;

        template <typename U, size_t R1, size_t C1_R2, size_t C2>
        friend Matrix<U, R1, C2> operator*(const Matrix<U, R1, C1_R2>& lhs, const Matrix<U, C1_R2, C2>& rhs);
    };

    template <typename T, size_t R1, size_t C1_R2, size_t C2>
    Matrix<T, R1, C2> operator*(const Matrix<T, R1, C1_R2>& lhs, const Matrix<T, C1_R2, C2>& rhs)
    {
        Matrix<T, R1, C2> result;

        std::shared_lock lock1(lhs.mutex_, std::defer_lock);
        std::shared_lock lock2(rhs.mutex_, std::defer_lock);
        std::unique_lock lockResult(result.mutex_, std::defer_lock);

        std::lock(lock1, lock2, lockResult);

        for (size_t i = 0; i < R1; ++i)
        {
            for (size_t j = 0; j < C2; ++j)
            {
                T sum = T{};
                for (size_t k = 0; k < C1_R2; ++k)
                {
                    sum += lhs.data_[i][k] * rhs.data_[k][j];
                }
                result.data_[i][j] = sum;
            }
        }

        return result;
    }

    template <typename T, size_t R, size_t C>
    Matrix<T, R, C> operator*(T scalar, const Matrix<T, R, C>& matrix)
    {
        return matrix * scalar;
    }
}