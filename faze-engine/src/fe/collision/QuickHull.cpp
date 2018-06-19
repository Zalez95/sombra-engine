#include <tuple>
#include <limits>
#include <algorithm>
#include "QuickHull.h"

namespace fe { namespace collision {

	HalfEdgeMesh QuickHull::calculate(const HalfEdgeMesh& meshData)
	{
		createInitialConvexHull(meshData);

		bool end = false;
		while (!end) {
			// 1. Find a convex hull face with a non empty outside vertices vector
			auto itFace = std::find_if(
				mFaceOutsideVertices.begin(), mFaceOutsideVertices.end(),
				[](const std::pair<int, std::vector<int>>& pair) { return !pair.second.empty(); }
			);
			if (itFace != mFaceOutsideVertices.end()) {
				// 2. Get the furthest HEVertex in the direction of the face normal
				int iEyeVertex = getFurthestVertex(itFace->second, itFace->first, mConvexHull);
				glm::vec3 eyePoint = mConvexHull.getVertex(iEyeVertex).location;

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

				// 4. Calculate the horizon edges from the current eyePoint
				// perspective
				std::vector<int> horizon, facesToRemove;
				std::tie(horizon, facesToRemove) = calculateHorizon(eyePoint, itFace->first, mConvexHull);

				// 5. Create new faces by joining the edges of the horizon with
				// the convex hull eyePoint
				for (int iHorizonEdge : horizon) {
					const HEEdge& currentEdge		= mConvexHull.getEdge(iHorizonEdge);
					const HEEdge& oppositeEdge	= mConvexHull.getEdge(currentEdge.oppositeEdge);

					int iV1 = mConvexHull.getEdge(currentEdge.oppositeEdge).vertex;
					int iV2 = currentEdge.vertex;

					int iNewFace = mConvexHull.addFace({ iV1, iV2, iEyeVertexConvexHull });
					mFaceOutsideVertices.emplace(iNewFace, getVerticesOutside(itFace->second, meshData, iNewFace));

					// Test if the new face is coplanar with the opposite one by
					// the horizon edge
					glm::vec3 currentFaceNormal = calculateFaceNormal(iNewFace, mConvexHull);
					glm::vec3 oppositeFaceNormal = calculateFaceNormal(oppositeEdge.face, mConvexHull);
					if (currentFaceNormal == oppositeFaceNormal) {
						// Merge the coplanar faces
						mConvexHull.mergeFace(iHorizonEdge);
					}
				}

				// 6. Remove the faces seen from the current eyePoint
				for (int iFaceToRemove : facesToRemove) {
					mConvexHull.removeFace(iFaceToRemove);
					mFaceOutsideVertices.erase(mFaceOutsideVertices.find(iFaceToRemove));
				}
			}
			else {
				end = true;
			}
		}

		return mConvexHull;
	}

// Private functions
	void QuickHull::createInitialConvexHull(const HalfEdgeMesh& meshData)
	{
		// Add the vertices to the convex hull
		std::vector<int> iSimplexVertices = calculateInitialSimplex(meshData);
		std::vector<int> iCHVertices;
		for (int iMeshVertex : iSimplexVertices) {
			int iConvexHullVertex = mConvexHull.addVertex(meshData.getVertex(iMeshVertex).location);
			mVertexIndexMap.emplace(iMeshVertex, iConvexHullVertex);
			iCHVertices.push_back(iMeshVertex);
		}

		// Add the faces to the convex hull
		int f1 = mConvexHull.addFace({ iCHVertices[0], iCHVertices[1], iCHVertices[2] });
		int f2 = mConvexHull.addFace({ iCHVertices[0], iCHVertices[3], iCHVertices[1] });
		int f3 = mConvexHull.addFace({ iCHVertices[0], iCHVertices[2], iCHVertices[3] });
		int f4 = mConvexHull.addFace({ iCHVertices[1], iCHVertices[3], iCHVertices[2] });

		mFaceOutsideVertices.emplace(f1, getVerticesOutside(iCHVertices, meshData, f1));
		mFaceOutsideVertices.emplace(f2, getVerticesOutside(iCHVertices, meshData, f2));
		mFaceOutsideVertices.emplace(f3, getVerticesOutside(iCHVertices, meshData, f3));
		mFaceOutsideVertices.emplace(f4, getVerticesOutside(iCHVertices, meshData, f4));
	}


