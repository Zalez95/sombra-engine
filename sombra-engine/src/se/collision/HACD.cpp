#include <limits>
#include <algorithm>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtc/constants.hpp>
#include "se/collision/HACD.h"
#include "se/collision/QuickHull.h"
#include "se/collision/HalfEdgeMeshExt.h"
#include "se/utils/FixedVector.h"
#include "Geometry.h"

namespace se::collision {

	void HACD::calculate(const HalfEdgeMesh& originalMesh)
	{
		initData(originalMesh);

		// Create a Queue of Graph Edges to collapse ordered by its cost
		// from highest to lowest
		std::vector<QHACDData> vertexPairsByCost;
		for (const DualGraphVertex& vertex1 : mDualGraph.vertices) {
			for (int iVertex2 : vertex1.neighbours) {
				// Filter bad vertices
				auto itVertex2 = std::lower_bound(mDualGraph.vertices.begin(), mDualGraph.vertices.end(), iVertex2);
				if (itVertex2 == mDualGraph.vertices.end()) { continue; }

				// Filter the neighbour vertices already evaluated
				const DualGraphVertex& vertex2 = *itVertex2;
				if (vertex2.id <= vertex1.id) { continue; }

				QHACDData curData = createQHACDData(vertex1, vertex2);
				vertexPairsByCost.insert(
					std::lower_bound(vertexPairsByCost.begin(), vertexPairsByCost.end(), curData, std::greater<QHACDData>()),
					curData
				);
			}
		}

		// Collapse the Graph Edge with the lowest cost until there's no more
		while (!vertexPairsByCost.empty()) {
			QHACDData curData = vertexPairsByCost.back();
			vertexPairsByCost.pop_back();

			// Filter the Graph Edges marked as removed or with a concavity
			// measure larger than the maximum concavity
			if (curData.remove) { continue; }
			if (curData.concavity >= mMaximumConcavity * mNormalizationFactor) { continue; }

			// 1. Update the ancestors of the first vertex with the second one's
			// ancestors
			auto itVertex1 = std::lower_bound(mDualGraph.vertices.begin(), mDualGraph.vertices.end(), curData.iVertex1);
			auto itVertex2 = std::lower_bound(mDualGraph.vertices.begin(), mDualGraph.vertices.end(), curData.iVertex2);
			updateAncestors(*itVertex1, *itVertex2);

			// 2. Merge both nodes into the first one
			halfEdgeCollapse(itVertex1->id, itVertex2->id, mDualGraph);

			// 3. Remove all the elements of the Queue that holds the Vertex 1
			// or 2
			for (QHACDData& otherData : vertexPairsByCost) {
				if (!otherData.remove) {
					otherData.remove = compareVertexIds(curData, otherData);
				}
			}

			// 4. Add new Graph Edges to collapse to the Queue with the updated
			// vertex 1 data
			itVertex1 = std::lower_bound(mDualGraph.vertices.begin(), mDualGraph.vertices.end(), curData.iVertex1);
			for (int iVertex2 : itVertex1->neighbours) {
				itVertex2 = std::lower_bound(mDualGraph.vertices.begin(), mDualGraph.vertices.end(), iVertex2);
				if (itVertex2 != mDualGraph.vertices.end()) {
					curData = createQHACDData(*itVertex1, *itVertex2);
					vertexPairsByCost.insert(
						std::lower_bound(vertexPairsByCost.begin(), vertexPairsByCost.end(), curData, std::greater<QHACDData>()),
						curData
					);
				}
			}
		}

		computeConvexSurfaces();
	}


	void HACD::resetData()
	{
		mFaceNormals.clear();
		mConvexMeshes.clear();
	}

// Private functions
	void HACD::initData(const HalfEdgeMesh& originalMesh)
	{
		// 1. Calculate the triangulated mesh
		mMesh = triangulateFaces(originalMesh);

		// 2. Calculate the face normals of the mesh
		mFaceNormals.replicate(mMesh.faces);
		for (auto it = mMesh.faces.begin(); it != mMesh.faces.end(); ++it) {
			mFaceNormals[it.getIndex()] = calculateFaceNormal(mMesh, it.getIndex());
		}

		// 3. Calculate the initial dual graph of the triangulated mesh
		mDualGraph = createDualGraph(mMesh);

		// 4. Calculate the AABB of the mesh
		AABB meshAABB = calculateAABB(mMesh);

		// 5. Calculate the normalization factor of the triangulated mesh
		mNormalizationFactor = calculateNormalizationFactor(meshAABB);

		// 6. Calculate the scaled epsilon value
		mScaledEpsilon = mNormalizationFactor * mEpsilon;

		// 7. Calculate the aspect ratio factor of the triangulated mesh
		mAspectRatioFactor = calculateAspectRatioFactor(mMaximumConcavity, mNormalizationFactor);
	}


