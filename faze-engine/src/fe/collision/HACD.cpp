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
			// 1. Calculate the pair of vertices with the lowest decimation cost
			int iBestVertex1 = -1, iBestVertex2 = -1;
			float lowestCost = std::numeric_limits<float>::max();
			for (const GraphVertex& vertex : mDualGraph.vertices) {
				int iVertex1 = vertex.id;
				for (int iVertex2 : vertex.neighbours) {
					// We filter the neighbour vertices already evaluated
					if (iVertex2 > iVertex1) {
						// Calculate the surface created from the current
						// vertices and their ancestors
						auto surfaceFaceIndices = calculateSurfaceFaceIndices(iVertex1, iVertex2);

						// Calculate the cost of the surface
						float concavity = calculateConcavity(surfaceFaceIndices, mMesh, mFaceNormals);
						float aspectRatio = calculateAspectRatio(surfaceFaceIndices, mMesh);
						float currentCost = calculateDecimationCost(concavity, aspectRatio);

						if ((concavity < mMaximumConcavity * mNormalizationFactor) && (currentCost < lowestCost)) {
							lowestCost = currentCost;
							iBestVertex1 = iVertex1;
							iBestVertex2 = iVertex2;
						}
					}
				}
			}

			if ((iBestVertex1 >= 0) && (iBestVertex2 >= 0)) {
				// 2. Merge the best nodes
				halfEdgeCollapse(iBestVertex1, iBestVertex2, mDualGraph);

				// 3. Update the ancestors of the first vertex with the second one
				// TODO: compute the convex hull of the surface partitions here
				updateAncestors(iBestVertex1, iBestVertex2);

				// 4. Update the partitions of the Dual Graph vertices
				calculatePartitions();
			}
			else {
				end = true;
			}
		}

		computeConvexSurfaces();
	}


	void HACD::resetData()
	{
		mFaceNormals.clear();
		mVertexAncestors.clear();
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

		// 4. Calculate the AABB of the mesh TODO: move
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


	Graph HACD::createDualGraph(const HalfEdgeMesh& meshData) const
	{
		Graph dualGraph;

		// Create the dual graph from the Mesh HEFaces
		for (auto itFace = meshData.faces.begin(); itFace != meshData.faces.end(); ++itFace) {
			int iVertex = dualGraph.vertices.create();
			dualGraph.vertices[iVertex].id = itFace.getIndex();
		}

		// Create the vertices neighbours from the Mesh adjacent HEFaces
		for (GraphVertex& v : dualGraph.vertices) {
			int iInitialEdge = meshData.faces[v.id].edge;
			int iCurrentEdge = iInitialEdge;
			do {
				HEEdge currentEdge = meshData.edges[iCurrentEdge];
				HEEdge oppositeEdge = meshData.edges[currentEdge.oppositeEdge];

				int iOtherVertex = oppositeEdge.face;
				auto itOtherVertex = std::lower_bound(dualGraph.vertices.begin(), dualGraph.vertices.end(), iOtherVertex, lessVertexId);

				// Check if we already set the other vertex as a neighbour of
				// the current one
				if (!std::binary_search(v.neighbours.begin(), v.neighbours.end(), iOtherVertex)) {
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


	std::vector<int> HACD::calculateSurfaceFaceIndices(int iVertex1, int iVertex2) const
	{
		std::vector<int> surfaceFaceIndices = { iVertex1, iVertex2 };

		auto itVertex1Ancestors = mVertexAncestors.equal_range(iVertex1);
		std::transform(
			itVertex1Ancestors.first, itVertex1Ancestors.second, std::back_inserter(surfaceFaceIndices),
			[](const std::pair<int, int>& p) { return p.second; }
		);

		auto itVertex2Ancestors = mVertexAncestors.equal_range(iVertex2);
		std::transform(
			itVertex2Ancestors.first, itVertex2Ancestors.second, std::back_inserter(surfaceFaceIndices),
			[](const std::pair<int, int>& p) { return p.second; }
		);

		return surfaceFaceIndices;
	}


	float HACD::calculateConcavity(
		const std::vector<int>& iFaces,
		const HalfEdgeMesh& meshData, const std::map<int, glm::vec3>& faceNormals
	) const
	{
		// 1. Create a new surface HEMesh from the given meshData's HEFaces
		HalfEdgeMesh surface;
		std::map<int, int> vertexMap, faceMap;

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

			int iFace2 = addFace(surface, iFace2Vertices);
			faceMap.emplace(iFace1, iFace2);
		}

		// 2. Calculate the convex hull of the surface HEMesh
		QuickHull qh(mQuickHullEpsilon);
		qh.calculate(surface);
		const HalfEdgeMesh& surfaceConvexHull = qh.getMesh();

		// 3. Calculate the maximum concavity as the distance of a point to its
		// projection on the convex hull of its mesh
		float maxConcavity = -std::numeric_limits<float>::max();
		for (auto iVertexPair : vertexMap) {
			glm::vec3 vertexLocation = meshData.vertices[iVertexPair.first].location;
			glm::vec3 vertexNormal = calculateVertexNormal(meshData, faceNormals, iVertexPair.first);

			glm::vec3 raycastedPoint = raycastInsideMesh(surfaceConvexHull, vertexLocation, vertexNormal);
			float currentConcavity = glm::length(raycastedPoint - vertexLocation);
			maxConcavity = std::max(maxConcavity, currentConcavity);
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


	glm::vec3 HACD::calculateVertexNormal(
		const HalfEdgeMesh& meshData,
		const std::map<int, glm::vec3>& faceNormals,
		int iVertex
	) const
	{
		glm::vec3 vertexNormal(0.0f);

		int iInitialEdge = meshData.vertices[iVertex].edge;
		int iCurrentEdge = iInitialEdge;
		do {
			const HEEdge& currentEdge = meshData.edges[iCurrentEdge];
			vertexNormal += faceNormals.find(currentEdge.face)->second;

			iCurrentEdge = currentEdge.nextEdge;
		}
		while (iCurrentEdge != iInitialEdge);

		return glm::normalize(vertexNormal);
	}


	glm::vec3 HACD::raycastInsideMesh(
		const HalfEdgeMesh& meshData,
		const glm::vec3& origin, const glm::vec3& direction
	) const
	{
		// Find the intersected HEFace
		int iFace = meshData.faces.begin().getIndex();
		bool directionInside = false;
		while (!directionInside) {
			// Check if the ray points towards the current HEFace from the
			// origin point
			int iInitialEdge = meshData.faces[iFace].edge;
			int iCurrentEdge = iInitialEdge;
			do {
				const HEEdge& currentEdge = meshData.edges[iCurrentEdge];
				const HEEdge& oppositeEdge = meshData.edges[currentEdge.oppositeEdge];

				glm::vec3 v1 = meshData.vertices[ oppositeEdge.vertex ].location;
				glm::vec3 v2 = meshData.vertices[ currentEdge.vertex ].location;
				glm::vec3 normal = glm::cross(origin - v1, origin - v2);
				if (dot(normal, direction) > 0) {
					iFace = oppositeEdge.face;
					directionInside = false;
					iCurrentEdge = iInitialEdge;
				}
				else {
					directionInside = true;
					iCurrentEdge = currentEdge.nextEdge;
				}
			}
			while (iCurrentEdge != iInitialEdge);
		}

		// Project the point on the HEFace
		std::vector<int> faceIndices = getFaceIndices(meshData, iFace);
		glm::vec3 p0 = meshData.vertices[faceIndices[0]].location;
		glm::vec3 p1 = meshData.vertices[faceIndices[1]].location;
		glm::vec3 p2 = meshData.vertices[faceIndices[2]].location;
		glm::vec3 faceNormal = glm::normalize(glm::cross(p1 - p0, p2 - p0));

		float length = glm::dot((p0 - origin), faceNormal) / glm::dot(direction, faceNormal);
		return origin + length * direction;
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


	void HACD::updateAncestors(int iVertex1, int iVertex2)
	{
		auto itVertex2Ancestors = mVertexAncestors.equal_range(iVertex2);
		for (auto itAncestor = itVertex2Ancestors.first; itAncestor != itVertex2Ancestors.second; ++itAncestor) {
			mVertexAncestors.emplace(iVertex1, itAncestor->second);
		}

		mVertexAncestors.emplace(iVertex1, iVertex2);
	}


	void HACD::calculatePartitions()
	{
		mGraphPartitions.clear();
		for (auto itVertex = mDualGraph.vertices.begin(); itVertex != mDualGraph.vertices.end(); ++itVertex) {
			int iVertex = itVertex.getIndex();
			auto itVertexAncestors = mVertexAncestors.equal_range(iVertex);

			GraphPartition partition;
			partition.push_back(iVertex);
			for (auto itAncestor = itVertexAncestors.first; itAncestor != itVertexAncestors.second; ++itAncestor) {
				partition.push_back(itAncestor->second);
			}

			mGraphPartitions.push_back(partition);
		}
	}


	void HACD::computeConvexSurfaces()
	{
		mConvexSurfaces.resize(mGraphPartitions.size());
		for (std::size_t iPartition = 0; iPartition < mGraphPartitions.size(); ++iPartition) {
			std::map<int, int> vertexIndexMap;
			for (int iMeshFace : mGraphPartitions[iPartition]) {
				// Add the HEVertices to the surface if they aren't already in
				// it
				std::vector<int> surfaceFaceIndices;
				for (int iMeshVertex : getFaceIndices(mMesh, iMeshFace)) {
					int iSurfaceVertex = -1;

					auto itVertexIndex = vertexIndexMap.find(iMeshVertex);
					if (itVertexIndex != vertexIndexMap.end()) {
						iSurfaceVertex = itVertexIndex->second;
					}
					else {
						glm::vec3 vertexLocation = mMesh.vertices[iMeshVertex].location;
						iSurfaceVertex = addVertex(mConvexSurfaces[iPartition], vertexLocation);
						vertexIndexMap.emplace(iMeshVertex, iSurfaceVertex);
					}

					surfaceFaceIndices.push_back(iSurfaceVertex);
				}

				// Add the HEFace to the surface
				addFace(mConvexSurfaces[iPartition], surfaceFaceIndices);
			}
		}
	}

}}
