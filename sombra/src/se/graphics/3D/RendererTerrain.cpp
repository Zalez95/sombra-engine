#include <algorithm>
#include "se/graphics/Pass.h"
#include "se/graphics/3D/RendererTerrain.h"
#include "se/graphics/3D/RenderableTerrain.h"
#include "se/graphics/core/GraphicsOperations.h"

namespace se::graphics {

	static constexpr float kNormal[] = {
		 0.0f, 0.0f, 0.0f,-0.5f,-0.5f,-0.5f,-0.5f, 0.0f,-0.5f, 0.5f,
		 0.0f, 0.5f, 0.5f, 0.5f, 0.5f, 0.0f, 0.5f,-0.5f, 0.0f,-0.5f
	};

	static constexpr float kBottom[] = {
		 0.0f, 0.0f,-0.5f,-0.5f,-0.5f, 0.0f,-0.5f, 0.5f, 0.0f, 0.5f,
		 0.5f, 0.5f, 0.5f, 0.0f, 0.5f,-0.5f,-0.5f,-0.5f,
	};

	static constexpr float kTop[] = {
		 0.0f, 0.0f,-0.5f,-0.5f,-0.5f, 0.0f,-0.5f, 0.5f, 0.5f, 0.5f,
		 0.5f, 0.0f, 0.5f,-0.5f, 0.0f,-0.5f,-0.5f,-0.5f
	};

	static constexpr float kLeft[] = {
		 0.0f, 0.0f,-0.5f,-0.5f,-0.5f, 0.5f, 0.0f, 0.5f, 0.5f, 0.5f,
		 0.5f, 0.0f, 0.5f,-0.5f, 0.0f,-0.5f,-0.5f,-0.5f
	};

	static constexpr float kRight[] = {
		 0.0f, 0.0f,-0.5f,-0.5f,-0.5f, 0.0f,-0.5f, 0.5f, 0.0f, 0.5f,
		 0.5f, 0.5f, 0.5f,-0.5f, 0.0f,-0.5f,-0.5f,-0.5f
	};

	static constexpr float kBottomLeft[] = {
		 0.0f, 0.0f,-0.5f,-0.5f,-0.5f, 0.5f, 0.0f, 0.5f,
		 0.5f, 0.5f, 0.5f, 0.0f, 0.5f,-0.5f,-0.5f,-0.5f
	};

	static constexpr float kBottomRight[] = {
		 0.0f, 0.0f,-0.5f,-0.5f,-0.5f, 0.0f,-0.5f, 0.5f,
		 0.0f, 0.5f, 0.5f, 0.5f, 0.5f,-0.5f,-0.5f,-0.5f
	};

	static constexpr float kTopLeft[] = {
		 0.0f, 0.0f,-0.5f,-0.5f,-0.5f, 0.5f, 0.5f, 0.5f,
		 0.5f, 0.0f, 0.5f,-0.5f, 0.0f,-0.5f,-0.5f,-0.5f
	};

	static constexpr float kTopRight[] = {
		 0.0f, 0.0f,-0.5f,-0.5f,-0.5f, 0.0f,-0.5f, 0.5f,
		 0.5f, 0.5f, 0.5f,-0.5f, 0.0f,-0.5f,-0.5f,-0.5f
	};


	RendererTerrain::Patch::Patch(Context& context, const float* vertices, std::size_t numVertices) :
		mNumVertices(numVertices), mInstanceCount(0)
	{
		mVAO = context.create<VertexArray>();

		mVBOXZPositions = context.create<VertexBuffer>();
		mVBOXZPositions.edit([=](VertexBuffer& vbo) {
			vbo.resizeAndCopy(vertices, numVertices);
		});

		context.execute([vao = mVAO, vbo = mVBOXZPositions](graphics::Context::Query& q) {
			q.getBindable(vao)->bind();
			q.getBindable(vbo)->bind();
		});
		mVAO.edit([](VertexArray& vao) {
			vao.enableAttribute(0);
			vao.setVertexAttribute(0, TypeId::Float, false, 2, 0);
		});

		mVBOXZLocations = context.create<VertexBuffer>();
		context.execute([vao = mVAO, vbo = mVBOXZLocations](graphics::Context::Query& q) {
			q.getBindable(vao)->bind();
			q.getBindable(vbo)->bind();
		});
		mVAO.edit([](VertexArray& vao) {
			vao.enableAttribute(1);
			vao.setVertexAttribute(1, TypeId::Float, false, 2, 0);
			vao.setAttributeDivisor(1, 1);
		});

		mVBOLods = context.create<VertexBuffer>();
		context.execute([vao = mVAO, vbo = mVBOLods](graphics::Context::Query& q) {
			q.getBindable(vao)->bind();
			q.getBindable(vbo)->bind();
		});
		mVAO.edit([](VertexArray& vao) {
			vao.enableAttribute(2);
			vao.setVertexIntegerAttribute(2, TypeId::Int, 1, 0);
			vao.setAttributeDivisor(2, 1);
		});
	}


