#ifndef HALF_EDGE_MESH_RAYCAST_HPP
#define HALF_EDGE_MESH_RAYCAST_HPP

#include <limits>
#include <algorithm>
#include "se/collision/HalfEdgeMeshExt.h"

namespace se::collision {

	/** Holds the data of each node in the kd-tree */
	template <unsigned int maxHeight>
	struct HalfEdgeMeshRaycast<maxHeight>::TreeNode
	{
		/** The indices of the HEFaces of the current node */
		std::vector<int> iFaces;

		/** The bounding box that contains all of the current HEFaces */
		AABB aabb;

		/** The index of our left child node in the kd-tree */
		int iLeftChild;

		/** The index of our right child node in the kd-tree */
		int iRightChild;
	};


	/** Struct KDBuildStackContent, its the data structure used in the kd-tree
	 * build algorithm for storing its state in a stack instead of using a
	 * recursive algorithm */
	template <unsigned int maxHeight>
	struct HalfEdgeMeshRaycast<maxHeight>::KDBuildStackContent
	{
		/** The state of the current iteration */
		enum { Build, Children, End } state;

		/** The return value of the current "recursion" */
		int* returnValue;

		/** The faces of the current kd-tree node */
		std::vector<int> faces;

		/** The AABB of the faces of the current kd-tree node */
		AABB aabb;

		/** The current "recursion" depth */
		unsigned int depth;

		/** The left and right kd-tree node indices */
		int iLeftChild, iRightChild;
	};


	/** Struct KDHitStackContent, its the data structure used in the kd-tree
	 * ray hit algorithm for storing its state in a stack instead of using a
	 * recursive algorithm */
	template <unsigned int maxHeight>
	struct HalfEdgeMeshRaycast<maxHeight>::KDHitStackContent
	{
		/** The state of the current iteration */
		enum { Check, Children, End } state;

		/** The return value of the current "recursion" */
		RayHit* returnValue;

		/** The index of the current node */
		int iCurrentNode;

		/** The left and right kd-tree child nodes return value */
		RayHit leftRayHit, rightLightHit;
	};


	template <unsigned int maxHeight>
	void HalfEdgeMeshRaycast<maxHeight>::buildKDTree(
		const HalfEdgeMesh* mesh,
		const utils::PackedVector<glm::vec3>* faceNormals
	) {
		mMesh = mesh;
		mFaceNormals = faceNormals;
		mKDTree.clear();
		mIRootNode = -1;

		utils::FixedVector<KDBuildStackContent, 2 * maxHeight> stack;

		// Push the root node to the stack
		std::vector<int> allFaceIndices;
		for (auto itFace = mMesh->faces.begin(); itFace != mMesh->faces.end(); ++itFace) {
			allFaceIndices.push_back(itFace.getIndex());
		}
		AABB meshAABB = calculateAABB(*mMesh);
		stack.push_back({ KDBuildStackContent::Build, &mIRootNode, allFaceIndices, meshAABB, 0, -1, -1 });

		while (!stack.empty()) {
			auto& [state, returnValue, faces, aabb, depth, iLeftChild, iRightChild] = stack.back();
			switch (state) {
				case KDBuildStackContent::Build:
					if ((faces.size() > 1) && (depth < kMaxDepth)) {
						// Calculate the split axis
						glm::vec3 minToMax = aabb.maximum - aabb.minimum;
						int iSplitAxis = std::distance(&minToMax.x, std::max_element(&minToMax.x, &minToMax.x + 3));

						// Split the HEFaces by the split axis at its middle point
						std::vector<int> leftFaces, rightFaces;
						float middle = aabb.minimum[iSplitAxis] + 0.5f * minToMax[iSplitAxis];
						for (int iFace : faces) {
							bool anyVerticesLeft = false;

							int iInitialEdge = mMesh->faces[iFace].edge;
							int iCurrentEdge = iInitialEdge;
							do {
								const HEEdge& currentEdge = mMesh->edges[iCurrentEdge];

								const glm::vec3& location = mMesh->vertices[currentEdge.vertex].location;
								if (location[iSplitAxis] > middle) {
									anyVerticesLeft = true;
									break;
								}

								iCurrentEdge = currentEdge.nextEdge;
							}
							while (iCurrentEdge != iInitialEdge);

							if (anyVerticesLeft) {
								leftFaces.push_back(iFace);
							}
							else {
								rightFaces.push_back(iFace);
							}
						}

						// Calculate the AABBs of the children kd-tree nodes
						AABB leftAABB = calculateAABBFromFaces(leftFaces);
						AABB rightAABB = calculateAABBFromFaces(rightFaces);

						// Process the child nodes
						stack.push_back({ KDBuildStackContent::Build, &iRightChild, rightFaces, rightAABB, depth + 1, -1, -1 });
						stack.push_back({ KDBuildStackContent::Build, &iLeftChild, leftFaces, leftAABB, depth + 1, -1, -1 });
						state = KDBuildStackContent::Children;
					}
					else {
						// Create the kd-tree node
						mKDTree.push_back({ faces, aabb, -1, -1 });
						*returnValue = mKDTree.size() - 1;
						state = KDBuildStackContent::End;
					}
					break;
				case KDHitStackContent::Children:
					// Create the kd-tree node
					mKDTree.push_back({ std::vector<int>(), aabb, iLeftChild, iRightChild });
					*returnValue = mKDTree.size() - 1;
					state = KDBuildStackContent::End;
					break;
				case KDBuildStackContent::End:
					stack.pop_back();
					break;
			}
		}
	}


