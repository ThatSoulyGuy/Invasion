#pragma once

#include <cstddef>
#include <iostream>
#include <string>
#include <type_traits>
#include <utility>
#include <tuple> 

namespace Invasion::Util::Types
{
    template<typename... Ts>
    struct BasicTuple;

    template<std::size_t I, typename... Ts>
    constexpr auto& Get(class BasicTuple<Ts...>& mp);

    template<std::size_t I, typename T>
    struct BasicTuple_Element
    {
        T value;

        constexpr BasicTuple_Element() = default;
        constexpr BasicTuple_Element(const T& v) : value(v) {}
        constexpr BasicTuple_Element(T&& v) : value(std::move(v)) {}

        constexpr T& Get() { return value; }
        constexpr const T& Get() const { return value; }
    };

    template<typename Indices, typename... Ts>
    struct BasicTuple_Implementation;

    template<std::size_t... Is, typename... Ts>
    struct BasicTuple_Implementation<std::index_sequence<Is...>, Ts...> : BasicTuple_Element<Is, Ts>...
    {
        constexpr BasicTuple_Implementation() = default;

        template<typename... Us> requires (sizeof...(Us) == sizeof...(Ts))
        constexpr BasicTuple_Implementation(Us&&... args) : BasicTuple_Element<Is, Ts>(std::forward<Us>(args))... { }

        template<std::size_t I>
        constexpr auto& Get()
        {
            return BasicTuple_Element<I, typename std::tuple_element<I, std::tuple<Ts...>>::type>::Get();
        }

        template<std::size_t I>
        constexpr const auto& Get() const
        {
            return BasicTuple_Element<I, typename std::tuple_element<I, std::tuple<Ts...>>::type>::Get();
        }
    };

    template<typename... Ts>
    struct BasicTuple : BasicTuple_Implementation<std::index_sequence_for<Ts...>, Ts...>
    {
        using BaseType = BasicTuple_Implementation<std::index_sequence_for<Ts...>, Ts...>;
        using BaseType::BaseType;

        using BaseType::Get;

        friend bool operator==(const BasicTuple& lhs, const BasicTuple& rhs)
        {
            return EqualsImplementation(lhs, rhs, std::index_sequence_for<Ts...>{});
        }

    private:

        template<std::size_t... Is>
        static bool EqualsImplementation(const BasicTuple& lhs, const BasicTuple& rhs, std::index_sequence<Is...>)
        {
            return (... && (lhs.template Get<Is>() == rhs.template Get<Is>()));
        }

    };

    template<typename... Us>
    BasicTuple(Us&&... args) -> BasicTuple<Us...>;

    template<std::size_t I, typename... Ts>
    constexpr auto& Get(BasicTuple<Ts...>& mp)
    {
        return mp.template Get<I>();
    }

    template<std::size_t I, typename... Ts>
    constexpr const auto& Get(const BasicTuple<Ts...>& mp)
    {
        return mp.template Get<I>();
    }
}

namespace std
{
    template<typename... Ts>
    struct tuple_size<Invasion::Util::Types::BasicTuple<Ts...>> : std::integral_constant<std::size_t, sizeof...(Ts)> {};

    template<std::size_t I, typename... Ts>
    struct tuple_element<I, Invasion::Util::Types::BasicTuple<Ts...>>
    {
        using type = typename tuple_element<I, tuple<Ts...>>::type;
    };

    template<typename... Ts>
    struct hash<Invasion::Util::Types::BasicTuple<Ts...>>
    {
        std::size_t operator()(const Invasion::Util::Types::BasicTuple<Ts...>& mp) const
        {
            return hash_impl(mp, std::index_sequence_for<Ts...>{});
        }

    private:

        template<std::size_t... Is>
        static std::size_t hash_impl(const Invasion::Util::Types::BasicTuple<Ts...>& mp, std::index_sequence<Is...>)
        {
            std::size_t seed = 0;
            (..., hash_combine(seed, mp.template Get<Is>()));
            return seed;
        }

        template<typename T>
        static void hash_combine(std::size_t& seed, const T& value)
        {
            std::hash<T> hasher;
            seed ^= hasher(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
    };
}