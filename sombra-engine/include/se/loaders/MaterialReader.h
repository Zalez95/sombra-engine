#ifndef MATERIAL_READER_H
#define MATERIAL_READER_H

#include <memory>
#include <vector>

namespace se::utils { class FileReader; }
namespace se::graphics { struct Material; }

namespace se::loaders {

	/**
	 * Class MaterialReader, it's used to load the materials from the given
	 * files
	 */
	class MaterialReader
	{
	private:	// Nested types
		using MaterialUPtr = std::unique_ptr<graphics::Material>;

		/** Struct FileFormat, it holds the name, version and other data of
		 * our material file format */
		struct FileFormat
		{
			static const std::string sFileName;
			static const std::string sFileExtension;
			static const int sVersion = 1;
			static const int sRevision = 3;
		};

	public:		// Functions
		/** Parses the Materials in the given file and returns them
		 *
		 * @note	the cursor of the FileReader will be moved after parsing
		 *			the file
		 * @param	fileReader the file reader with the Materials that we want
		 *			to parse
		 * @return	a vector with the parsed Materials
		 * @throw	runtime_error in case of any error while parsing */
		std::vector<MaterialUPtr> read(utils::FileReader& fileReader) const;
	private:
		/** Checks the header of the given file
		 *
		 * @param	fileReader the reader of the file with the header we want
		 *			to check
		 * @return	true if the file and version is ok, false otherwise */
		bool checkHeader(utils::FileReader& fileReader) const;

		/** Parses the Materials in the given file and returns them
		 *
		 * @param	fileReader the file reader with the materials that we want
		 *			to parse
		 * @return	a vector with the parsed materials
		 * @throw	runtime_error in case of any error while parsing */
		std::vector<MaterialUPtr> parseMaterials(
			utils::FileReader& fileReader
		) const;

		/** Parses the Material at the current position of the given file and
		 * returns it
		 *
		 * @param	fileReader the file reader with the file that we want
		 *			to read
		 * @return	a pointer to the parsed Material
		 * @throw	runtime_error in case of an unexpected text */
		MaterialUPtr parseMaterial(utils::FileReader& fileReader) const;
	};

}

#endif		// MATERIAL_READER_H