	HACD::QHACDData HACD::createQHACDData(
		const DualGraphVertex& vertex1, const DualGraphVertex& vertex2
	) const
	{
		// Calculate the surface created from the current vertices and
		// their ancestors
		auto surfaceFaceIndices = calculateSurfaceFaceIndices(vertex1, vertex2);
		auto [surface, surfaceNormals] = getMeshFromIndices(surfaceFaceIndices, mMesh, mFaceNormals);

		// Calculate the cost of the surface
		QuickHull qh(mEpsilon);
		qh.calculate(surface);
		float concavity = calculateConcavity(surface, surfaceNormals, qh.getMesh(), qh.getNormals());
		float aspectRatio = calculateAspectRatio(surface);
		float cost = calculateDecimationCost(concavity, aspectRatio);

		return { vertex1.id, vertex2.id, cost, concavity, false };
	}


	void HACD::updateAncestors(
		DualGraphVertex& vertex1, const DualGraphVertex& vertex2
	) {
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


	bool HACD::compareVertexIds(const QHACDData& qd1, const QHACDData& qd2)
	{
		return qd1.iVertex1 == qd2.iVertex1 || qd1.iVertex1 == qd2.iVertex2
			|| qd1.iVertex2 == qd2.iVertex1 || qd1.iVertex2 == qd2.iVertex2;
	}


	void HACD::computeConvexSurfaces()
	{
		QuickHull qh(mEpsilon);

		mConvexMeshes.reserve(mDualGraph.vertices.size());
		for (const auto& graphVertex : mDualGraph.vertices) {
			// Create a surface from the current vertex and its ancestors
			std::vector<int> iFaces = { graphVertex.id };
			iFaces.insert(iFaces.end(), graphVertex.data.begin(), graphVertex.data.end());
			HalfEdgeMesh surface = getMeshFromIndices(iFaces, mMesh, mFaceNormals).first;

			// Push the convex hull of the surface to the convex surfaces vector
			qh.resetData();
			qh.calculate(surface);
			mConvexMeshes.push_back(qh.getMesh());
		}
	}


	HACD::DualGraph HACD::createDualGraph(const HalfEdgeMesh& meshData)
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

				iCurrentEdge = currentEdge.nextEdge;
			}
			while (iCurrentEdge != iInitialEdge);
		}

		return dualGraph;
	}


	float HACD::calculateNormalizationFactor(const AABB& aabb)
	{
		return glm::length(aabb.maximum - aabb.minimum);
	}


	float HACD::calculateAspectRatioFactor(float maximumConcavity, float normalizationFactor)
	{
		return maximumConcavity / (10.0f * normalizationFactor);
	}


	std::vector<int> HACD::calculateSurfaceFaceIndices(
		const DualGraphVertex& vertex1, const DualGraphVertex& vertex2
	) {
		std::vector<int> surfaceFaceIndices = { vertex1.id, vertex2.id };
		surfaceFaceIndices.insert(surfaceFaceIndices.end(), vertex1.data.begin(), vertex1.data.end());
		surfaceFaceIndices.insert(surfaceFaceIndices.end(), vertex2.data.begin(), vertex2.data.end());

		return surfaceFaceIndices;
	}


	std::pair<HalfEdgeMesh, ContiguousVector<glm::vec3>> HACD::getMeshFromIndices(
		const std::vector<int>& iFaces,
		const HalfEdgeMesh& meshData, const ContiguousVector<glm::vec3>& faceNormals
	) {
		HalfEdgeMesh newMesh;
		ContiguousVector<glm::vec3> newMeshNormals;

		std::map<int, int> vertexMap;
		for (int iFace1 : iFaces) {
			std::vector<int> iFace1Vertices, iFace2Vertices;

			getFaceIndices(meshData, iFace1, std::back_inserter(iFace1Vertices));
			for (int iVertex1 : iFace1Vertices) {
				auto itVertex1 = vertexMap.find(iVertex1);
				if (itVertex1 != vertexMap.end()) {
					iFace2Vertices.push_back(itVertex1->second);
				}
				else {
					int iVertex2 = addVertex(newMesh, meshData.vertices[iVertex1].location);
					vertexMap.emplace(iVertex1, iVertex2);
					iFace2Vertices.push_back(iVertex2);
				}
			}

			addFace(newMesh, iFace2Vertices.begin(), iFace2Vertices.end());
			newMeshNormals.emplace(faceNormals[iFace1]);
		}

		return std::make_pair(newMesh, newMeshNormals);
	}


	float HACD::calculateConcavity(
		const HalfEdgeMesh& originalMesh, const ContiguousVector<glm::vec3>& faceNormals,
		const HalfEdgeMesh& convexHullMesh, const ContiguousVector<glm::vec3>& convexHullNormals
	) const
	{
		float concavity = 0.0f;

		// Check if the convex hull is flat
		glm::vec3 polygonNormal = (convexHullNormals.empty())? glm::vec3(0.0f) : *convexHullNormals.begin();
		if (std::all_of(
				convexHullNormals.begin(), convexHullNormals.end(),
				[&](const glm::vec3& normal) { return glm::all(glm::epsilonEqual(normal, polygonNormal, mScaledEpsilon)); }
			)
		) {
			// Add the 3D concavity
			concavity += calculateConcavity3D(originalMesh, faceNormals, convexHullMesh, convexHullNormals);
		}

		// Add the 2D concavity
		float originalArea = calculateArea(originalMesh);
		float convexHullArea = calculateArea(convexHullMesh);
		float convexHullVolume = calculateVolume(convexHullMesh, convexHullNormals);
		float weight2D = std::max(0.0f, 1.0f - std::pow(convexHullVolume / convexHullArea, 2.0f));
		concavity += weight2D * calculateConcavity2D(originalArea, convexHullArea);

		return concavity;
	}


	float HACD::calculateConcavity2D(float originalArea, float convexHullArea)
	{
		float areaDifference = convexHullArea - originalArea;
		return (areaDifference < 0.0f)? 0.0f : std::sqrt(areaDifference);
	}


	float HACD::calculateConcavity3D(
		const HalfEdgeMesh& originalMesh, const ContiguousVector<glm::vec3>& faceNormals,
		const HalfEdgeMesh& convexHullMesh, const ContiguousVector<glm::vec3>& convexHullNormals
	) const
	{
		float maxConcavity = -std::numeric_limits<float>::max();
		for (auto itVertex = originalMesh.vertices.begin(); itVertex != originalMesh.vertices.end(); ++itVertex) {
			glm::vec3 vertexLocation = itVertex->location;
			glm::vec3 vertexNormal = calculateVertexNormal(originalMesh, faceNormals, itVertex.getIndex());

			auto [intersects, intersection] = getInternalIntersection(convexHullMesh, convexHullNormals, vertexLocation, vertexNormal);
			if (intersects) {
				float currentConcavity = glm::length(intersection - vertexLocation);
				maxConcavity = std::max(maxConcavity, currentConcavity);
			}
		}

		return maxConcavity;
	}


	float HACD::calculateAspectRatio(const HalfEdgeMesh& meshData)
	{
		// 1. Calculate the perimeter of the surface of the triangles
		float perimeter = 0.0f;
		for (auto itFace = meshData.faces.begin(); itFace != meshData.faces.end(); ++itFace) {
			int iInitialEdge = itFace->edge;
			int iCurrentEdge = iInitialEdge;
			do {
				const HEEdge& currentEdge = meshData.edges[iCurrentEdge];
				const HEEdge& oppositeEdge = meshData.edges[currentEdge.oppositeEdge];
				if (!meshData.faces.isActive(oppositeEdge.face)) {
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
		float area = calculateArea(meshData);

		return std::pow(perimeter, 2) / (4 * glm::pi<float>() * area);
	}


	float HACD::calculateDecimationCost(float concavity, float aspectRatio) const
	{
		return concavity / mNormalizationFactor + mAspectRatioFactor * aspectRatio;
	}


	std::pair<bool, glm::vec3> HACD::getInternalIntersection(
		const HalfEdgeMesh& meshData, const ContiguousVector<glm::vec3>& faceNormals,
		const glm::vec3& origin, const glm::vec3& direction
	) const
	{
		// Search the first intersected HEFace
		for (auto itFace = meshData.faces.begin(); itFace != meshData.faces.end(); ++itFace) {
			glm::vec3 facePoint = meshData.vertices[ meshData.edges[itFace->edge].vertex ].location;
			glm::vec3 faceNormal = faceNormals[itFace.getIndex()];

			// Discard faces with normals pointing in the opposite direction
			if (glm::dot(faceNormal, direction) < -mScaledEpsilon) { continue; }

			auto [intersects, intersection] = rayPlaneIntersection(origin, direction, facePoint, faceNormal, mScaledEpsilon);
			if (intersects && isPointBetweenHEEdges(meshData, itFace->edge, faceNormal, intersection)) {
				return std::make_pair(intersects, intersection);
			}
		}

		return std::make_pair(false, glm::vec3(0.0f));
	}


	bool HACD::isPointBetweenHEEdges(
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
			if (glm::dot(glm::cross(p2 - p1, loopNormal), point - p1) > mScaledEpsilon) {
				inside = false;
			}

			iCurrentEdge = currentEdge.nextEdge;
		}
		while ((iCurrentEdge != iInitialEdge) && inside);

		return inside;
	}

}
