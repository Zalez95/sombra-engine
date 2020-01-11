#ifndef FONT_READER_H
#define FONT_READER_H

#include <vector>
#include "Result.h"
#include "../../graphics/2D/Font.h"

namespace se::app {

	/**
	 * Class FontReader, it's used to load the Fonts from the given
	 * files
	 */
	class FontReader
	{
	private:	// Attributes
		/** The width of the generated glyphs in pixels (also applies for the
		 * height) */
		static constexpr unsigned int kGlyphWidth = 48;

	public:		// Functions
		/** Reads the font located at the given path
		 *
		 * @param	path the location of the Font
		 * @param	characterSet the set of characters to load to the Font
		 * @param	output where the new Font will be stored
		 * @return	a Result object with the result of the operation
		 * @note	if a character of the characterSet is not found in the font
		 *			it won't be loaded */
		static Result read(
			const std::string& path,
			const std::vector<char>& characterSet, se::graphics::Font& output
		);
	};

}

#endif		// FONT_READER_H
