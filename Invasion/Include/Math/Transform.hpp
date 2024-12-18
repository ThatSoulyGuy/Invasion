#pragma once

#include "ECS/Component.hpp"
#include "Math/Matrix.hpp"

using namespace Invasion::ECS;

namespace Invasion::Math
{
    class Transform : public Component, public std::enable_shared_from_this<Transform>
    {

    public:
            
        Transform(const Transform&) = delete;
        Transform& operator=(const Transform&) = delete;

        void Translate(const Vector<float, 3>& translation)
        {
            std::unique_lock lock(mutex_);

            localPosition += translation;

            MarkDirty();
        }

        void Rotate(const Vector<float, 3>& rotation)
        {
            std::unique_lock lock(mutex_);

            localRotation += rotation;

            MarkDirty();
        }

        void Scale(const Vector<float, 3>& scale)
        {
            std::unique_lock lock(mutex_);

            localScale *= scale;

            MarkDirty();
        }

        Vector<float, 3> GetLocalPosition() const
        {
            std::shared_lock lock(mutex_);

            return localPosition;
        }

        Vector<float, 3> GetLocalRotation() const
        {
            std::shared_lock lock(mutex_);

            return localRotation;
        }

        Vector<float, 3> GetLocalScale() const
        {
            std::shared_lock lock(mutex_);

            return localScale;
        }

        void SetLocalPosition(const Vector<float, 3>& position)
        {
            std::unique_lock lock(mutex_);
            localPosition = position;

            MarkDirty();
        }

        void SetLocalRotation(const Vector<float, 3>& rotation)
        {
            std::unique_lock lock(mutex_);
            localRotation = rotation;

            MarkDirty();
        }

        void SetLocalScale(const Vector<float, 3>& scale)
        {
            std::unique_lock lock(mutex_);
            localScale = scale;

            MarkDirty();
        }

        Vector<float, 3> GetWorldPosition()
        {
            UpdateWorldTransform();

            std::shared_lock lock(mutex_);

            return worldPosition;
        }

        Vector<float, 3> GetWorldRotation()
        {
            UpdateWorldTransform();

            std::shared_lock lock(mutex_);

            return worldRotation;
        }

        Vector<float, 3> GetWorldScale()
        {
            UpdateWorldTransform();

            std::shared_lock lock(mutex_);

            return worldScale;
        }

        Vector<float, 3> GetRight()
        {
            UpdateWorldTransform();

            std::shared_lock lock(mutex_);
                
            return Vector<float, 3>{ worldMatrix[0][0], worldMatrix[0][1], worldMatrix[0][2] }.Normalize();
        }

        Vector<float, 3> GetUp()
        {
            UpdateWorldTransform();

            std::shared_lock lock(mutex_);
                
            return Vector<float, 3>{ worldMatrix[1][0], worldMatrix[1][1], worldMatrix[1][2] }.Normalize();
        }

        Vector<float, 3> GetForward()
        {
            UpdateWorldTransform();

            std::shared_lock lock(mutex_);
                
            return Vector<float, 3>{ worldMatrix[2][0], worldMatrix[2][1], worldMatrix[2][2] }.Normalize();
        }

        Matrix<float, 4, 4> GetModelMatrix()
        {
            UpdateWorldTransform();

            std::shared_lock lock(mutex_);

            return worldMatrix;
        }

        void SetParent(const Shared<Transform>& parent)
        {
            std::unique_lock lock(mutex_);

            if (this->parent.lock() != parent)
            {
                if (auto currentParent = this->parent.lock())
                    currentParent->RemoveChild(shared_from_this());
                    
                this->parent = parent;

                if (parent)
                    parent->AddChild(shared_from_this());
                    
                MarkDirty();
            }
        }

        Shared<Transform> GetParent() const
        {
            std::shared_lock lock(mutex_);
            return parent.lock();
        }

        const MutableArray<Shared<Transform>> GetChildren() const
        {
            std::shared_lock lock(mutex_);
            return children;
        }
            
