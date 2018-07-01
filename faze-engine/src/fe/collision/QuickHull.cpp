#include <tuple>
#include <limits>
#include <algorithm>
#include "QuickHull.h"

namespace fe { namespace collision {

	HalfEdgeMesh QuickHull::calculate(const HalfEdgeMesh& meshData)
	{
		createInitialConvexHull(meshData);

		// 1. Find a convex hull face with a non empty outside vertices vector
		auto itFace = mFaceOutsideVertices.begin();
		while ((itFace = std::find_if(
					mFaceOutsideVertices.begin(), mFaceOutsideVertices.end(),
					[](const std::pair<int, std::vector<int>>& pair) { return !pair.second.empty(); }
				)
			) != mFaceOutsideVertices.end()
		) {
			// 2. Get the furthest HEVertex in the direction of the face normal
			int iEyeVertex = getFurthestVertex(itFace->second, itFace->first, meshData);
			glm::vec3 eyePoint = meshData.getVertex(iEyeVertex).location;

			// 3. Add the eyePoint to the convex hull if it isn't already inside
			int iEyeVertexConvexHull;
			auto itVertex = mVertexIndexMap.find(iEyeVertex);
			if (itVertex != mVertexIndexMap.end()) {
				iEyeVertexConvexHull = itVertex->second;
			}
			else {
				iEyeVertexConvexHull = mConvexHull.addVertex(eyePoint);
				mVertexIndexMap.emplace(iEyeVertex, iEyeVertexConvexHull);
			}

			// 4. Calculate the horizon HEEdges and HEFaces to remove from the
			// current eyePoint perspective
			std::vector<int> horizon, facesToRemove;
			std::tie(horizon, facesToRemove) = calculateHorizon(eyePoint, itFace->first, mConvexHull, mFaceNormals);

			// 5. Remove the HEFaces seen from the current eyePoint and collect
			// all their outside HEVertices
			std::vector<int> allVerticesOutside;
			for (int iFaceToRemove : facesToRemove) {
				mConvexHull.removeFace(iFaceToRemove);
				mFaceNormals.erase(iFaceToRemove);

				auto itFOutisideVertices = mFaceOutsideVertices.find(iFaceToRemove);
				if (itFOutisideVertices != mFaceOutsideVertices.end()) {
					std::vector<int>& faceOutsideVerticesRemoved = itFOutisideVertices->second;
					std::sort(faceOutsideVerticesRemoved.begin(), faceOutsideVerticesRemoved.end());

					std::vector<int> faceOutsideVerticesJoined;
					std::set_union(
						allVerticesOutside.begin(), allVerticesOutside.end(),
						faceOutsideVerticesRemoved.begin(), faceOutsideVerticesRemoved.end(),
						std::back_inserter(faceOutsideVerticesJoined)
					);
					allVerticesOutside = faceOutsideVerticesJoined;

					mFaceOutsideVertices.erase(itFOutisideVertices);
				}
			}

			// 6. Create new HEFaces by joining the edges of the horizon with
			// the convex hull eyePoint
			for (int iHorizonEdge : horizon) {
				const HEEdge& currentEdge	= mConvexHull.getEdge(iHorizonEdge);
				const HEEdge& oppositeEdge	= mConvexHull.getEdge(currentEdge.oppositeEdge);

				// Create the new HEFace
				int iV0 = oppositeEdge.vertex, iV1 = currentEdge.vertex;
				int iNewFace = mConvexHull.addFace({ iV0, iV1, iEyeVertexConvexHull });
				mFaceNormals.emplace(iNewFace, calculateFaceNormal(iNewFace, mConvexHull));
				mFaceOutsideVertices.emplace(iNewFace, getVerticesOutside(allVerticesOutside, meshData, iNewFace));

				// Merge the coplanar faces
				mergeCoplanarFaces(iNewFace);
			}
		}

		return mConvexHull;
	}

// Private functions
	void QuickHull::createInitialConvexHull(const HalfEdgeMesh& meshData)
	{
		// Calculate an initial simplex from the meshData
		std::vector<int> iSimplexVertices = calculateInitialSimplex(meshData);

		// Add the vertices to the convex hull
		std::vector<int> iCHVertices;
		for (int iMeshVertex : iSimplexVertices) {
			int iConvexHullVertex = mConvexHull.addVertex(meshData.getVertex(iMeshVertex).location);
			mVertexIndexMap.emplace(iMeshVertex, iConvexHullVertex);
			iCHVertices.push_back(iConvexHullVertex);
		}

		// Add the faces to the convex hull
		int iF0 = mConvexHull.addFace({ iCHVertices[0], iCHVertices[1], iCHVertices[2] });
		int iF1 = mConvexHull.addFace({ iCHVertices[0], iCHVertices[3], iCHVertices[1] });
		int iF2 = mConvexHull.addFace({ iCHVertices[0], iCHVertices[2], iCHVertices[3] });
		int iF3 = mConvexHull.addFace({ iCHVertices[1], iCHVertices[3], iCHVertices[2] });

		mFaceNormals.emplace(iF0, calculateFaceNormal(iF0, mConvexHull));
		mFaceNormals.emplace(iF1, calculateFaceNormal(iF1, mConvexHull));
		mFaceNormals.emplace(iF2, calculateFaceNormal(iF2, mConvexHull));
		mFaceNormals.emplace(iF3, calculateFaceNormal(iF3, mConvexHull));

		std::vector<int> allVertexIndices;
		for (auto it = meshData.getVerticesVector().begin(); it != meshData.getVerticesVector().end(); ++it) {
			allVertexIndices.push_back(it.getIndex());
		}
		mFaceOutsideVertices.emplace(iF0, getVerticesOutside(allVertexIndices, meshData, iF0));
		mFaceOutsideVertices.emplace(iF1, getVerticesOutside(allVertexIndices, meshData, iF1));
		mFaceOutsideVertices.emplace(iF2, getVerticesOutside(allVertexIndices, meshData, iF2));
		mFaceOutsideVertices.emplace(iF3, getVerticesOutside(allVertexIndices, meshData, iF3));
	}


