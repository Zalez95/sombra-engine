#include <ft2build.h>
#include FT_FREETYPE_H
#include "se/app/loaders/FontReader.h"
#include "se/graphics/core/Texture.h"

namespace se::app {

	Result FontReader::read(
		const std::string& path,
		const std::vector<char>& characterSet, se::graphics::Font& output
	) {
		FT_Library library;
		if (FT_Init_FreeType(&library)) {
			return Result(false, "An error occurred during library initialization");
		}

		FT_Face face;
		if (FT_New_Face(library, path.c_str(), 0, &face)) {
			return Result(false, "Failed to load font");
		}

		// Read the name
		if (face->family_name) {
			output.name = face->family_name;
		}

		// Read the characters (glyphs)
		FT_Set_Pixel_Sizes(face, kGlyphWidth, 0);
		for (char c : characterSet) {
			if (!FT_Load_Char(face, c, FT_LOAD_RENDER)) {
				auto glyphTexture = std::make_unique<graphics::Texture>();
				glyphTexture->setFiltering(graphics::TextureFilter::Linear, graphics::TextureFilter::Linear);
				glyphTexture->setWrapping(graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge);
				glyphTexture->setImage(
					face->glyph->bitmap.buffer,
					graphics::TypeId::UnsignedByte,
					graphics::ColorFormat::Red,
					face->glyph->bitmap.width, face->glyph->bitmap.rows
				);

				output.characters[c] = {
					std::move(glyphTexture),
					{ face->glyph->bitmap.width, face->glyph->bitmap.rows },
					{ face->glyph->bitmap_left, face->glyph->bitmap_top },
					static_cast<unsigned int>(face->glyph->advance.x >> 6),
				};
			}
		}

		FT_Done_Face(face);
		FT_Done_FreeType(library);

		return Result();
	}

}
