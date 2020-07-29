#include <array>
#include <stack>
#include <limits>
#include <algorithm>
#include <glm/gtc/epsilon.hpp>
#include "se/utils/FixedVector.h"
#include "se/collision/AABB.h"
#include "se/collision/QuickHull.h"
#include "se/collision/HalfEdgeMeshExt.h"

namespace se::collision {

	/** Struct QH2DStackContent, its the data structure used in the QuickHull 2D
	 * algorithm for storing its state in a stack instead of using a
	 * recursive algorithm */
	struct QH2DStackContent
	{
		/** The state of the current iteration */
		enum { Search, Add, End } state;

		/** The indices of the vertices of the current edge */
		int iVertex1, iVertex2;

		/** The indices of the outside vertices of the current interation */
		std::vector<int> outsideVertices;
	};


	void QuickHull::calculate(const HalfEdgeMesh& originalMesh)
	{
		// Calculate the scaled epsilon value
		AABB meshAABB = calculateAABB(originalMesh);
		mScaledEpsilon = glm::length(meshAABB.maximum - meshAABB.minimum) * mEpsilon;

		// Calculate an initial simplex from the originalMesh
		std::vector<int> iSimplexVertices = calculateInitialSimplex(originalMesh);

		// Calculate the convex hull
		if (iSimplexVertices.size() < 3) {
			mConvexHullMesh = originalMesh;
		}
		else if (iSimplexVertices.size() < 4) {
			calculateQuickHull2D(originalMesh, iSimplexVertices);
		}
		else {
			calculateQuickHull3D(originalMesh, iSimplexVertices);
		}

		// Remove the vertices without HEEdge that has been left alone in the
		// merge HEFace steps
		for (auto itVertex = mConvexHullMesh.vertices.begin(); itVertex != mConvexHullMesh.vertices.end(); ++itVertex) {
			if (itVertex->edge < 0) {
				removeVertex(mConvexHullMesh, itVertex.getIndex());
			}
		}
	}


	void QuickHull::resetData()
	{
		mConvexHullMesh = HalfEdgeMesh();
		mConvexHullNormals = utils::PackedVector<glm::vec3>();
		mFaceOutsideVertices = utils::PackedVector<std::vector<int>>();
		mVertexIndexMap = std::unordered_map<int, int>();
	}

// Private functions
	std::vector<int> QuickHull::calculateInitialSimplex(const HalfEdgeMesh& meshData) const
	{
		std::vector<int> iSimplexVertices(4, -1);

		// 1. Find the extreme vertices in each axis
		std::vector<int> extremePointIndices(6, -1);
		for (auto it = meshData.vertices.begin(); it != meshData.vertices.end(); ++it) {
			for (int iAxis = 0; iAxis < 3; ++iAxis) {
				int& iAxisMin = extremePointIndices[2*iAxis];
				if ((iAxisMin < 0) || (it->location[iAxis] < meshData.vertices[iAxisMin].location[iAxis])) {
					iAxisMin = it.getIndex();
				}

				int& iAxisMax = extremePointIndices[2*iAxis + 1];
				if ((iAxisMax < 0) || (it->location[iAxis] > meshData.vertices[iAxisMax].location[iAxis])) {
					iAxisMax = it.getIndex();
				}
			}
		}

		// 2. Find from the extreme vertices the pair which are furthest apart
		float maxLength = -std::numeric_limits<float>::max();
		for (int i = 0; i < 6; ++i) {
			if (extremePointIndices[i] < 0) { continue; }

			for (int j = i + 1; j < 6; ++j) {
				if (extremePointIndices[j] < 0) { continue; }

				glm::vec3 p1p2	= meshData.vertices[extremePointIndices[j]].location
								- meshData.vertices[extremePointIndices[i]].location;

				float currentLength = glm::dot(p1p2, p1p2);
				if (currentLength > maxLength) {
					iSimplexVertices[0] = extremePointIndices[i];
					iSimplexVertices[1] = extremePointIndices[j];
					maxLength = currentLength;
				}
			}
		}

		if (maxLength <= 0.0f) {
			// The point 1 is also the 0
			iSimplexVertices[1] = -1;
		}
		else {
			// 3. Find the furthest point to the edge between the last 2 vertices
			glm::vec3 p0 = meshData.vertices[iSimplexVertices[0]].location;
			glm::vec3 p1 = meshData.vertices[iSimplexVertices[1]].location;
			glm::vec3 dirP0P1 = glm::normalize(p1 - p0);
			maxLength = -std::numeric_limits<float>::max();
			for (auto it = meshData.vertices.begin(); it != meshData.vertices.end(); ++it) {
				glm::vec3 projection = p0 + dirP0P1 * glm::dot(it->location - p0, dirP0P1);

				glm::vec3 p2e = it->location - projection;
				float currentLength = glm::dot(p2e, p2e);
				if (currentLength > maxLength) {
					iSimplexVertices[2] = it.getIndex();
					maxLength = currentLength;
				}
			}

			if (maxLength <= 0.0f) {
				// The point 2 is on the same line as 1 and 0
				iSimplexVertices[2] = -1;
			}
			else {
				// 4. Find the furthest point to the triangle created from the last 3
				// vertices
				glm::vec3 p2 = meshData.vertices[iSimplexVertices[2]].location;
				glm::vec3 dirP0P2 = glm::normalize(p2 - p0);
				glm::vec3 tNormal = glm::normalize(glm::cross(dirP0P1, dirP0P2));
				maxLength = -std::numeric_limits<float>::max();
				for (auto it = meshData.vertices.begin(); it != meshData.vertices.end(); ++it) {
					float currentLength = std::abs( glm::dot(it->location - p0, tNormal) );
					if (currentLength > maxLength) {
						iSimplexVertices[3] = it.getIndex();
						maxLength = currentLength;
					}
				}

				if (maxLength <= 0.0f) {
					// The point 3 is on the same plane as 0, 1 and 2
					iSimplexVertices[3] = -1;
				}
			}
		}

		// Remove the invalid simplex vertices
		iSimplexVertices.erase(
			std::remove_if(iSimplexVertices.begin(), iSimplexVertices.end(), [](int i) { return i < 0; }),
			iSimplexVertices.end()
		);

		return iSimplexVertices;
	}


