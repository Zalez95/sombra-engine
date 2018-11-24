#ifndef FONT_READER_H
#define FONT_READER_H

#include <memory>
#include <vector>

namespace se::utils { class FileReader; }
namespace se::graphics { class Font; struct Character; }

namespace se::loaders {

	/**
	 * Class FontReader, it's used to load the Fonts from the given
	 * files
	 */
	class FontReader
	{
	private:	// Nested types
		using FontUPtr = std::unique_ptr<graphics::Font>;

	public:		// Functions
		/** Parses the Font in the given file and returns it
		 *
		 * @note	the cursor of the FileReader will be moved after parsing
		 *			the file
		 * @param	fileReader the file reader with the Font that we want
		 *			to parse
		 * @return	the parsed Font
		 * @throw	runtime_error in case of any error while parsing */
		FontUPtr read(utils::FileReader& fileReader) const;
	private:
		/** Parses the Font in the given file and returns it
		 *
		 * @param	fileReader the file reader with the Font that we want
		 *			to parse
		 * @return	a pointer to the parsed Font
		 * @throw	runtime_error in case of any error while parsing */
		FontUPtr parseFont(utils::FileReader& fileReader) const;

		/** Parses the Character at the current position of the given file and
		 * returns it
		 *
		 * @param	fileReader the file reader with the file that we want
		 *			to read
		 * @return	the parsed Character
		 * @throw	runtime_error in case of an unexpected text */
		graphics::Character parseCharacter(utils::FileReader& fileReader) const;
	};

}

#endif		// FONT_READER_H