	std::vector<int> QuickHull::calculateInitialSimplex(const HalfEdgeMesh& meshData) const
	{
		std::vector<int> iSimplexVertices(4, -1);

		// 1. Find the extreme vertices in each axis
		std::vector<int> extremePointIndices(6, 0);
		for (auto it = meshData.getVerticesVector().begin(); it != meshData.getVerticesVector().end(); ++it) {
			for (int j = 0; j < 3; ++j) {
				if (it->location[j] < meshData.getVertex(extremePointIndices[2*j]).location[j]) {
					extremePointIndices[2*j] = it.getIndex();
				}
				if (it->location[j] > meshData.getVertex(extremePointIndices[2*j + 1]).location[j]) {
					extremePointIndices[2*j + 1] = it.getIndex();
				}
			}
		}

		// 2. Find from the extreme vertices the pair which are furthest apart
		float maxLength = -std::numeric_limits<float>::max();
		for (int i = 0; i < 6; ++i) {
			glm::vec3 p1 = meshData.getVertex(extremePointIndices[i]).location;

			for (int j = i + 1; j < 6; ++j) {
				glm::vec3 p2 = meshData.getVertex(extremePointIndices[j]).location;

				float currentLength = glm::length(p2 - p1);
				if (currentLength > maxLength) {
					iSimplexVertices[0] = extremePointIndices[i];
					iSimplexVertices[1] = extremePointIndices[j];
					maxLength = currentLength;
				}
			}
		}

		// 3. Find the furthest point to the edge between the last 2 vertices
		glm::vec3 p0 = meshData.getVertex(iSimplexVertices[0]).location;
		glm::vec3 p1 = meshData.getVertex(iSimplexVertices[1]).location;
		glm::vec3 dirP0P1 = glm::normalize(p1 - p0);
		maxLength = -std::numeric_limits<float>::max();
		for (auto it = meshData.getVerticesVector().begin(); it != meshData.getVerticesVector().end(); ++it) {
			glm::vec3 projection = p0 + dirP0P1 * glm::dot(it->location - p0, dirP0P1);
			float currentLength = glm::length(it->location - projection);
			if (currentLength > maxLength) {
				iSimplexVertices[2] = it.getIndex();
				maxLength = currentLength;
			}
		}

		// 4. Find the furthest point to the triangle created from the last 3
		// vertices
		glm::vec3 p2 = meshData.getVertex(iSimplexVertices[2]).location;
		glm::vec3 dirP0P2 = glm::normalize(p2 - p0);
		glm::vec3 tNormal = glm::normalize(glm::cross(dirP0P1, dirP0P2));
		maxLength = -std::numeric_limits<float>::max();
		for (auto it = meshData.getVerticesVector().begin(); it != meshData.getVerticesVector().end(); ++it) {
			float currentLength = std::abs(glm::dot(it->location, tNormal));
			if (currentLength > maxLength) {
				iSimplexVertices[3] = it.getIndex();
				maxLength = currentLength;
			}
		}

		return iSimplexVertices;
	}


