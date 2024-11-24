#pragma once

#include <type_traits>x
#include <string>
#include "Util/Typedefs.hpp"

#define BUILDABLE_PROPERTY(name, type, container) \
	type name = type(); \
	public: static constexpr auto name##Pointer = &container::name; \
    using name##Setter = Setter<container, type, container::name##Pointer>;

namespace Invasion::Util
{
    template <typename Class, typename MemberType, MemberType Class::* MemberPtr>
    struct Setter
    {
        using ValueType = MemberType;
        ValueType value;

        void operator()(Class& obj) const
        {
            obj.*MemberPtr = value;
        }
    };

    template <typename T>
    class Builder
    {

    public:

        static Builder<T> New() { return Builder<T>(); }

        template <typename SetterType>
        Builder<T>& Set(SetterType setter)
        {
            static_assert(std::is_invocable_v<SetterType, T&>, "SetterType must be callable with T&");

            setters += ([setter](T& obj) { setter(obj); });
            return *this;
        }

		void Build(T& obj) const
        {
			setters.ForEach([&obj](std::function<void(T&)> setter) { setter(obj); });
        }

    private:

		Builder() = default;

        MutableArray<std::function<void(T&)>> setters;

    };
}