#pragma once

#include <wrl.h>
#include "Util/Types/BasicArray.hpp"
#include "Util/Types/BasicMap.hpp"
#include "Util/Types/BasicString.hpp"
#include "Util/Types/BasicTuple.hpp"

namespace Invasion::Util
{
	template <typename T>
	using MutableArray = Types::BasicArray<T, std::vector<T>>;

	template <typename T, size_t N>
	using ImmutableArray = Types::BasicArray<T, std::array<T, N>>;

	using NarrowString = Types::BasicString<char>;
	using WideString = Types::BasicString<wchar_t>;

	template <typename Key, typename Value>
	using OrderedMap = Types::BasicMap<Key, Value, std::map>;

	template <typename Key, typename Value>
	using UnorderedMap = Types::BasicMap<Key, Value, std::unordered_map>;

	template <typename... Arguments>
	using Tuple = Types::BasicTuple<Arguments...>;

	template <typename T>
	using Shared = std::shared_ptr<T>;

	template <typename T>
	using Unique = std::unique_ptr<T>;

	template <typename T>
	using Weak = std::weak_ptr<T>;

	template <typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
}