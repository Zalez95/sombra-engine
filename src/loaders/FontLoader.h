#ifndef FONT_LOADER_H
#define FONT_LOADER_H

#include <memory>
#include <vector>
namespace utils { class FileReader; }
namespace graphics { class Font; struct Character; }

namespace loaders {

	/**
	 * Class FontLoader, it's used to load the Fonts from the given
	 * files
	 */
	class FontLoader
	{
	private:	// Nested types
		/** Struct FILE_FORMAT, it holds the name, version and other data of
		 * our Font file format */
		struct FILE_FORMAT
		{
			static const std::string	FILE_EXTENSION;
		};

		typedef std::unique_ptr<graphics::Font> FontUPtr;

	public:		// Functions
		/** Creates a new FontLoader */
		FontLoader() {};

		/** Class destructor */
		~FontLoader() {};

		/** Parses the Font in the given file and returns it
		 * 
		 * @note	the cursor of the FileReader will be moved after parsing
		 *			the file
		 * @param	fileReader the file reader with the Font that we want
		 *			to parse
		 * @return	the parsed Font */
		FontUPtr load(utils::FileReader& fileReader) const;
	private:
		/** Parses the Font in the given file and returns it
		 * 
		 * @param	fileReader the file reader with the Font that we want
		 *			to parse
		 * @return	a pointer to the parsed Font */
		FontUPtr parseFont(utils::FileReader& fileReader) const;

		/** Parses the Character at the current position of the given file and
		 * returns it
		 *
		 * @param	fileReader the file reader with the file that we want
		 *			to read
		 * @return	the parsed Character */
		graphics::Character parseCharacter(utils::FileReader& fileReader) const;
	};

}

#endif		// FONT_LOADER_H
