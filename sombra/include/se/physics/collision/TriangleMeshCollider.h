#ifndef TRIANGLE_MESH_COLLIDER_H
#define TRIANGLE_MESH_COLLIDER_H

#include <memory>
#include "ConcaveCollider.h"

namespace se::physics {

	class TriangleCollider;
	template <typename T> class AABBAVLTree;


	/**
	 * Class TriangleMeshCollider, it's a Collider with a concave shape stored
	 * as a Triangle mesh
	 */
	class TriangleMeshCollider : public ConcaveCollider
	{
	private:	// Nested types
		using ColliderUPtr = std::unique_ptr<Collider>;

	private:	// Attributes
		/** The vertices of the Mesh in local space */
		std::vector<glm::vec3> mVertices;

		/** The indices to the vertices of the triangle faces */
		std::vector<unsigned short> mIndices;

		/** The AABB Tree used for checking ray casts and overlaps, it holds
		 * the Triangle index as user data */
		std::unique_ptr<AABBAVLTree<unsigned short>> mAABBTree;

		/** The transformation matrix of the TriangleMeshCollider */
		glm::mat4 mTransformsMatrix;

		/** If the TriangleMeshCollider has been updated or not */
		bool mUpdated;

	public:		// Functions
		/** Creates a new TriangleMeshCollider located at the origin of
		 * coordinates from the given vertices
		 *
		 * @param	vertices a pointer to the vertices location in local space
		 * @param	verticesSize the number of vertices
		 * @param	indices a pointer to the indices of the triangle faces
		 * @param	indicesSize the number indices */
		TriangleMeshCollider(
			const glm::vec3* vertices = nullptr, std::size_t verticesSize = 0,
			const unsigned short* indices = nullptr, std::size_t indicesSize = 0
		);
		TriangleMeshCollider(const TriangleMeshCollider& other);
		TriangleMeshCollider(TriangleMeshCollider&& other) = default;

		/** Class destructor */
		~TriangleMeshCollider();

		/** Assignment operator */
		TriangleMeshCollider& operator=(const TriangleMeshCollider& other);
		TriangleMeshCollider& operator=(TriangleMeshCollider&& other) = default;

		/** @copydoc Collider::clone() */
		virtual std::unique_ptr<Collider> clone() const override
		{ return std::make_unique<TriangleMeshCollider>(*this); };

		/** @return	a pointer to the vertices in local space of the
		 *			TriangleMeshCollider mesh */
		const glm::vec3* getVertices() const { return mVertices.data(); };

		/** @return	the number of vertices of the TriangleMeshCollider mesh */
		std::size_t getNumVertices() const { return mVertices.size(); };

		/** @return	a pointer to the indices of the TriangleMeshCollider mesh */
		const unsigned short* getIndices() const { return mIndices.data(); };

		/** @return	the number of indices of the TriangleMeshCollider mesh */
		std::size_t getNumIndices() const { return mIndices.size(); };

		/** Sets the Mesh of the TriangleMeshCollider
		 *
		 * @param	vertices a pointer to the vertices location in local space
		 * @param	verticesSize the number of vertices
		 * @param	indices a pointer to the indices of the triangle faces
		 * @param	indicesSize the number indices */
		void setMesh(
			const glm::vec3* vertices, std::size_t verticesSize,
			const unsigned short* indices, std::size_t indicesSize
		);

		/** @copydoc Collider::setTransforms() */
		virtual void setTransforms(const glm::mat4& transforms) override;

		/** @copydoc Collider::getTransforms() */
		virtual glm::mat4 getTransforms() const override
		{ return mTransformsMatrix; };

		/** @copydoc Collider::getAABB() */
		virtual AABB getAABB() const override;

		/** @copydoc Collider::updated() */
		virtual bool updated() const override { return mUpdated; };

		/** @copydoc Collider::resetUpdatedState() */
		virtual void resetUpdatedState() override { mUpdated = false; };

		/** @copydoc ConcaveCollider::processOverlapingParts() */
		virtual void processOverlapingParts(
			const AABB& aabb, float epsilon, const ConvexShapeCallback& callback
		) const override;

		/** @copydoc ConcaveCollider::processIntersectingParts() */
		virtual void processIntersectingParts(
			const Ray& ray, float epsilon, const ConvexShapeCallback& callback
		) const override;
	private:
		/** Calculates a new AABB Tree, this function must be called each time
		 * the TriangleMeshCollider is updated */
		void calculateAABBTree();

		/** Calculates the collider located at the given index
		 *
		 * @param	triangleIndex the index of the triangle
		 * @return	the TriangleCollider */
		TriangleCollider getTriangleCollider(
			unsigned short triangleIndex
		) const;
	};

}

#endif		// TRIANGLE_MESH_COLLIDER_H
