#include <tuple>
#include <limits>
#include <algorithm>
#include "QuickHull.h"

namespace fe { namespace collision {

	void QuickHull::calculate(const HalfEdgeMesh& meshData)
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
			int iEyeVertex = getFurthestVertexFrom(itFace->second, meshData, mFaceNormals[itFace->first]);
			glm::vec3 eyePoint = meshData.getVertex(iEyeVertex).location;

			// 3. Check if the eyePoint is already inside in the convex hull
			int iEyeVertexConvexHull;
			auto itVertex = mVertexIndexMap.find(iEyeVertex);
			if (itVertex == mVertexIndexMap.end()) {
				// 3.1 Add the eyePoint to the convex hull
				iEyeVertexConvexHull = mConvexHull.addVertex(eyePoint);
				mVertexIndexMap.emplace(iEyeVertex, iEyeVertexConvexHull);

				// 3.2. Calculate the horizon HEEdges and HEFaces to remove from
				// the current eyePoint perspective
				std::vector<int> horizon, facesToRemove;
				std::tie(horizon, facesToRemove) = calculateHorizon(eyePoint, itFace->first, mConvexHull, mFaceNormals);

				// 3.3. Remove the HEFaces seen from the current eyePoint and
				// collect all their outside HEVertices
				std::vector<int> allOutsideVertices;
				for (int iFaceToRemove : facesToRemove) {
					mConvexHull.removeFace(iFaceToRemove);
					mFaceNormals.erase(iFaceToRemove);

					auto itFOutsideVertices = mFaceOutsideVertices.find(iFaceToRemove);

					std::vector<int> joinedOutsideVertices;
					std::sort(itFOutsideVertices->second.begin(), itFOutsideVertices->second.end());
					std::set_union(
						allOutsideVertices.begin(), allOutsideVertices.end(),
						itFOutsideVertices->second.begin(), itFOutsideVertices->second.end(),
						std::back_inserter(joinedOutsideVertices)
					);
					allOutsideVertices = joinedOutsideVertices;

					mFaceOutsideVertices.erase(itFOutsideVertices);
				}

				// 3.4. Create new HEFaces by joining the edges of the horizon
				// with the convex hull eyePoint
				for (int iHorizonEdge : horizon) {
					const HEEdge& currentEdge	= mConvexHull.getEdge(iHorizonEdge);
					const HEEdge& oppositeEdge	= mConvexHull.getEdge(currentEdge.oppositeEdge);

					// Create the new HEFace
					int iV0 = oppositeEdge.vertex, iV1 = currentEdge.vertex;
					int iNewFace = mConvexHull.addFace({ iV0, iV1, iEyeVertexConvexHull });
					mFaceNormals.emplace(iNewFace, calculateFaceNormal(iNewFace, mConvexHull));
					mFaceOutsideVertices.emplace(iNewFace, getVerticesOutside(allOutsideVertices, meshData, iNewFace));

					// Merge the coplanar faces
					mergeCoplanarFaces(iNewFace);
				}
			}
		}
	}


	void QuickHull::resetData()
	{
		mConvexHull = HalfEdgeMesh();
		mFaceNormals.clear();
		mFaceOutsideVertices.clear();
		mVertexIndexMap.clear();
	}