	void QuickHull::calculateQuickHull2D(const HalfEdgeMesh& originalMesh, const std::vector<int>& iSimplexVertices)
	{
		assert(originalMesh.vertices.size() >= 3 && "The mesh must have at least 3 vertices");

		glm::vec3 p0 = originalMesh.vertices[0].location, p1 = originalMesh.vertices[1].location, p2 = originalMesh.vertices[2].location;
		glm::vec3 dirP0P1 = p1 - p0, dirP0P2 = p2 - p1, planeNormal = glm::cross(dirP0P1, dirP0P2);

		// Calculate the convex hull vertices with the iterative quickhull 2D algorithm
		std::stack<QH2DStackContent> stack;

		std::vector<int> allVertexIndices, faceIndices;
		for (auto itVertex = originalMesh.vertices.begin(); itVertex != originalMesh.vertices.end(); ++itVertex) {
			allVertexIndices.push_back(itVertex.getIndex());
		}

		auto halfVertexIndices2 = filterOutsideVertices(originalMesh.vertices, allVertexIndices, planeNormal, iSimplexVertices[1], iSimplexVertices[0]);
		stack.push({ QH2DStackContent::Add, iSimplexVertices[1], iSimplexVertices[0], halfVertexIndices2 });

		auto halfVertexIndices1 = filterOutsideVertices(originalMesh.vertices, allVertexIndices, planeNormal, iSimplexVertices[0], iSimplexVertices[1]);
		stack.push({ QH2DStackContent::Add, iSimplexVertices[0], iSimplexVertices[1], halfVertexIndices1 });

		while (!stack.empty()) {
			auto& [state, iVertex1, iVertex2, outsideVertices] = stack.top();
			switch (state) {
				case QH2DStackContent::Search:
					if (!outsideVertices.empty()) {
						// 1. Get the furthest vertex from the edge between the
						// vertex 1 and 2
						int iFurthestVertex = getFurthestVertexFromEdge(
							originalMesh.vertices, outsideVertices,
							iVertex1, iVertex2
						);

						// 4. Search the next convex hull vertex in the set of
						// vertices outside the edge furthest-vertex2
						auto outsideVertices2 = filterOutsideVertices(originalMesh.vertices, outsideVertices, planeNormal, iFurthestVertex, iVertex2);
						stack.push({ QH2DStackContent::Add, iFurthestVertex, iVertex2, outsideVertices2 });

						// 2. Search the next convex hull vertex in the set of
						// vertices outside the edge vertex1-furthest
						auto outsideVertices1 = filterOutsideVertices(originalMesh.vertices, outsideVertices, planeNormal, iVertex1, iFurthestVertex);
						stack.push({ QH2DStackContent::Search, iVertex1, iFurthestVertex, outsideVertices1 });
					}
					state = QH2DStackContent::End;
					break;
				case QH2DStackContent::Add:
					// 3. Add the furthest vertex (iV1) to the convex hull
					faceIndices.push_back( addVertex(mConvexHullMesh, originalMesh.vertices[iVertex1].location) );
					state = QH2DStackContent::Search;	// Continue searching
					break;
				case QH2DStackContent::End:
					stack.pop();
					break;
			}
		}

		// Add the convex hull face
		int iNewFace = addFace(mConvexHullMesh, faceIndices.begin(), faceIndices.end());
		mConvexHullNormals.emplace( calculateFaceNormal(mConvexHullMesh, iNewFace) );
	}


