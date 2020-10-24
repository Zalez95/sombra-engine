#include <cstring>
#include <algorithm>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "se/app/loaders/FontReader.h"
#include "se/graphics/core/Texture.h"

namespace se::app {

	Result FontReader::read(
		const char* path,
		const std::vector<char>& characterSet, const glm::uvec2& characterSize,
		const glm::uvec2& atlasSize, graphics::Font& output
	) {
		if ((characterSize.x + 2 * kGlyphSeparation > atlasSize.x)
			|| (characterSize.y + 2 * kGlyphSeparation > atlasSize.y)
		) {
			return Result(true);
		}

		FT_Library library;
		if (FT_Init_FreeType(&library)) {
			return Result(false, "An error occurred during library initialization");
		}

		FT_Face face;
		if (FT_New_Face(library, path, 0, &face)) {
			return Result(false, "Failed to load font");
		}

		// Create the atlas texture
		output.textureAtlas = std::make_shared<graphics::Texture>(graphics::TextureTarget::Texture2D);
		if (!output.textureAtlas) {
			return Result(false, "Failed to create the Font atlas texture");
		}

		// Read the name
		if (face->family_name) {
			output.name = face->family_name;
		}

		// Read the characters (glyphs) in RGBA format
		FT_Set_Pixel_Sizes(face, characterSize.x, characterSize.y);

		auto pixels = new glm::lowp_u8vec4[atlasSize.x * atlasSize.y];
		std::memset(pixels, 0, atlasSize.x * atlasSize.y * sizeof(glm::lowp_u8vec4));
		glm::uvec2 atlasPosition(kGlyphSeparation, kGlyphSeparation);

		for (char c : characterSet) {
			if (FT_Load_Char(face, c, FT_LOAD_RENDER)) { continue; }

			// Add the character metadata
			output.characters[c] = {
				atlasPosition,
				{ face->glyph->bitmap.width, face->glyph->bitmap.rows },
				{ face->glyph->bitmap_left, face->glyph->bitmap_top },
				static_cast<unsigned int>(face->glyph->advance.x >> 6),
			};

			// Add the character glyph to the font atlas texture
			for (std::size_t i = 0; i < face->glyph->bitmap.rows; ++i) {
				for (std::size_t j = 0; j < face->glyph->bitmap.width; ++j) {
					std::size_t atlasIndex = (atlasPosition.y + i) * atlasSize.x + (atlasPosition.x + j);
					std::size_t glyphIndex = i * face->glyph->bitmap.width + j;

					pixels[atlasIndex].r = 255;
					pixels[atlasIndex].g = 255;
					pixels[atlasIndex].b = 255;
					pixels[atlasIndex].a = face->glyph->bitmap.buffer[glyphIndex];
				}
			}

			// Update the atlasPosition for the next character
			atlasPosition.x += face->glyph->bitmap.width + kGlyphSeparation;
			if (atlasPosition.x + characterSize.x + kGlyphSeparation > atlasSize.x) {
				atlasPosition.x = kGlyphSeparation;
				atlasPosition.y += characterSize.y + kGlyphSeparation;
				if (atlasPosition.y + characterSize.y + kGlyphSeparation > atlasSize.y) {
					break;
				}
			}
		}

		output.maxCharacterSize = characterSize;
		output.textureAtlas->setFiltering(graphics::TextureFilter::Linear, graphics::TextureFilter::Linear)
			.setWrapping(graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge)
			.setImage(
				pixels, graphics::TypeId::UnsignedByte, graphics::ColorFormat::RGBA,
				graphics::ColorFormat::RGBA, atlasSize.x, atlasSize.y
			);
		output.atlasSize = atlasSize;

		// Clear
		delete[] pixels;
		FT_Done_Face(face);
		FT_Done_FreeType(library);

		return Result();
	}

}