	void RendererTerrain::Patch::submitInstance(const glm::vec2& nodeLocation, int lod)
	{
		mInstanceCount++;
		mXZLocations.push_back(nodeLocation);
		mLods.push_back(lod);
	}


	void RendererTerrain::Patch::drawInstances(Context::Query& q)
	{
		// Set submitted instances data
		q.getBindable(mVAO)->bind();
		q.getTBindable(mVBOXZLocations)->resizeAndCopy(mXZLocations.data(), mInstanceCount);
		q.getTBindable(mVBOLods)->resizeAndCopy(mLods.data(), mInstanceCount);

		// Render instanced
		GraphicsOperations::drawArraysInstanced(PrimitiveType::TriangleFan, mNumVertices, mInstanceCount);

		// Clear submitted instances
		mInstanceCount = 0;
		mXZLocations.clear();
		mLods.clear();
	}


	RendererTerrain::RendererTerrain(const std::string& name, Context& context) :
		Renderer3D(name),
		mNormal(context, kNormal, 20),
		mBottom(context, kBottom, 18), mTop(context, kTop, 18), mLeft(context, kLeft, 18), mRight(context, kRight, 18),
		mBottomLeft(context, kBottomLeft, 16), mBottomRight(context, kBottomRight, 16), mTopLeft(context, kTopLeft, 16), mTopRight(context, kTopRight, 16)
	{}

// Private functions
	void RendererTerrain::sortQueue()
	{
		// Sort the render queue by Pass
		std::sort(
			mRenderQueue.begin(), mRenderQueue.end(),
			[](const RenderablePassPair& lhs, const RenderablePassPair& rhs) { return lhs.second < rhs.second; }
		);
	}


	void RendererTerrain::render(Context::Query& q)
	{
		const Pass* lastPass = nullptr;
		for (auto& [renderable, pass] : mRenderQueue) {
			if (pass != lastPass) {
				lastPass = pass;
				pass->bind(q);
			}

			renderable->bind(q, pass);

			// Draw the patches
			submitNode(renderable->getQuadTree().getRootNode(), glm::vec2(0.0f));
			mNormal.drawInstances(q);
			mBottom.drawInstances(q);
			mTop.drawInstances(q);
			mLeft.drawInstances(q);
			mRight.drawInstances(q);
			mBottomLeft.drawInstances(q);
			mBottomRight.drawInstances(q);
			mTopLeft.drawInstances(q);
			mTopRight.drawInstances(q);
		}
	}


	void RendererTerrain::clearQueue()
	{
		mRenderQueue.clear();
	}


	void RendererTerrain::submitRenderable3D(Renderable3D& renderable, Pass& pass)
	{
		auto renderableTerrain = dynamic_cast<RenderableTerrain*>(&renderable);
		if (renderableTerrain) {
			mRenderQueue.emplace_back(renderableTerrain, &pass);
		}
	}


	void RendererTerrain::submitNode(const QuadTree::Node& node, const glm::vec2& parentLocation)
	{
		glm::vec2 nodeLocation(parentLocation + node.xzSeparation);

		if (node.isLeaf) {
			if (node.neighboursLods[static_cast<int>(QuadTree::Direction::Bottom)] < node.lod) {
				if (node.neighboursLods[static_cast<int>(QuadTree::Direction::Left)] < node.lod) {
					mBottomLeft.submitInstance(nodeLocation, node.lod);
				}
				else if (node.neighboursLods[static_cast<int>(QuadTree::Direction::Right)] < node.lod) {
					mBottomRight.submitInstance(nodeLocation, node.lod);
				}
				else {
					mBottom.submitInstance(nodeLocation, node.lod);
				}
			}
			else if (node.neighboursLods[static_cast<int>(QuadTree::Direction::Top)] < node.lod) {
				if (node.neighboursLods[static_cast<int>(QuadTree::Direction::Left)] < node.lod) {
					mTopLeft.submitInstance(nodeLocation, node.lod);
				}
				else if (node.neighboursLods[static_cast<int>(QuadTree::Direction::Right)] < node.lod) {
					mTopRight.submitInstance(nodeLocation, node.lod);
				}
				else {
					mTop.submitInstance(nodeLocation, node.lod);
				}
			}
			else if (node.neighboursLods[static_cast<int>(QuadTree::Direction::Left)] < node.lod) {
				mLeft.submitInstance(nodeLocation, node.lod);
			}
			else if (node.neighboursLods[static_cast<int>(QuadTree::Direction::Right)] < node.lod) {
				mRight.submitInstance(nodeLocation, node.lod);
			}
			else {
				mNormal.submitInstance(nodeLocation, node.lod);
			}
		}
		else {
			for (auto& child : node.children) {
				submitNode(*child, nodeLocation);
			}
		}
	}

}
