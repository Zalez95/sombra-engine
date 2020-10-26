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


	RenderableTerrain::Patch::Patch(const float* vertices, std::size_t numVertices) :
		mNumVertices(numVertices), mInstanceCount(0)
	{
		mVAO.bind();

		mVBOXZPositions.resizeAndCopy(vertices, numVertices);

		mVBOXZPositions.bind();
		mVAO.setVertexAttribute(0, TypeId::Float, false, 2, 0);

		mVBOXZLocations.bind();
		mVAO.setVertexAttribute(1, TypeId::Float, false, 2, 0);
		mVAO.setAttributeDivisor(1, 1);

		mVBOLods.bind();
		mVAO.setVertexIntegerAttribute(2, TypeId::Int, 1, 0);
		mVAO.setAttributeDivisor(2, 1);
	}


	void RenderableTerrain::Patch::submitInstance(const glm::vec2& nodeLocation, int lod)
	{
		mInstanceCount++;
		mXZLocations.push_back(nodeLocation);
		mLods.push_back(lod);
	}


	void RenderableTerrain::Patch::drawInstances()
	{
		// Set submitted instances data
		mVAO.bind();
		mVBOXZLocations.resizeAndCopy(mXZLocations.data(), mInstanceCount);
		mVBOLods.resizeAndCopy(mLods.data(), mInstanceCount);

		// Render instanced
		GraphicsOperations::drawArraysInstanced(PrimitiveType::TriangleFan, mNumVertices, mInstanceCount);

		// Clear submitted instances
		mInstanceCount = 0;
		mXZLocations.clear();
		mLods.clear();
	}


	RenderableTerrain::RenderableTerrain(float size, const std::vector<float>& lodDistances) :
		mQuadTree(size, lodDistances),
		mNormal(kNormal, 20),
		mBottom(kBottom, 18), mTop(kTop, 18), mLeft(kLeft, 18), mRight(kRight, 18),
		mBottomLeft(kBottomLeft, 16), mBottomRight(kBottomRight, 16), mTopLeft(kTopLeft, 16), mTopRight(kTopRight, 16) {}


	void RenderableTerrain::setHighestLodLocation(const glm::vec3& location)
	{
		mQuadTree.updateHighestLodLocation(location);
	}


	void RenderableTerrain::draw()
	{
		// Submit the nodes to their respective patch
		submitNode(mQuadTree.getRootNode(), glm::vec2(0.0f));

		// Draw the patches
		mNormal.drawInstances();
		mBottom.drawInstances();
		mTop.drawInstances();
		mLeft.drawInstances();
		mRight.drawInstances();
		mBottomLeft.drawInstances();
		mBottomRight.drawInstances();
		mTopLeft.drawInstances();
		mTopRight.drawInstances();
	}

// Private functions
	void RenderableTerrain::submitNode(const QuadTree::Node& node, const glm::vec2& parentLocation)
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