        void AddChild(const std::shared_ptr<Transform>& child)
        {
            std::unique_lock lock(mutex_);

            children += child;
        }

        void RemoveChild(const std::shared_ptr<Transform>& child)
        {
            std::unique_lock lock(mutex_);
                
            if (children.Contains(child))
                children -= child;
        }

        static Shared<Transform> Create()
        {
            return Shared<Transform>(new Transform());
        }

    private:

        Transform() : localPosition{ 0.0f, 0.0f, 0.0f }, localRotation{ 0.0f, 0.0f, 0.0f }, localScale{ 1.0f, 1.0f, 1.0f }, isDirty_{ true } { }

        void MarkDirty()
        {
            isDirty_ = true;

            for (const auto& child : children)
                child->MarkDirty();
        }

        void UpdateWorldTransform()
        {
            std::unique_lock lock(mutex_);

            if (isDirty_)
            {
                Matrix<float, 4, 4> translationMatrix = Matrix<float, 4, 4>::Translation(localPosition);
                Matrix<float, 4, 4> rotationMatrix = Matrix<float, 4, 4>::EulerRotation(localRotation);
                Matrix<float, 4, 4> scaleMatrix = Matrix<float, 4, 4>::Scale(localScale);

                Matrix<float, 4, 4> localMatrix = scaleMatrix * rotationMatrix * translationMatrix;

                if (auto parent = this->parent.lock())
                {
                    lock.unlock();

                    parent->UpdateWorldTransform();

                    lock.lock();

                    std::shared_lock parentLock(parent->mutex_);
                        
                    worldMatrix = localMatrix * parent->worldMatrix;
                }
                else
                    worldMatrix = localMatrix;

                DecomposeMatrix(worldMatrix, worldPosition, worldRotation, worldScale);

                isDirty_ = false;
            }
        }

        void DecomposeMatrix(const Matrix<float, 4, 4>& matrix, Vector<float, 3>& position, Vector<float, 3>& rotation, Vector<float, 3>& scale)
        {
            position = Vector<float, 3>{ matrix[3][0], matrix[3][1], matrix[3][2] };

            scale = Vector<float, 3>
            {
                std::sqrt(matrix[0][0] * matrix[0][0] + matrix[0][1] * matrix[0][1] + matrix[0][2] * matrix[0][2]),
                std::sqrt(matrix[1][0] * matrix[1][0] + matrix[1][1] * matrix[1][1] + matrix[1][2] * matrix[1][2]),
                std::sqrt(matrix[2][0] * matrix[2][0] + matrix[2][1] * matrix[2][1] + matrix[2][2] * matrix[2][2])
            };

            Matrix<float, 4, 4> rotationMatrix = matrix;

            for (int i = 0; i < 3; ++i)
            {
                rotationMatrix[0][i] /= scale[0];
                rotationMatrix[1][i] /= scale[1];
                rotationMatrix[2][i] /= scale[2];
            }

            rotation[1] = std::asin(-matrix[2][0]);

            if (std::cos(rotation[1]) != 0)
            {
                rotation[0] = std::atan2(matrix[2][1], matrix[2][2]);
                rotation[2] = std::atan2(matrix[1][0], matrix[0][0]);
            }
            else
            {
                rotation[0] = std::atan2(-matrix[0][2], matrix[1][1]);
                rotation[2] = 0;
            }

            rotation = rotation * (180.0f / static_cast<float>(DirectX::XM_PI));
        }

        mutable std::shared_mutex mutex_;

        Vector<float, 3> localPosition;
        Vector<float, 3> localRotation;
        Vector<float, 3> localScale;

        Vector<float, 3> worldPosition;
        Vector<float, 3> worldRotation;
        Vector<float, 3> worldScale;
        Matrix<float, 4, 4> worldMatrix;

        Weak<Transform> parent;

        MutableArray<Shared<Transform>> children;

        bool isDirty_;
    };
}