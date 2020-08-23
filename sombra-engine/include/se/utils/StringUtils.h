#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <array>
#include <vector>
#include <string>
#include <streambuf>

namespace se::utils {

	/**
	 * Class ArrayStreambuf, it's an streambuf with a compile-time
	 * fixed size
	 */
	template <typename CharT, std::streamsize Size>
	class ArrayStreambuf : public std::basic_streambuf<CharT>
	{
	private:	// Nested types
		using Base = std::basic_streambuf<CharT>;
		using char_type = typename Base::char_type;

	private:	// Attributes
		/** The buffer used for storing the text */
		std::array<char_type, Size> mBuffer;

	public:		// Functions
		/** Creates a new ArrayStreambuf */
		ArrayStreambuf() : mBuffer{} { setBuffer(); };
		ArrayStreambuf(const ArrayStreambuf& other) :
			mBuffer(other.mBuffer) { setBuffer(); };
		ArrayStreambuf(ArrayStreambuf&& other) :
			mBuffer(other.mBuffer) { setBuffer(); };

		/** Assignment operator */
		ArrayStreambuf& operator=(const ArrayStreambuf& other)
		{
			mBuffer = other.mBuffer;
			setBuffer();
			return *this;
		};
		ArrayStreambuf& operator=(ArrayStreambuf&& other)
		{
			mBuffer = other.mBuffer;
			setBuffer();
			return *this;
		};

		/** @return	a pointer to the internal buffer of the ArrayStreambuf */
		const char* data() const { return mBuffer.data(); };
	private:
		/** Sets mBuffer as the buffer to use by the parent streambuf */
		void setBuffer()
		{
			Base::setp(mBuffer.data(), mBuffer.data() + Size);
			Base::setg(mBuffer.data(), mBuffer.data(), mBuffer.data() + Size);
		};
	};


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

#endif		// STRING_UTILS_H
