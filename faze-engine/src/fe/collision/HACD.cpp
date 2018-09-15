#include <tuple>
#include <limits>
#include <algorithm>
#include <glm/gtc/constants.hpp>
#include "fe/collision/HACD.h"
#include "fe/collision/QuickHull.h"
#include "Geometry.h"

namespace fe { namespace collision {

	void HACD::calculate(const HalfEdgeMesh& meshData)
	{
		initData(meshData);

		bool end = false;
		while (!end) {
			end = true;

			// 1. Calculate the pair of vertices with the lowest decimation cost
			DualGraphVertex *bestVertex1 = nullptr, *bestVertex2 = nullptr;
			float lowestCost = std::numeric_limits<float>::max();
			for (DualGraphVertex& vertex1 : mDualGraph.vertices) {
				for (int iVertex2 : vertex1.neighbours) {
					auto itVertex2 = std::lower_bound(mDualGraph.vertices.begin(), mDualGraph.vertices.end(), iVertex2);

					// We filter the neighbour vertices already evaluated
					if ((itVertex2 != mDualGraph.vertices.end()) && (itVertex2->id > vertex1.id)) {
						// Calculate the surface created from the current
						// vertices and their ancestors
						auto surfaceFaceIndices = calculateSurfaceFaceIndices(vertex1, *itVertex2);

						// Calculate the cost of the surface
						float concavity = calculateConcavity(surfaceFaceIndices, mMesh, mFaceNormals);
						float aspectRatio = calculateAspectRatio(surfaceFaceIndices, mMesh);
						float currentCost = calculateDecimationCost(concavity, aspectRatio);

						if (currentCost < lowestCost) {
							lowestCost = currentCost;
							bestVertex1 = &vertex1;
							bestVertex2 = &(*itVertex2);
						}

						if (concavity < mMaximumConcavity * mNormalizationFactor) {
							end = false;
						}
					}
				}
			}

			if (!end) {
				// 2. Update the ancestors of the first vertex with the second one
				updateAncestors(*bestVertex1, *bestVertex2);

				// 3. Merge both nodes into the first one
				halfEdgeCollapse(bestVertex1->id, bestVertex2->id, mDualGraph);
			}
		}

		computeConvexSurfaces();
	}


	void HACD::resetData()
	{
		mFaceNormals.clear();
		mConvexSurfaces.clear();
	}

// Private functions
	void HACD::initData(const HalfEdgeMesh& meshData)
	{
		// 1. Calculate the triangulated mesh
		mMesh = meshData;
		triangulateFaces(mMesh);

		// 2. Calculate the face normals of the mesh
		for (auto it = mMesh.faces.begin(); it != mMesh.faces.end(); ++it) {
			mFaceNormals.emplace(it.getIndex(), calculateFaceNormal(mMesh, it.getIndex()));
		}

		// 3. Calculate the initial dual graph of the triangulated mesh
		mDualGraph = createDualGraph(mMesh);

		// 4. Calculate the AABB of the mesh
		AABB meshAABB = {
			glm::vec3( std::numeric_limits<float>::max()),
			glm::vec3(-std::numeric_limits<float>::max())
		};
		for (const HEVertex& vertex : mMesh.vertices) {
			meshAABB.minimum = glm::min(meshAABB.minimum, vertex.location);
			meshAABB.maximum = glm::max(meshAABB.maximum, vertex.location);
		}

		// 5. Calculate the normalization factor of the triangulated mesh
		mNormalizationFactor = calculateNormalizationFactor(meshAABB);

		// 6. Calculate the aspect ratio factor of the triangulated mesh
		mAspectRatioFactor = calculateAspectRatioFactor(mNormalizationFactor);
	}


	HACD::DualGraph HACD::createDualGraph(const HalfEdgeMesh& meshData) const
	{
		DualGraph dualGraph;

		// Create the dual graph from the Mesh HEFaces
		for (auto itFace = meshData.faces.begin(); itFace != meshData.faces.end(); ++itFace) {
			dualGraph.vertices.emplace_back(itFace.getIndex(), std::vector<int>());
		}
		std::sort(dualGraph.vertices.begin(), dualGraph.vertices.end());

		// Create the vertices neighbours from the Mesh adjacent HEFaces
		for (DualGraphVertex& v : dualGraph.vertices) {
			int iInitialEdge = meshData.faces[v.id].edge;
			int iCurrentEdge = iInitialEdge;
			do {
				HEEdge currentEdge = meshData.edges[iCurrentEdge];
				HEEdge oppositeEdge = meshData.edges[currentEdge.oppositeEdge];

				int iOtherVertex = oppositeEdge.face;
				auto itOtherVertex = std::lower_bound(dualGraph.vertices.begin(), dualGraph.vertices.end(), iOtherVertex);

				// Check if we already set the other vertex as a neighbour of
				// the current one
				if ((itOtherVertex != dualGraph.vertices.end())
					&& !std::binary_search(v.neighbours.begin(), v.neighbours.end(), iOtherVertex)
				) {
					// Set the other vertex as a neighbour of the current one
					v.neighbours.insert(
						std::lower_bound(v.neighbours.begin(), v.neighbours.end(), iOtherVertex),
						iOtherVertex
					);

					// Set the current vertex as a neighbour of the other one
					itOtherVertex->neighbours.insert(
						std::lower_bound(itOtherVertex->neighbours.begin(), itOtherVertex->neighbours.end(), v.id),
						v.id
					);
				}
			}
			while (iCurrentEdge != iInitialEdge);
		}

		return dualGraph;
	}