	std::vector<int> QuickHull::getVerticesOutside(
		const std::vector<int>& vertexIndices, const HalfEdgeMesh& meshData,
		int iFace
	) const
	{
		std::vector<int> verticesOutside;
		if (!mConvexHull.getFacesVector().isActive(iFace)) { return verticesOutside; }

		// Get the face data from the convex hull
		const HEFace& face = mConvexHull.getFace(iFace);
		const glm::vec3 faceNormal = mFaceNormals.find(iFace)->second;
		const HEVertex& faceVertex = mConvexHull.getVertex(mConvexHull.getEdge(face.edge).vertex);

		for (int i : vertexIndices) {
			float currentDistance = glm::dot(meshData.getVertex(i).location - faceVertex.location, faceNormal);
			if (currentDistance > 0) {
				verticesOutside.push_back(i);
			}
		}

		return verticesOutside;
	}


	int QuickHull::getFurthestVertex(
		const std::vector<int>& vertexIndices,
		int iFace, const HalfEdgeMesh& meshData
	) const
	{
		if (!mConvexHull.getFacesVector().isActive(iFace)) { return -1; }

		const HEFace& face = mConvexHull.getFace(iFace);
		const glm::vec3 faceNormal = mFaceNormals.find(iFace)->second;
		const HEVertex& faceVertex = mConvexHull.getVertex(meshData.getEdge(face.edge).vertex);

		int furthestPoint = -1;
		float maxDistance = -std::numeric_limits<float>::max();
		for (int i : vertexIndices) {
			float currentDistance = glm::dot(meshData.getVertex(i).location - faceVertex.location, faceNormal);
			if (currentDistance > maxDistance) {
				maxDistance		= currentDistance;
				furthestPoint	= i;
			}
		}

		return furthestPoint;
	}


	void QuickHull::mergeCoplanarFaces(int iFace)
	{
		// Test all the HEEFace edges
		bool initialEdgeUpdated;
		int iInitialEdge = mConvexHull.getFace(iFace).edge;
		int iCurrentEdge = iInitialEdge;
		do {
			const HEEdge& currentEdge	= mConvexHull.getEdge(iCurrentEdge);
			const HEEdge& oppositeEdge	= mConvexHull.getEdge(currentEdge.oppositeEdge);

			initialEdgeUpdated = false;
			int iNextEdge = currentEdge.nextEdge;

			// Test if the current HEFace is coplanar with the opposite one by
			// the current edge
			glm::vec3 currentFaceNormal		= mFaceNormals.find(iFace)->second;
			glm::vec3 oppositeFaceNormal	= mFaceNormals.find(oppositeEdge.face)->second;
			if (currentFaceNormal == oppositeFaceNormal) {
				// Merge the two HEFaces into the current one
				int iRemovedFace = oppositeEdge.face;
				mConvexHull.mergeFace(iCurrentEdge);

				// Remove the opposite HEFace normal
				mFaceNormals.erase(iRemovedFace);

				// Remove the opposite HEFace outside HEVertices
				mFaceOutsideVertices.erase(iRemovedFace);

				// Update the iInitialEdge if it has been removed
				if (iCurrentEdge == iInitialEdge) {
					iInitialEdge = iNextEdge;
					initialEdgeUpdated = true;
				}
			}

			iCurrentEdge = iNextEdge;
		}
		while ((iCurrentEdge != iInitialEdge) || initialEdgeUpdated);
	}

}}
