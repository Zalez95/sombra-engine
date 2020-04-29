#include "se/graphics/2D/RenderableText.h"
#include "se/graphics/2D/Renderer2D.h"

namespace se::graphics {

	static constexpr unsigned short sQuadIndices[] = { 0, 2, 1, 1, 2, 3 };


	void RenderableText::submitVertices(Renderer2D& renderer) const
	{
		// Add the vertices of the quad of each character in the text
		glm::vec2 characterScale = mSize / glm::vec2(mFont->maxCharacterSize);
		glm::vec2 advance(0.0f);

		for (char c : mText) {
			auto itCharacter = mFont->characters.find(c);
			if (itCharacter != mFont->characters.end()) {
				const Character& character = itCharacter->second;

				glm::vec2 offset = characterScale * glm::vec2(character.offset.x, mFont->maxCharacterSize.y - character.offset.y);
				glm::vec2 position = mPosition + advance + offset;
				glm::vec2 scale = characterScale * glm::vec2(character.size);
				glm::vec2 uvPosition = glm::vec2(character.atlasPosition) / glm::vec2(mFont->atlasSize);
				glm::vec2 uvScale = glm::vec2(character.size) / glm::vec2(mFont->atlasSize);

				Renderer2D::BatchVertex vertices[] = {
					{ { position.x, position.y }, { uvPosition.x, uvPosition.y }, mColor },
					{ { position.x + scale.x, position.y }, { uvPosition.x + uvScale.x, uvPosition.y }, mColor },
					{ { position.x, position.y + scale.y }, { uvPosition.x, uvPosition.y + uvScale.y }, mColor },
					{ { position.x + scale.x, position.y + scale.y }, { uvPosition.x + uvScale.x, uvPosition.y + uvScale.y }, mColor }
				};
				renderer.submitVertices(vertices, 4, sQuadIndices, 6, mFont->textureAtlas.get());

				advance += characterScale * glm::vec2(character.advance, 0.0f);
			}
		}
	}

}
