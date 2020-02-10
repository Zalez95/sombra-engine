#include <cctype>
#include <algorithm>
#include "se/utils/StringUtils.h"

namespace se::utils {

	void trimLeft(std::string& str)
	{
		str.erase(
			str.begin(),
			std::find_if(
				str.begin(), str.end(),
				[](char c) { return !std::isspace(static_cast<int>(c)); }
			)
		);
	}


	void trimRight(std::string& str)
	{
		str.erase(
			std::find_if(
				str.rbegin(), str.rend(),
				[](char c) { return !std::isspace(static_cast<int>(c)); }
			).base(),
			str.end()
		);
	}


	void trimBoth(std::string& str)
	{
		trimLeft(str);
		trimRight(str);
	}


	std::vector<std::string> splitBy(const std::string& str, char c)
	{
		std::vector<std::string> substrings;

		std::size_t start = 0;
		while (true) {
			std::size_t iChar = str.find_first_of(c, start);
			if (iChar != std::string::npos) {
				substrings.push_back( str.substr(start, iChar - start) );
				start = iChar + 1;
			}
			else {
				substrings.push_back( str.substr(start) );
				break;
			}
		}

		return substrings;
	}

}