	std::vector<int> HACD::calculateSurfaceFaceIndices(
		const DualGraphVertex& vertex1, const DualGraphVertex& vertex2
	) const
	{
		std::vector<int> surfaceFaceIndices = { vertex1.id, vertex2.id };
		surfaceFaceIndices.insert(surfaceFaceIndices.end(), vertex1.data.begin(), vertex1.data.end());
		surfaceFaceIndices.insert(surfaceFaceIndices.end(), vertex2.data.begin(), vertex2.data.end());

		return surfaceFaceIndices;
	}


	float HACD::calculateConcavity(
		const std::vector<int>& iFaces,
		const HalfEdgeMesh& meshData, const std::map<int, glm::vec3>& faceNormals
	) const
	{
		// 1. Create a new surface HEMesh from the given meshData's HEFaces
		HalfEdgeMesh surface;
		std::map<int, int> vertexMap;

		for (int iFace1 : iFaces) {
			std::vector<int> iFace1Vertices = getFaceIndices(meshData, iFace1);
			std::vector<int> iFace2Vertices;
			for (int iVertex1 : iFace1Vertices) {
				auto itVertex1 = vertexMap.find(iVertex1);
				if (itVertex1 != vertexMap.end()) {
					iFace2Vertices.push_back(itVertex1->second);
				}
				else {
					int iVertex2 = addVertex(surface, meshData.vertices[iVertex1].location);
					vertexMap.emplace(iVertex1, iVertex2);
					iFace2Vertices.push_back(iVertex2);
				}
			}

			addFace(surface, iFace2Vertices);
		}

		// 2. Calculate the convex hull of the surface HEMesh
		QuickHull qh(mQuickHullEpsilon);
		qh.calculate(surface);
		const HalfEdgeMesh& convexHull = qh.getMesh();
		const std::map<int, glm::vec3>& convexHullNormals = qh.getNormalsMap();

		// 3. Calculate the maximum concavity as the distance of a point to its
		// projection on the convex hull of its mesh
		float maxConcavity = -std::numeric_limits<float>::max();
		for (auto iVertexPair : vertexMap) {
			glm::vec3 vertexLocation = meshData.vertices[iVertexPair.first].location;
			glm::vec3 vertexNormal = calculateVertexNormal(meshData, faceNormals, iVertexPair.first);

			bool intersects;
			glm::vec3 intersection;
			std::tie(intersects, intersection) = raycastInsideMesh(convexHull, convexHullNormals, vertexLocation, vertexNormal);
			if (intersects) {
				float currentConcavity = glm::length(intersection - vertexLocation);
				maxConcavity = std::max(maxConcavity, currentConcavity);
			}
		}

		return maxConcavity;
	}


	float HACD::calculateAspectRatio(const std::vector<int>& iFaces, const HalfEdgeMesh& meshData) const
	{
		// 1. Calculate the perimeter of the surface of the triangles
		float perimeter = 0.0f;
		for (int iFace : iFaces) {
			int iInitialEdge = meshData.faces[iFace].edge;
			int iCurrentEdge = iInitialEdge;
			do {
				const HEEdge& currentEdge = meshData.edges[iCurrentEdge];
				const HEEdge& oppositeEdge = meshData.edges[currentEdge.oppositeEdge];
				if (std::none_of(iFaces.begin(), iFaces.end(), [&](int iFace) { return iFace == oppositeEdge.face; })) {
					glm::vec3 sharedV1 = meshData.vertices[oppositeEdge.vertex].location;
					glm::vec3 sharedV2 = meshData.vertices[currentEdge.vertex].location;
					perimeter += glm::length(sharedV2 - sharedV1);
				}
				iCurrentEdge = currentEdge.nextEdge;
			}
			while (iCurrentEdge != iInitialEdge);
		}

		// 2. Calculate the area of the surface as the sum of the areas of the
		// triangles
		float area = 0.0f;
		std::vector<std::vector<int>> faceVertexIndices;
		for (int iFace : iFaces) {
			auto faceIndices = getFaceIndices(meshData, iFace);
			area += calculateTriangleArea({
				meshData.vertices[faceIndices[0]].location,
				meshData.vertices[faceIndices[1]].location,
				meshData.vertices[faceIndices[2]].location
			});
		}

		return std::pow(perimeter, 2) / (4 * glm::pi<float>() * area);
	}