	template <unsigned int maxHeight>
	RayHit HalfEdgeMeshRaycast<maxHeight>::closestHit(
		const glm::vec3& rayOrigin, const glm::vec3& rayDirection
	) const
	{
		RayHit ret = { false, -1, glm::vec3(0.0f), std::numeric_limits<float>::max() };

		utils::FixedVector<KDHitStackContent, 2 * maxHeight> stack;

		// Push the root node to the stack
		stack.push_back({ KDHitStackContent::Check, &ret, mIRootNode, RayHit(), RayHit() });

		while (!stack.empty()) {
			auto& [state, returnValue, iCurrentNode, leftRayHit, rightRayHit] = stack.back();
			switch (state) {
				case KDHitStackContent::Check:
					*returnValue = { false, -1, glm::vec3(0.0f), std::numeric_limits<float>::max() };
					if (intersects(mKDTree[iCurrentNode].aabb, rayOrigin, rayDirection, mEpsilon)) {
						if ((mKDTree[iCurrentNode].iLeftChild < 0) || (mKDTree[iCurrentNode].iRightChild < 0)) {
							// Search the closest intersection to the rayOrigin
							// with the current HEFaces
							for (int iFace : mKDTree[iCurrentNode].iFaces) {
								glm::vec3 facePoint = mMesh->vertices[ mMesh->edges[ mMesh->faces[iFace].edge ].vertex ].location;
								glm::vec3 faceNormal = (*mFaceNormals)[iFace];

								auto [intersects, intersection] = utils::rayPlaneIntersection(rayOrigin, rayDirection, facePoint, faceNormal, mEpsilon);
								if (intersects && isPointBetweenHEEdges(*mMesh, mMesh->faces[iFace].edge, faceNormal, intersection)) {
									float distance = glm::length(intersection - rayOrigin);
									if (distance < returnValue->distance) {
										*returnValue = { true, iFace, intersection, distance };
									}
								}
							}
							state = KDHitStackContent::End;
						}
						else {
							// Check if there is an intersection with the
							// child nodes
							stack.push_back({ KDHitStackContent::Check, &rightRayHit, mKDTree[iCurrentNode].iRightChild, RayHit(), RayHit() });
							stack.push_back({ KDHitStackContent::Check, &leftRayHit, mKDTree[iCurrentNode].iLeftChild, RayHit(), RayHit() });
							state = KDHitStackContent::Children;
						}
					}
					else {
						state = KDHitStackContent::End;
					}
					break;
				case KDHitStackContent::Children:
					*returnValue = (leftRayHit.distance < rightRayHit.distance)? leftRayHit : rightRayHit;
					state = KDHitStackContent::End;
					break;
				case KDHitStackContent::End:
					stack.pop_back();
					break;
			}
		}

		return ret;
	}

// Private functions
	template <unsigned int maxHeight>
	AABB HalfEdgeMeshRaycast<maxHeight>::calculateAABBFromFaces(const std::vector<int>& faceIndices) const
	{
		AABB ret = {
			glm::vec3( std::numeric_limits<float>::max()),
			glm::vec3(-std::numeric_limits<float>::max())
		};

		for (int iFace : faceIndices) {

			int iInitialEdge = mMesh->faces[iFace].edge;
			int iCurrentEdge = iInitialEdge;
			do {
				const HEEdge& currentEdge = mMesh->edges[iCurrentEdge];
				const HEVertex& vertex = mMesh->vertices[currentEdge.vertex];

				ret.minimum = glm::min(ret.minimum, vertex.location);
				ret.maximum = glm::max(ret.maximum, vertex.location);

				iCurrentEdge = currentEdge.nextEdge;
			}
			while (iCurrentEdge != iInitialEdge);
		}

		return ret;
	}


	template <unsigned int maxHeight>
	bool HalfEdgeMeshRaycast<maxHeight>::isPointBetweenHEEdges(
		const HalfEdgeMesh& meshData, int iInitialEdge,
		const glm::vec3& loopNormal, const glm::vec3& point
	) const
	{
		bool inside = true;

		int iCurrentEdge = iInitialEdge;
		do {
			const HEEdge& currentEdge = meshData.edges[iCurrentEdge];
			const HEEdge& oppositeEdge = meshData.edges[currentEdge.oppositeEdge];

			glm::vec3 p1 = meshData.vertices[oppositeEdge.vertex].location;
			glm::vec3 p2 = meshData.vertices[currentEdge.vertex].location;
			if (glm::dot(glm::cross(p2 - p1, loopNormal), point - p1) > mEpsilon) {
				inside = false;
			}

			iCurrentEdge = currentEdge.nextEdge;
		}
		while ((iCurrentEdge != iInitialEdge) && inside);

		return inside;
	}

}

#endif		// HALF_EDGE_MESH_RAYCAST_HPP