// Private functions
	void QuickHull::createInitialConvexHull(const HalfEdgeMesh& meshData)
	{
		// Calculate an initial simplex from the meshData
		std::vector<int> iSimplexVertices = calculateInitialSimplex(meshData);

		// Add the vertices to the convex hull
		std::vector<int> chVertexIndices;
		for (int iMeshVertex : iSimplexVertices) {
			int iConvexHullVertex = mConvexHull.addVertex(meshData.getVertex(iMeshVertex).location);
			mVertexIndexMap.emplace(iMeshVertex, iConvexHullVertex);
			chVertexIndices.push_back(iConvexHullVertex);
		}

		// Add the faces to the convex hull, 
		const glm::vec3 p0 = mConvexHull.getVertex(chVertexIndices[0]).location,
						p1 = mConvexHull.getVertex(chVertexIndices[1]).location,
						p2 = mConvexHull.getVertex(chVertexIndices[2]).location,
						p3 = mConvexHull.getVertex(chVertexIndices[3]).location;
		int iF0, iF1, iF2, iF3;
		const glm::vec3 tNormal = glm::cross(p1 - p0, p2 - p0);
		if (glm::dot(p3 - p0, tNormal) <= 0.0f) {
			iF0 = mConvexHull.addFace({ chVertexIndices[0], chVertexIndices[1], chVertexIndices[2] });
			iF1 = mConvexHull.addFace({ chVertexIndices[0], chVertexIndices[3], chVertexIndices[1] });
			iF2 = mConvexHull.addFace({ chVertexIndices[0], chVertexIndices[2], chVertexIndices[3] });
			iF3 = mConvexHull.addFace({ chVertexIndices[1], chVertexIndices[3], chVertexIndices[2] });
		}
		else {
			iF0 = mConvexHull.addFace({ chVertexIndices[0], chVertexIndices[2], chVertexIndices[1] });
			iF1 = mConvexHull.addFace({ chVertexIndices[0], chVertexIndices[1], chVertexIndices[3] });
			iF2 = mConvexHull.addFace({ chVertexIndices[0], chVertexIndices[3], chVertexIndices[2] });
			iF3 = mConvexHull.addFace({ chVertexIndices[1], chVertexIndices[2], chVertexIndices[3] });
		}

		// Add the HEFaces normals
		mFaceNormals.emplace(iF0, calculateFaceNormal(iF0, mConvexHull));
		mFaceNormals.emplace(iF1, calculateFaceNormal(iF1, mConvexHull));
		mFaceNormals.emplace(iF2, calculateFaceNormal(iF2, mConvexHull));
		mFaceNormals.emplace(iF3, calculateFaceNormal(iF3, mConvexHull));

		// Add the HEFaces outside vertices
		std::vector<int> meshVertexIndices;
		for (auto it = meshData.getVerticesVector().begin(); it != meshData.getVerticesVector().end(); ++it) {
			meshVertexIndices.push_back(it.getIndex());
		}
		mFaceOutsideVertices.emplace(iF0, getVerticesOutside(meshVertexIndices, meshData, iF0));
		mFaceOutsideVertices.emplace(iF1, getVerticesOutside(meshVertexIndices, meshData, iF1));
		mFaceOutsideVertices.emplace(iF2, getVerticesOutside(meshVertexIndices, meshData, iF2));
		mFaceOutsideVertices.emplace(iF3, getVerticesOutside(meshVertexIndices, meshData, iF3));
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
			if (currentDistance > mEpsilon) {
				verticesOutside.push_back(i);
			}
		}

		return verticesOutside;
	}


	int QuickHull::getFurthestVertexFrom(
		const std::vector<int>& vertexIndices, const HalfEdgeMesh& meshData,
		const glm::vec3& direction
	) const
	{
		auto itMaxVertex = std::max_element(vertexIndices.begin(), vertexIndices.end(), [&](int iV1, int iV2) {
			float dot1 = glm::dot(meshData.getVertex(iV1).location, direction);
			float dot2 = glm::dot(meshData.getVertex(iV2).location, direction);
			return dot1 < dot2;
		});
		return (itMaxVertex != vertexIndices.end())? *itMaxVertex : -1;
	}


	void QuickHull::mergeCoplanarFaces(int iFace)
	{
		HEFace inputFace = mConvexHull.getFace(iFace);
		glm::vec3 inputFaceNormal = mFaceNormals.find(iFace)->second;

		// Test all the HEEFace edges
		bool initialEdgeUpdated;
		int iInitialEdge = inputFace.edge;
		int iCurrentEdge = iInitialEdge;
		do {
			const HEEdge& currentEdge	= mConvexHull.getEdge(iCurrentEdge);
			const HEEdge& oppositeEdge	= mConvexHull.getEdge(currentEdge.oppositeEdge);

			initialEdgeUpdated = false;
			int iNextEdge = currentEdge.nextEdge;

			// Check if the opposite HEFace exists
			if (oppositeEdge.face >= 0) {
				// Test if the current HEFace is coplanar with the opposite
				// one by the current edge
				glm::vec3 oppositeFaceNormal = mFaceNormals.find(oppositeEdge.face)->second;
				if (inputFaceNormal == oppositeFaceNormal) {
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
			}

			iCurrentEdge = iNextEdge;
		}
		while ((iCurrentEdge != iInitialEdge) || initialEdgeUpdated);
	}

}}
