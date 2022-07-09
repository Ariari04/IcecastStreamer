#pragma once

#include <string>
#include <sys/stat.h>

namespace PathOperations
{
	inline std::string GetExtension(const std::string& strFileName)
	{
		std::string tmp = strFileName;
		size_t pos = tmp.rfind('.');
		if (pos != std::string::npos)
		{
			return tmp.substr(pos + 1);
		}
		return "";
	}

	inline std::string GetNameWithoutExtension(const std::string& strFileName)
	{
		std::string tmp = strFileName;
		size_t pos = tmp.rfind('.');

		if (pos != std::string::npos)
		{
			tmp = tmp.substr(0, pos);
		}

		return tmp;
	}

	inline std::string GetNameWithoutPathAndExtension(const std::string& strFileName)
	{
		std::string tmp = GetNameWithoutExtension(strFileName);
		size_t pos = tmp.rfind('\\');

		if (pos == std::string::npos)
		{
			pos = tmp.rfind('/');
		}

		if (pos != std::string::npos)
		{
			tmp = tmp.substr(pos + 1);
		}

		return tmp;
	}

	inline std::string GetDirectoryName(const std::string& strFileName)
	{
		std::string tmp = strFileName;
		size_t pos = tmp.rfind('\\');
		if (pos == std::string::npos)
		{
			pos = tmp.rfind('/');
		}

		if (pos != std::string::npos)
		{
			tmp = tmp.substr(pos);
		}

		return tmp;
	}

	inline bool CheckFileExists(const std::string& fullPath)
	{
		struct stat fileStat;

		if (0 != stat(fullPath.c_str(), &fileStat))
		{
			return false;
		}

		return true;
	}

	inline bool DeleteFile(const std::string& fullPath)
	{
		if (!CheckFileExists(fullPath))
		{
			return false;
		}

		return remove(fullPath.c_str()) == 0;
	}

	inline std::string GenerateTmpName(const std::string& strBaseName)
	{
		for(int i = 1; i < (1 << 16); ++i)
		{
			char buf[0x1000];
			sprintf(buf, "%svce%d", strBaseName.c_str(), i);
			if (!CheckFileExists(buf))
				return buf;
		}
		return "";
	}

};//namespace PathOperations
