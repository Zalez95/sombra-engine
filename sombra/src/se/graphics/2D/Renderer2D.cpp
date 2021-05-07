#include <algorithm>
#include "se/graphics/Pass.h"
#include "se/graphics/2D/Renderer2D.h"
#include "se/graphics/2D/Renderable2D.h"
#include "se/graphics/core/Texture.h"
#include "se/graphics/core/GraphicsOperations.h"

namespace se::graphics {

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
		mVAO.enableAttribute(0);
		mVAO.setVertexAttribute(0, TypeId::Float, false, 2, 0);

		mVBOTexCoords.bind();
		mVAO.enableAttribute(1);
		mVAO.setVertexAttribute(1, TypeId::Float, false, 2, 0);

		mVBOColors.bind();
		mVAO.enableAttribute(2);
		mVAO.setVertexAttribute(2, TypeId::Float, false, 4, 0);

		mVBOTextureIds.bind();
		mVAO.enableAttribute(3);
		mVAO.setVertexIntegerAttribute(3, TypeId::UnsignedByte, 1, 0);

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

		GraphicsOperations::drawIndexed(PrimitiveType::Triangle, mIBO.getIndexCount(), mIBO.getIndexType());

		// Clear the batch data
		mPositions.clear();
		mTexCoords.clear();
		mColors.clear();
		mTextureIds.clear();
		mIndices.clear();
	}


	void Renderer2D::submit(Renderable& renderable, Pass& pass)
	{
		auto renderable2D = dynamic_cast<Renderable2D*>(&renderable);
		if (renderable2D) {
			mRenderQueue.emplace_back(renderable2D, &pass);
		}
	}


	void Renderer2D::render()
	{
		if (mRenderQueue.empty()) { return; }

		// Sort by the Renderable2Ds z-index and by their Pass
		std::sort(
			mRenderQueue.begin(), mRenderQueue.end(),
			[](const RenderablePassPair& lhs, const RenderablePassPair& rhs) {
				return (lhs.first->getZIndex() < rhs.first->getZIndex())
					|| ((lhs.first->getZIndex() == rhs.first->getZIndex()) && (lhs.second < rhs.second));
			}
		);

		// Submit and draw the batches
		mPass = mRenderQueue.front().second;
		for (auto& [renderable, pass] : mRenderQueue) {
			if (pass != mPass) {
				drawBatch();
				mPass = pass;
			}

			renderable->submitVertices(*this);
		}
		drawBatch();
		mRenderQueue.clear();
	}


	void Renderer2D::submitVertices(
		BatchVertex* vertices, std::size_t vertexCount,
		const unsigned short* indices, std::size_t indexCount,
		Texture* texture
	) {
		// Draw if the Batch is full
		if ((mBatch.getVerticesLeft() < vertexCount) || (mBatch.getIndicesLeft() < indexCount)) {
			drawBatch();
		}

		// Set the texture indices of the vertices
		if (texture) {
			unsigned char textureIndex = kNoTexture;

			auto itTexture = std::find(mTextures.begin(), mTextures.end(), texture);
			if (itTexture != mTextures.end()) {
				textureIndex = static_cast<unsigned char>( std::distance(mTextures.begin(), itTexture) );
			}
			else {
				// Draw if the Batch we can't add more textures
				if (mTextures.size() == kMaxTextures) {
					drawBatch();
				}

				mTextures.push_back(texture);
				textureIndex = static_cast<unsigned char>( mTextures.size() - 1 );
			}

			for (std::size_t i = 0; i < vertexCount; ++i) {
				vertices[i].textureId = textureIndex;
			}
		}

		// Submit the vertex data to the Batch
		mBatch.submit(vertices, vertexCount, indices, indexCount);
	}

// Private functions
	void Renderer2D::drawBatch()
	{
		mPass->bind();

		unsigned int numTextures = std::min(kMaxTextures, static_cast<unsigned int>(mTextures.size()));
		for (unsigned int i = 0; i < numTextures; ++i) {
			mTextures[i]->setTextureUnit(i);
			mTextures[i]->bind();
		}

		mBatch.draw();

		mTextures.clear();
	}

}