	int QuickHull::getFurthestVertexFromEdge(
		const utils::PackedVector<HEVertex>& vertices,
		const std::vector<int>& vertexIndices, int iVertex1, int iVertex2
	) const
	{
		glm::vec3 p1 = vertices[iVertex1].location, p2 = vertices[iVertex2].location;
		glm::vec3 dirP1P2 = glm::normalize(p2 - p1);
		return *std::max_element(
			vertexIndices.begin(), vertexIndices.end(),
			[&](int iV1, int iV2) {
				glm::vec3 proj1 = p1 + dirP1P2 * glm::dot(vertices[iV1].location - p1, dirP1P2);
				glm::vec3 proj2 = p1 + dirP1P2 * glm::dot(vertices[iV2].location - p1, dirP1P2);

				glm::vec3 vDist1 = vertices[iV1].location - proj1;
				glm::vec3 vDist2 = vertices[iV2].location - proj2;
				return glm::dot(vDist1, vDist1) < glm::dot(vDist2, vDist2);
			}
		);
	}


	std::vector<int> QuickHull::filterOutsideVertices(
		const utils::PackedVector<HEVertex>& vertices, const std::vector<int>& vertexIndices,
		const glm::vec3& planeNormal, int iVertex1, int iVertex2
	) const
	{
		std::vector<int> filteredVertices;

		glm::vec3 p1 = vertices[iVertex1].location, p2 = vertices[iVertex2].location;
		glm::vec3 dirP1P2 = p2 - p1, dirOutside = glm::cross(dirP1P2, planeNormal);

		std::copy_if(
			vertexIndices.begin(), vertexIndices.end(), std::back_inserter(filteredVertices),
			[&](int iVertex) { return glm::dot(vertices[iVertex].location - p1, dirOutside) > mScaledEpsilon; }
		);

		return filteredVertices;
	}


