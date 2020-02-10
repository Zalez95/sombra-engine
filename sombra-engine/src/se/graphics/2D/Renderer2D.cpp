#include <algorithm>
#include "se/graphics/core/Texture.h"
#include "se/graphics/2D/Renderer2D.h"
#include "se/graphics/2D/Renderable2D.h"
#include "se/graphics/2D/RenderableText.h"
#include "../core/GLWrapper.h"

namespace se::graphics {

	Renderer2D::Batch::Batch() : mVertexCount(0)
	{
		mVAO.bind();

		mVBOPositions.bind();
		mVAO.setVertexAttribute(0, TypeId::Float, false, 2, 0);

		mVBOTexCoords.bind();
		mVAO.setVertexAttribute(1, TypeId::Float, false, 2, 0);

		mVBOColors.bind();
		mVAO.setVertexAttribute(2, TypeId::Float, false, 4, 0);

		mVBOTextureIds.bind();
		mVAO.setVertexAttribute(3, TypeId::UnsignedByte, false, 1, 0);

		mVAO.unbind();
	}


	void Renderer2D::Batch::submit(const BatchVertex* vertices, std::size_t vertexCount)
	{
		for (std::size_t i = 0; i < vertexCount; ++i) {
			mPositions.push_back(vertices[i].position);
			mTexCoords.push_back(vertices[i].texCoords);
			mColors.push_back(vertices[i].color);
			mTextureIds.push_back(vertices[i].textureId);
		}

		mVertexCount += vertexCount;
	}


	void Renderer2D::Batch::draw()
	{
		mVBOPositions.setData(mPositions.data(), mVertexCount);
		mVBOTexCoords.setData(mTexCoords.data(), mVertexCount);
		mVBOColors.setData(mColors.data(), mVertexCount);
		mVBOTextureIds.setData(mTextureIds.data(), mVertexCount);

		// Render instanced
		mVAO.bind();
		GL_WRAP( glDrawArrays(GL_TRIANGLES, 0, mVertexCount) );

		// Clear the batch data
		mVertexCount = 0;
		mPositions.clear();
		mTexCoords.clear();
		mColors.clear();
		mTextureIds.clear();
	}


	Renderer2D::Renderer2D()
	{
		if (!mProgram.init()) {
			SOMBRA_FATAL_LOG << "Failed to create the Program2D";
		}
	}


	Renderer2D::~Renderer2D()
	{
		mProgram.end();
	}


	void Renderer2D::start(const glm::mat4& projectionMatrix)
	{
		GL_WRAP( glEnable(GL_BLEND) );
		GL_WRAP( glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) );
		GL_WRAP( glDisable(GL_DEPTH_TEST) );

		mProgram.enable();
		mProgram.setProjectionMatrix(projectionMatrix);
	}


	void Renderer2D::submit(const Renderable2D* renderable2D)
	{
		if (renderable2D) {
			// Add the texture
			unsigned char textureId = renderable2D->getTexture()? addTexture(renderable2D->getTexture()) : -1;

			glm::vec2 position = renderable2D->getPosition();
			glm::vec2 size = renderable2D->getSize();
			glm::vec4 color = renderable2D->getColor();
			BatchVertex vertices[] = {
				{ { position.x, position.y }, { 0.0f, 0.0f }, color, textureId },
				{ { position.x, position.y + size.y }, { 0.0f, 1.0f }, color, textureId },
				{ { position.x + size.x, position.y }, { 1.0f, 0.0f }, color, textureId },
				{ { position.x + size.x, position.y }, { 1.0f, 0.0f }, color, textureId },
				{ { position.x, position.y + size.y }, { 0.0f, 1.0f }, color, textureId },
				{ { position.x + size.x, position.y + size.y }, { 1.0f, 1.0f }, color, textureId }
			};

			mBatch.submit(vertices, 6);
		}
	}


	void Renderer2D::submit(const RenderableText* renderableText)
	{
		if (renderableText) {
			auto color = renderableText->getColor();
			auto font = renderableText->getFont();

			// Add the texture
			unsigned char textureId = font->textureAtlas? addTexture(font->textureAtlas) : -1;

			// Add the vertices of the quad of each character in the text
			glm::vec2 characterScale = renderableText->getSize() / glm::vec2(font->maxCharacterSize);
			glm::vec2 advance(0.0f);

			for (char c : renderableText->getText()) {
				auto itCharacter = font->characters.find(c);
				if (itCharacter != font->characters.end()) {
					const Character& character = itCharacter->second;

					glm::vec2 offset = characterScale * glm::vec2(character.offset.x, font->maxCharacterSize.y - character.offset.y);
					glm::vec2 position = renderableText->getPosition() + advance + offset;
					glm::vec2 scale = characterScale * glm::vec2(character.size);
					glm::vec2 uvPosition = glm::vec2(character.atlasPosition) / glm::vec2(font->atlasSize);
					glm::vec2 uvScale = glm::vec2(character.size) / glm::vec2(font->atlasSize);

					BatchVertex vertices[] = {
						{ { position.x, position.y }, { uvPosition.x, uvPosition.y }, color, textureId },
						{ { position.x, position.y + scale.y }, { uvPosition.x, uvPosition.y + uvScale.y }, color, textureId },
						{ { position.x + scale.x, position.y }, { uvPosition.x + uvScale.x, uvPosition.y }, color, textureId },
						{ { position.x + scale.x, position.y }, { uvPosition.x + uvScale.x, uvPosition.y }, color, textureId },
						{ { position.x, position.y + scale.y }, { uvPosition.x, uvPosition.y + uvScale.y }, color, textureId },
						{ { position.x + scale.x, position.y + scale.y }, { uvPosition.x + uvScale.x, uvPosition.y + uvScale.y }, color, textureId }
					};
					mBatch.submit(vertices, 6);

					advance += characterScale * glm::vec2(character.advance, 0.0f);
				}
			}
		}
	}


	void Renderer2D::end()
	{
		// Draw the last submitted Renderables
		drawBatch();

		GL_WRAP( glEnable(GL_DEPTH_TEST) );
		GL_WRAP( glDisable(GL_BLEND) );
	}

// Private functions
	unsigned char Renderer2D::addTexture(TextureSPtr texture)
	{
		unsigned char textureIndex = 0;

		auto itTexture = std::find(mTextures.begin(), mTextures.end(), texture);
		if (itTexture != mTextures.end()) {
			textureIndex = static_cast<unsigned char>( std::distance(mTextures.begin(), itTexture) );
		}
		else {
			if (mTextures.full()) {
				drawBatch();
			}

			mTextures.push_back(texture);
			textureIndex = static_cast<unsigned char>( mTextures.size() - 1 );
		}

		return textureIndex;
	}

	void Renderer2D::drawBatch()
	{
		mProgram.setTextures(mTextures.data(), mTextures.size());
		mBatch.draw();
		mTextures.clear();
	}

}
