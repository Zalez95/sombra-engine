#include <limits>
#include "QuickHull.h"

namespace fe { namespace collision {

	QuickHull::QuickHull(const MeshAdjacencyData& meshData)
	{
		createInitialHull(meshData);
		for (int iFace = 0; iFace < mConvexHull.getNumFaces(); ++iFace) {
			// 1. Get the vertices outside of the current Hull by the given face
			std::vector<int> verticesOutside = getVerticesOutside(iFace, meshData);
			if (!verticesOutside.empty()) {
				// 2. Get the furthest point in the direction of the face normal
				int iEyePoint = getFurthestPoint(iFace, verticesOutside, meshData);
				glm::vec3 eyePoint = meshData.getVertex(iEyePoint).location;

				// 3. Calculate the horizon edges from the current eyePoint
				// perspective
				std::vector<int> horizon = calculateHorizon(eyePoint, iFace, meshData).first;

				// 4. Create the new faces of the hull by joining the edges of
				// the horizon with the eyePoint
				for (int iEdge : horizon) {
					const Edge& currentEdge	= meshData.getEdge(iEdge);
					const Edge& nextEdge	= meshData.getEdge(currentEdge.nextEdge);
					// FIXME: add the points with the correct indexes
					// FIXME: only if the face of the current edge isn't coplanar to the one in the opposite edge
					mConvexHull.addFace({ iEyePoint, currentEdge.vertex, nextEdge.vertex });
				}
			}
		}
	}


	void QuickHull::createInitialHull(const MeshAdjacencyData& meshData)
	{
		std::array<int, 4> simplex;

		// 1. Find the extreme vertices in each axis
		std::array<size_t, 6> extremePointIndices{};
		for (int i = 0; i < meshData.getNumVertices(); ++i) {
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
		for (int i = 0; i < meshData.getNumVertices(); ++i) {
			for (int j = i + 1; j < meshData.getNumVertices(); ++j) {
				float currentLength = glm::length(meshData.getVertex(j).location - meshData.getVertex(i).location);
				if (currentLength > maxLength) {
					simplex[0] = i;
					simplex[1] = j;
					maxLength = currentLength;
				}
			}
		}

		// 3. Find the furthest point to the edge between the last 2 vertices
		glm::vec3 dirP1P2 = glm::normalize(meshData.getVertex(simplex[1]).location - meshData.getVertex(simplex[0]).location);
		maxLength = -std::numeric_limits<float>::max();
		for (int i = 0; i < meshData.getNumVertices(); ++i) {
			glm::vec3 projection = meshData.getVertex(simplex[0]).location + dirP1P2 * glm::dot(meshData.getVertex(i).location, dirP1P2);
			float currentLength = glm::length(meshData.getVertex(i).location - projection);
			if (currentLength > maxLength) {
				simplex[2] = i;
				maxLength = currentLength;
			}
		}

		// 4. Find the furthest point to the triangle created from the last 3
		// vertices
		glm::vec3 dirP1P3 = glm::normalize(meshData.getVertex(simplex[2]).location - meshData.getVertex(simplex[0]).location);
		glm::vec3 tNormal = glm::normalize(glm::cross(dirP1P2, dirP1P3));
		maxLength = -std::numeric_limits<float>::max();
		for (int i = 0; i < meshData.getNumVertices(); ++i) {
			float currentLength = std::abs(glm::dot(meshData.getVertex(i).location, tNormal));
			if (currentLength > maxLength) {
				simplex[3] = i;
				maxLength = currentLength;
			}
		}

		for (int i : simplex) {
			mConvexHull.addVertex(meshData.getVertex(i).location);
		}
		mConvexHull.addFace({ 0, 1, 2 });
		mConvexHull.addFace({ 0, 3, 1 });
		mConvexHull.addFace({ 0, 2, 3 });
		mConvexHull.addFace({ 1, 3, 2 });
	}


	std::vector<int> QuickHull::getVerticesOutside(
		int iFace,
		const MeshAdjacencyData& meshData
	) const
	{
		glm::vec3 faceNormal = calculateFaceNormal(iFace, meshData);
		const Vertex& faceVertex = meshData.getVertex( meshData.getEdge(meshData.getFace(iFace).edge).vertex );

		std::vector<int> verticesOutside;
		for (int i = 0; i < meshData.getNumVertices(); ++i) {
			float currentDistance = glm::dot(meshData.getVertex(i).location - faceVertex.location, faceNormal);

			if (currentDistance > 0) {
				verticesOutside.push_back(i);
			}
			else if (currentDistance == 0) {
				bool vertexInFace = false;

				int iInitialEdge = meshData.getFace(iFace).edge;
				int iCurrentEdge = iInitialEdge;
				do {
					Edge currentEdge = meshData.getEdge(iCurrentEdge);
					if (currentEdge.vertex == i) {
						vertexInFace = true;
						break;
					}
					iCurrentEdge = currentEdge.nextEdge;
				}
				while (iCurrentEdge != iInitialEdge);

				if (!vertexInFace) {
					verticesOutside.push_back(i);
				}
			}
		}

		return verticesOutside;
	}


	int QuickHull::getFurthestPoint(
		int iFace,
		const std::vector<int>& vertexIndices,
		const MeshAdjacencyData& meshData
	) const
	{
		glm::vec3 faceNormal = calculateFaceNormal(iFace, meshData);	// TODO: cache the face normals
		const Vertex& faceVertex = meshData.getVertex(meshData.getEdge(meshData.getFace(iFace).edge).vertex);

		int furthestPoint;
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