	void QuickHull::calculateQuickHull3D(const HalfEdgeMesh& originalMesh, const std::vector<int>& iSimplexVertices)
	{
		createInitial3DConvexHull(originalMesh, iSimplexVertices);

		// 1. Find a convex hull face with a non empty outside vertices vector
		auto itFace = mFaceOutsideVertices.begin();
		while ((itFace = std::find_if(
					mFaceOutsideVertices.begin(), mFaceOutsideVertices.end(),
					[](const std::vector<int>& outsideVertices) { return !outsideVertices.empty(); }
				)
			) != mFaceOutsideVertices.end()
		) {
			// 2. Get the furthest HEVertex in the direction of the face normal
			int iEyeVertex = getFurthestVertexInDirection(*itFace, originalMesh, mConvexHullNormals[itFace.getIndex()]);
			glm::vec3 eyePoint = originalMesh.vertices[iEyeVertex].location;

			// 3. Check if the eyePoint is already inside in the convex hull
			auto itVertex = mVertexIndexMap.find(iEyeVertex);
			if (itVertex == mVertexIndexMap.end()) {
				// 3.1. Calculate the convex hull's horizon HEEdges and HEFaces
				// to remove from the current eyePoint perspective
				auto [horizon, facesToRemove] = calculateHorizon(mConvexHullMesh, mConvexHullNormals, eyePoint, itFace.getIndex());

				// 3.2. Remove the convex hull's HEFaces seen from the current
				// eyePoint perspective and collect all their outside HEVertices
				std::vector<int> allOutsideVertices;
				for (int iFaceToRemove : facesToRemove) {
					std::vector<int> joinedOutsideVertices;
					std::set_union(
						allOutsideVertices.begin(), allOutsideVertices.end(),
						mFaceOutsideVertices[iFaceToRemove].begin(), mFaceOutsideVertices[iFaceToRemove].end(),
						std::back_inserter(joinedOutsideVertices)
					);
					allOutsideVertices = joinedOutsideVertices;

					removeFace(mConvexHullMesh, iFaceToRemove);
					mConvexHullNormals.erase( mConvexHullNormals.begin().setIndex(iFaceToRemove) );
					mFaceOutsideVertices.erase( mFaceOutsideVertices.begin().setIndex(iFaceToRemove) );
				}

				// 3.3. Add the eyePoint to the convex hull
				int iEyeVertexConvexHull = addVertex(mConvexHullMesh, eyePoint);
				mVertexIndexMap.emplace(iEyeVertex, iEyeVertexConvexHull);

				// 3.4. Create new HEFaces by joining the edges of the horizon
				// with the convex hull eyePoint
				for (int iHorizonEdge : horizon) {
					const HEEdge& currentEdge	= mConvexHullMesh.edges[iHorizonEdge];
					const HEEdge& oppositeEdge	= mConvexHullMesh.edges[currentEdge.oppositeEdge];

					// Create the new HEFace
					std::array<int, 3> vertexIndices = { oppositeEdge.vertex, currentEdge.vertex, iEyeVertexConvexHull };
					int iNewFace = addFace(mConvexHullMesh, vertexIndices.begin(), vertexIndices.end());
					mConvexHullNormals.emplace( calculateFaceNormal(mConvexHullMesh, iNewFace) );
					mFaceOutsideVertices.emplace( getVerticesOutside(allOutsideVertices, originalMesh, iNewFace) );

					// Merge the coplanar faces
					mergeCoplanarFaces(iNewFace);
				}
			}
		}
	}


	void QuickHull::createInitial3DConvexHull(const HalfEdgeMesh& originalMesh, const std::vector<int>& iSimplexVertices)
	{
		// Get all the vertex indices from the original mesh sorted ascendently
		std::vector<int> meshVertexIndices;
		for (auto it = originalMesh.vertices.begin(); it != originalMesh.vertices.end(); ++it) {
			meshVertexIndices.push_back(it.getIndex());
		}
		std::sort(meshVertexIndices.begin(), meshVertexIndices.end());

		// Add the vertices to the convex hull
		utils::FixedVector<int, 4> chVertexIndices;
		for (int iMeshVertex : iSimplexVertices) {
			int iConvexHullVertex = addVertex(mConvexHullMesh, originalMesh.vertices[iMeshVertex].location);
			mVertexIndexMap.emplace(iMeshVertex, iConvexHullVertex);
			chVertexIndices.push_back(iConvexHullVertex);
		}

		// Add the HEFaces to the convex hull
		const glm::vec3 p0 = mConvexHullMesh.vertices[chVertexIndices[0]].location,
						p1 = mConvexHullMesh.vertices[chVertexIndices[1]].location,
						p2 = mConvexHullMesh.vertices[chVertexIndices[2]].location,
						p3 = mConvexHullMesh.vertices[chVertexIndices[3]].location;
		const glm::vec3 tNormal = glm::cross(p1 - p0, p2 - p0);

		std::array<std::array<int , 3>, 4> faceIndices;
		if (glm::dot(p3 - p0, tNormal) <= 0.0f) {
			faceIndices = {{
				{{ chVertexIndices[0], chVertexIndices[1], chVertexIndices[2] }},
				{{ chVertexIndices[0], chVertexIndices[3], chVertexIndices[1] }},
				{{ chVertexIndices[0], chVertexIndices[2], chVertexIndices[3] }},
				{{ chVertexIndices[1], chVertexIndices[3], chVertexIndices[2] }}
			}};
		}
		else {
			faceIndices = {{
				{{ chVertexIndices[0], chVertexIndices[2], chVertexIndices[1] }},
				{{ chVertexIndices[0], chVertexIndices[1], chVertexIndices[3] }},
				{{ chVertexIndices[0], chVertexIndices[3], chVertexIndices[2] }},
				{{ chVertexIndices[1], chVertexIndices[2], chVertexIndices[3] }}
			}};
		}

		for (const auto& face : faceIndices) {
			int iFace = addFace(mConvexHullMesh, face.begin(), face.end());
			mConvexHullNormals.emplace( calculateFaceNormal(mConvexHullMesh, iFace) );
			mFaceOutsideVertices.emplace( getVerticesOutside(meshVertexIndices, originalMesh, iFace) );
		}
	}