	std::vector<int> QuickHull::calculateInitialSimplex(const HalfEdgeMesh& meshData) const
	{
		std::vector<int> iSimplexVertices(4, -1);

		// 1. Find the extreme vertices in each axis
		std::vector<int> extremePointIndices(6, 0);
		for (int i = 1; i < meshData.getNumVertices(); ++i) {
			for (int j = 0; j < 3; ++j) {
				if (meshData.getVertex(i).location[j] < meshData.getVertex(extremePointIndices[2*j]).location[j]) {
					extremePointIndices[2*j] = i;
				}
				if (meshData.getVertex(i).location[j] > meshData.getVertex(extremePointIndices[2*j + 1]).location[j]) {
					extremePointIndices[2*j + 1] = i;
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
					iSimplexVertices[0] = i;
					iSimplexVertices[1] = j;
					maxLength = currentLength;
				}
			}
		}

		// 3. Find the furthest point to the edge between the last 2 vertices
		glm::vec3 p1 = meshData.getVertex(iSimplexVertices[0]).location;
		glm::vec3 p2 = meshData.getVertex(iSimplexVertices[1]).location;
		glm::vec3 dirP1P2 = glm::normalize(p2 - p1);
		maxLength = -std::numeric_limits<float>::max();
		for (int i = 0; i < meshData.getNumVertices(); ++i) {
			glm::vec3 currentPoint = meshData.getVertex(i).location;
			glm::vec3 projection = p1 + dirP1P2 * glm::dot(currentPoint - p1, dirP1P2);
			float currentLength = glm::length(currentPoint - projection);
			if (currentLength > maxLength) {
				iSimplexVertices[2] = i;
				maxLength = currentLength;
			}
		}

		// 4. Find the furthest point to the triangle created from the last 3
		// vertices
		glm::vec3 p3 = meshData.getVertex(iSimplexVertices[2]).location;
		glm::vec3 dirP1P3 = glm::normalize(p3 - p1);
		glm::vec3 tNormal = glm::normalize(glm::cross(dirP1P2, dirP1P3));
		maxLength = -std::numeric_limits<float>::max();
		for (int i = 0; i < meshData.getNumVertices(); ++i) {
			glm::vec3 currentPoint = meshData.getVertex(i).location;
			float currentLength = std::abs(glm::dot(currentPoint, tNormal));
			if (currentLength > maxLength) {
				iSimplexVertices[3] = i;
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
		// Get the face data from the convex hull
		HEFace face = mConvexHull.getFace(iFace);
		glm::vec3 faceNormal = calculateFaceNormal(iFace, mConvexHull);
		const HEVertex& faceVertex = mConvexHull.getVertex(mConvexHull.getEdge(face.edge).vertex);

		std::vector<int> verticesOutside;
		for (int i : vertexIndices) {
			float currentDistance = glm::dot(meshData.getVertex(i).location - faceVertex.location, faceNormal);

			if (currentDistance > 0) {
				verticesOutside.push_back(i);
			}
			else if (currentDistance == 0) {
				// Check if the vertex index is in mVertexIndexMap
				auto itVertexPair = mVertexIndexMap.find(i);
				if (itVertexPair != mVertexIndexMap.end()) {
					// Check if the vertex index is in any of the HEEdges of the HEFace
					bool vertexInFace = false;
					int iInitialEdge = face.edge;
					int iCurrentEdge = iInitialEdge;
					do {
						HEEdge currentEdge = meshData.getEdge(iCurrentEdge);
						if (currentEdge.vertex == itVertexPair->second) {
							vertexInFace = true;
							break;
						}
						iCurrentEdge = currentEdge.nextEdge;
					}
					while (iCurrentEdge != iInitialEdge);

					if (vertexInFace) {
						verticesOutside.push_back(i);
					}
				}
			}
		}

		return verticesOutside;
	}


	int QuickHull::getFurthestVertex(
		const std::vector<int>& vertexIndices,
		int iFace, const HalfEdgeMesh& meshData
	) const
	{
		glm::vec3 faceNormal = calculateFaceNormal(iFace, meshData);
		const HEVertex& faceVertex = meshData.getVertex(meshData.getEdge(meshData.getFace(iFace).edge).vertex);

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

}}
