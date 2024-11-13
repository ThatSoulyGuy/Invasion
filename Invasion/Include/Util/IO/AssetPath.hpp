#pragma once

#include <format>
#include "Util/Typedefs.hpp"

namespace Invasion::Util::IO
{
	struct AssetPath
	{

	public:
		
		AssetPath() = default;

		AssetPath(const NarrowString& localPath, const NarrowString& domain)
		{
			this->localPath = localPath;
			this->domain = domain;
		}
		
		AssetPath(const AssetPath&) = default;
		AssetPath& operator=(const AssetPath&) = default;

		NarrowString GetLocalPath() const
		{
			return localPath;
		}

		NarrowString GetDomain() const
		{
			return domain;
		}

		NarrowString GetFullPath() const
		{
			return std::format("Assets/{}/{}", domain, localPath);
		}

	private:
		
		NarrowString localPath;
		NarrowString domain;

	};
}