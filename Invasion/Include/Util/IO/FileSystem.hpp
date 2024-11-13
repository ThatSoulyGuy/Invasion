#pragma once

#include <fstream>
#include <iostream>
#include <ostream>
#include <filesystem>
#include "Util/Typedefs.hpp"

namespace Invasion::Util::IO
{
	class FileSystem
	{

	public:

		FileSystem(const FileSystem&) = delete;
		FileSystem(FileSystem&&) = delete;
		FileSystem& operator=(const FileSystem&) = delete;
		FileSystem& operator=(FileSystem&&) = delete;

		static NarrowString GetExecutableDirectory()
		{
			return std::filesystem::path(std::filesystem::current_path()).string();
		}

		static NarrowString GetExecutableName()
		{
			return std::filesystem::path(std::filesystem::current_path()).filename().string();
		}

		static NarrowString GetExecutableExtension()
		{
			return std::filesystem::path(std::filesystem::current_path()).extension().string();
		}

		static NarrowString GetExecutablePath()
		{
			return std::filesystem::path(std::filesystem::current_path()).string();
		}

		static NarrowString GetExecutableNameWithoutExtension()
		{
			return std::filesystem::path(std::filesystem::current_path()).stem().string();
		}

		static bool FileExists(const NarrowString& path)
		{
			return std::filesystem::exists(path.operator std::basic_string<char, std::char_traits<char>, std::allocator<char>>());
		}

		static bool DirectoryExists(const NarrowString& path)
		{
			return std::filesystem::is_directory(path.operator std::basic_string<char, std::char_traits<char>, std::allocator<char>>());
		}

		static NarrowString ReadFile(const NarrowString& path)
		{
			std::ifstream file(path.operator std::basic_string<char, std::char_traits<char>, std::allocator<char>>(), std::ios::ate | std::ios::in);

			std::string content;

			if (!file) 
				return "";
			else 
			{
				std::streamsize size = file.tellg();
				file.seekg(0, std::ios::beg);

				content = std::string(size, '\0');

				if (file.read(&content[0], size))
					return "";
			}

			return content;
		}

	private:

		FileSystem() = default;
	};
}