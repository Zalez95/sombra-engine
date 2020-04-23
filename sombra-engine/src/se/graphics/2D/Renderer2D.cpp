#include <algorithm>
#include "se/graphics/core/Texture.h"
#include "se/graphics/2D/Renderer2D.h"
#include "se/graphics/2D/Renderable2D.h"
#include "se/graphics/2D/RenderableText.h"
#include "se/graphics/core/Graphics.h"
#include "se/utils/Log.h"

namespace se::graphics {

	static constexpr unsigned short sQuadIndices[] = { 0, 2, 1, 1, 2, 3 };


	Renderer2D::Batch::Batch(std::size_t maxVertices, std::size_t maxIndices) :
		mPositions(maxVertices), mTexCoords(maxVertices), mColors(maxVertices), mTextureIds(maxVertices),
		mIndices(maxIndices)
	{
		mVAO.bind();

		mVBOPositions.resizeAndCopy(mPositions.data(), maxVertices);
		mVBOTexCoords.resizeAndCopy(mTexCoords.data(), maxVertices);
		mVBOColors.resizeAndCopy(mColors.data(), maxVertices);
		mVBOTextureIds.resizeAndCopy(mTextureIds.data(), maxVertices);

		mVBOPositions.bind();
		mVAO.setVertexAttribute(0, TypeId::Float, false, 2, 0);

		mVBOTexCoords.bind();
		mVAO.setVertexAttribute(1, TypeId::Float, false, 2, 0);

		mVBOColors.bind();
		mVAO.setVertexAttribute(2, TypeId::Float, false, 4, 0);

		mVBOTextureIds.bind();
		mVAO.setVertexAttribute(3, TypeId::UnsignedByte, false, 1, 0);

		mIBO.bind();

		mPositions.clear();
		mTexCoords.clear();
		mColors.clear();
		mTextureIds.clear();
		mIndices.clear();
	}


	void Renderer2D::Batch::submit(
		const BatchVertex* vertices, std::size_t vertexCount,
		const unsigned short* indices, std::size_t indexCount
	) {
		auto lastVertex = static_cast<unsigned short>(mPositions.size());

		for (std::size_t i = 0; i < vertexCount; ++i) {
			mPositions.push_back(vertices[i].position);
			mTexCoords.push_back(vertices[i].texCoords);
			mColors.push_back(vertices[i].color);
			mTextureIds.push_back(vertices[i].textureId);
		}

		for (std::size_t i = 0; i < indexCount; ++i) {
			mIndices.push_back(lastVertex + indices[i]);
		}
	}


	void Renderer2D::Batch::draw()
	{
		std::size_t vertexCount = mPositions.size();
		std::size_t indexCount = mIndices.size();

		// Update the buffers and draw
		mVAO.bind();

		mVBOPositions.copy(mPositions.data(), vertexCount);
		mVBOTexCoords.copy(mTexCoords.data(), vertexCount);
		mVBOColors.copy(mColors.data(), vertexCount);
		mVBOTextureIds.copy(mTextureIds.data(), vertexCount);
		mIBO.resizeAndCopy(mIndices.data(), TypeId::UnsignedShort, indexCount);

		Graphics::drawIndexed(PrimitiveType::Triangle, mIBO.getIndexCount(), mIBO.getIndexType());

		// Clear the batch data
		mPositions.clear();
		mTexCoords.clear();
		mColors.clear();
		mTextureIds.clear();
		mIndices.clear();
	}


	Renderer2D::Renderer2D() : mBatch(4 * kQuadsPerBatch, 6 * kQuadsPerBatch)
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
		Graphics::setBlending(true);
		Graphics::setDepthTest(false);

		mProgram.bind();
		mProgram.setProjectionMatrix(projectionMatrix);
	}


	void Renderer2D::submit(const Renderable2D* renderable2D)
	{
		if (renderable2D) {
			// Add the texture
			unsigned char textureId = renderable2D->getTexture()? addTexture(renderable2D->getTexture()) : -1;

			// Draw if the Batch is full
			if ((mBatch.getVerticesLeft() < 4) || (mBatch.getIndicesLeft() < 6)) {
				drawBatch();
			}

			glm::vec2 position = renderable2D->getPosition();
			glm::vec2 size = renderable2D->getSize();
			glm::vec4 color = renderable2D->getColor();
			BatchVertex vertices[] = {
				{ { position.x, position.y }, { 0.0f, 0.0f }, color, textureId },
				{ { position.x + size.x, position.y }, { 1.0f, 0.0f }, color, textureId },
				{ { position.x, position.y + size.y }, { 0.0f, 1.0f }, color, textureId },
				{ { position.x + size.x, position.y + size.y }, { 1.0f, 1.0f }, color, textureId }
			};
			mBatch.submit(vertices, 4, sQuadIndices, 6);
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
					// Draw if the Batch is full
					if ((mBatch.getVerticesLeft() < 4) || (mBatch.getIndicesLeft() < 6)) {
						drawBatch();
					}

					const Character& character = itCharacter->second;

					glm::vec2 offset = characterScale * glm::vec2(character.offset.x, font->maxCharacterSize.y - character.offset.y);
					glm::vec2 position = renderableText->getPosition() + advance + offset;
					glm::vec2 scale = characterScale * glm::vec2(character.size);
					glm::vec2 uvPosition = glm::vec2(character.atlasPosition) / glm::vec2(font->atlasSize);
					glm::vec2 uvScale = glm::vec2(character.size) / glm::vec2(font->atlasSize);

					BatchVertex vertices[] = {
						{ { position.x, position.y }, { uvPosition.x, uvPosition.y }, color, textureId },
						{ { position.x + scale.x, position.y }, { uvPosition.x + uvScale.x, uvPosition.y }, color, textureId },
						{ { position.x, position.y + scale.y }, { uvPosition.x, uvPosition.y + uvScale.y }, color, textureId },
						{ { position.x + scale.x, position.y + scale.y }, { uvPosition.x + uvScale.x, uvPosition.y + uvScale.y }, color, textureId }
					};
					mBatch.submit(vertices, 4, sQuadIndices, 6);

					advance += characterScale * glm::vec2(character.advance, 0.0f);
				}
			}
		}
	}


	void Renderer2D::end()
	{
		// Draw the last submitted Renderables
		drawBatch();

		Graphics::setDepthTest(true);
		Graphics::setBlending(false);
	}

// Private functions
	unsigned char Renderer2D::addTexture(TextureRef texture)
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