	std::pair<bool, glm::vec3> HACD::raycastInsideMesh(
		const HalfEdgeMesh& meshData, const std::map<int, glm::vec3>& faceNormals,
		const glm::vec3& origin, const glm::vec3& direction
	) const
	{
		bool intersects = false;
		glm::vec3 intersection;

		// Search the intersected triangle HEFace
		for (auto itFace = meshData.faces.begin(); (itFace != meshData.faces.end()) && !intersects; ++itFace) {
			int iFace = itFace.getIndex();
			glm::vec3 facePoint = meshData.vertices[ meshData.edges[itFace->edge].vertex ].location;
			glm::vec3 faceNormal = faceNormals.at(iFace);

			// Calculate the intersection of the origin with current HEFace
			// plane along the direction vector
			std::tie(intersects, intersection) = projectPointInDirection(origin, direction, facePoint, faceNormal);

			// Check if the intersected point is inside the polygon of the
			// HEFace
			if (intersects) {
				int iInitialEdge = itFace->edge;
				int iCurrentEdge = iInitialEdge;
				do {
					const HEEdge& currentEdge = meshData.edges[iCurrentEdge];
					const HEEdge& oppositeEdge = meshData.edges[currentEdge.oppositeEdge];

					glm::vec3 p1 = meshData.vertices[oppositeEdge.vertex].location;
					glm::vec3 p2 = meshData.vertices[currentEdge.vertex].location;
					if (glm::dot(glm::cross(p2 - p1, faceNormal), intersection) > 0) {
						intersects = false;
					}
				}
				while ((iCurrentEdge != iInitialEdge) && intersects);
			}
		}

		return std::make_pair(intersects, intersection);
	}


	float HACD::calculateNormalizationFactor(const AABB& aabb) const
	{
		return glm::length(aabb.maximum - aabb.minimum);
	}


	float HACD::calculateAspectRatioFactor(float normalizationFactor) const
	{
		return mMaximumConcavity / (10.0f * normalizationFactor);
	}


	float HACD::calculateDecimationCost(float concavity, float aspectRatio) const
	{
		return concavity / mNormalizationFactor + mAspectRatioFactor * aspectRatio;
	}


	void HACD::updateAncestors(
		DualGraphVertex& vertex1, const DualGraphVertex& vertex2
	) const
	{
		std::vector<int> joinedAncestors;
		std::set_union(
			vertex1.data.begin(), vertex1.data.end(),
			vertex2.data.begin(), vertex2.data.end(),
			std::back_inserter(joinedAncestors)
		);

		vertex1.data = joinedAncestors;
		vertex1.data.insert(
			std::lower_bound(vertex1.data.begin(), vertex1.data.end(), vertex2.id),
			vertex2.id
		);
	}


	void HACD::computeConvexSurfaces()
	{
		mConvexSurfaces.resize(mDualGraph.vertices.size());
		int iCurrentSurface = 0;
		for (auto graphVertex : mDualGraph.vertices) {
			std::vector<int> iFaces = { graphVertex.id };
			iFaces.insert(iFaces.end(), graphVertex.data.begin(), graphVertex.data.end());

			std::map<int, int> vertexIndexMap;
			for (int iFace : iFaces) {
				// Add the HEVertices to the surface if they aren't already in
				// it
				std::vector<int> surfaceFaceIndices;
				for (int iMeshVertex : getFaceIndices(mMesh, iFace)) {
					int iSurfaceVertex = -1;

					auto itVertexIndex = vertexIndexMap.find(iMeshVertex);
					if (itVertexIndex != vertexIndexMap.end()) {
						iSurfaceVertex = itVertexIndex->second;
					}
					else {
						glm::vec3 vertexLocation = mMesh.vertices[iMeshVertex].location;
						iSurfaceVertex = addVertex(mConvexSurfaces[iCurrentSurface], vertexLocation);
						vertexIndexMap.emplace(iMeshVertex, iSurfaceVertex);
					}

					surfaceFaceIndices.push_back(iSurfaceVertex);
				}

				// Add the HEFace to the surface
				addFace(mConvexSurfaces[iCurrentSurface], surfaceFaceIndices);
			}

			++iCurrentSurface;
		}
	}

}}
