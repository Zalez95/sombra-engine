#include <string>
#include <vector>

namespace se::utils {

	/** Removes the spaces, tabs, etc. characters located at the left of the
	 * given string
	 *
	 * @param	str the string to remove the spaces from */
	void trimLeft(std::string& str);

	/** Removes the spaces, tabs, etc. characters located at the right of the
	 * given string
	 *
	 * @param	str the string to remove the spaces from */
	void trimRight(std::string& str);

	/** Trims both the left and right spaces from the given string
	 *
	 * @param	str the string to remove the spaces from */
	void trimBoth(std::string& str);

	/** Splits the given string by the given character
	 *
	 * @param	str the string to split
	 * @param	c the character to split it by
	 * @return	a vector with all the substrings generated */
	std::vector<std::string> splitBy(const std::string& str, char c);

}