	std::vector<int> QuickHull::getVerticesOutside(
		const std::vector<int>& vertexIndices, const HalfEdgeMesh& meshData,
		int iFace
	) const
	{
		std::vector<int> verticesOutside;
		if (!mConvexHullMesh.faces.isActive(iFace)) { return verticesOutside; }

		// Get the face data from the convex hull
		const HEFace& face = mConvexHullMesh.faces[iFace];
		const glm::vec3 faceNormal = mConvexHullNormals[iFace];
		const HEVertex& faceVertex = mConvexHullMesh.vertices[ mConvexHullMesh.edges[face.edge].vertex ];

		std::copy_if(
			vertexIndices.begin(), vertexIndices.end(), std::back_inserter(verticesOutside),
			[&](int i) { return glm::dot(meshData.vertices[i].location - faceVertex.location, faceNormal) > mScaledEpsilon; }
		);

		return verticesOutside;
	}


	int QuickHull::getFurthestVertexInDirection(
		const std::vector<int>& vertexIndices, const HalfEdgeMesh& meshData,
		const glm::vec3& direction
	) const
	{
		auto itMaxVertex = std::max_element(vertexIndices.begin(), vertexIndices.end(), [&](int iV1, int iV2) {
			float dot1 = glm::dot(meshData.vertices[iV1].location, direction);
			float dot2 = glm::dot(meshData.vertices[iV2].location, direction);
			return dot1 < dot2;
		});
		return (itMaxVertex != vertexIndices.end())? *itMaxVertex : -1;
	}


	void QuickHull::mergeCoplanarFaces(int iFace)
	{
		HEFace inputFace = mConvexHullMesh.faces[iFace];
		glm::vec3 inputFaceNormal = mConvexHullNormals[iFace];

		// Calculate the HEFaces to merge by testing all the opposite HEFaces
		// of the HEEdge loop of the current HEFace
		std::vector<int> facesToMerge;
		int iInitialEdge = inputFace.edge;
		int iCurrentEdge = iInitialEdge;
		do {
			const HEEdge& currentEdge	= mConvexHullMesh.edges[iCurrentEdge];
			const HEEdge& oppositeEdge	= mConvexHullMesh.edges[currentEdge.oppositeEdge];

			// Check if the opposite HEFace exists
			if (oppositeEdge.face >= 0) {
				// Test if the current HEFace is coplanar with the opposite
				// one by the current edge
				glm::vec3 oppositeFaceNormal = mConvexHullNormals[oppositeEdge.face];
				if (glm::all( glm::epsilonEqual(inputFaceNormal, oppositeFaceNormal, mScaledEpsilon) )) {
					facesToMerge.push_back(oppositeEdge.face);
				}
			}

			iCurrentEdge = currentEdge.nextEdge;
		}
		while (iCurrentEdge != iInitialEdge);

		// Merge the HEFaces
		std::vector<int> allOutsideVertices = mFaceOutsideVertices[iFace];
		for (int iFaceToMerge : facesToMerge) {
			// Merge the two HEFaces into the current one
			mergeFaces(mConvexHullMesh, iFace, iFaceToMerge);

			// Remove the opposite HEFace normal
			mConvexHullNormals.erase( mConvexHullNormals.begin().setIndex(iFaceToMerge) );

			// Collect and remove all the opposite HEFace outside HEVertices
			std::vector<int> joinedOutsideVertices;
			std::set_union(
				allOutsideVertices.begin(), allOutsideVertices.end(),
				mFaceOutsideVertices[iFaceToMerge].begin(), mFaceOutsideVertices[iFaceToMerge].end(),
				std::back_inserter(joinedOutsideVertices)
			);
			allOutsideVertices = joinedOutsideVertices;

			mFaceOutsideVertices.erase( mFaceOutsideVertices.begin().setIndex(iFaceToMerge) );
		}

		mFaceOutsideVertices[iFace] = allOutsideVertices;
	}

}
