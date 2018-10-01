#include <set>
#include <stack>
#include <tuple>
#include <limits>
#include <algorithm>
#include "fe/collision/QuickHull.h"

namespace fe { namespace collision {

	void QuickHull::calculate(const HalfEdgeMesh& meshData)
	{
		// Calculate an initial simplex from the meshData
		std::vector<int> iSimplexVertices = calculateInitialSimplex(meshData);

		if (iSimplexVertices.size() < 3) {
			mConvexHull = meshData;
		}
		else if (iSimplexVertices.size() < 4) {
			calculateQuickHull2D(meshData, iSimplexVertices);
		}
		else {
			calculateQuickHull3D(meshData, iSimplexVertices);
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

			glm::vec3 p1 = meshData.vertices[extremePointIndices[i]].location;

			for (int j = i + 1; j < 6; ++j) {
				if (extremePointIndices[j] < 0) { continue; }

				glm::vec3 p2 = meshData.vertices[extremePointIndices[j]].location;

				float currentLength = glm::length(p2 - p1);
				if (currentLength > maxLength) {
					iSimplexVertices[0] = extremePointIndices[i];
					iSimplexVertices[1] = extremePointIndices[j];
					maxLength = currentLength;
				}
			}
		}

		if (maxLength <= 0) {
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
				float currentLength = glm::length(it->location - projection);
				if (currentLength > maxLength) {
					iSimplexVertices[2] = it.getIndex();
					maxLength = currentLength;
				}
			}

			if (maxLength <= 0) {
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

				if (maxLength <= 0) {
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


	void QuickHull::calculateQuickHull2D(const HalfEdgeMesh& meshData, const std::vector<int>& iSimplexVertices)
	{
		assert(meshData.vertices.size() >= 3 && "The mesh must have at least 3 vertices");

		glm::vec3 p0 = meshData.vertices[0].location, p1 = meshData.vertices[1].location, p2 = meshData.vertices[2].location;
		glm::vec3 dirP0P1 = p1 - p0, dirP0P2 = p2 - p1, planeNormal = glm::cross(dirP0P1, dirP0P2);

		// Calculate the convex hull vertices with the iterative quickhull 2D algorithm
		std::stack<QH2DData> quickHull2DStack;

		std::vector<int> allVertexIndices, faceIndices;
		for (auto itVertex = meshData.vertices.begin(); itVertex != meshData.vertices.end(); ++itVertex) {
			allVertexIndices.push_back(itVertex.getIndex());
		}

		auto halfVertexIndices2 = filterOutsideVertices(meshData.vertices, allVertexIndices, planeNormal, iSimplexVertices[1], iSimplexVertices[0]);
		quickHull2DStack.push({ QH2DData::Add, iSimplexVertices[1], iSimplexVertices[0], halfVertexIndices2 });

		auto halfVertexIndices1 = filterOutsideVertices(meshData.vertices, allVertexIndices, planeNormal, iSimplexVertices[0], iSimplexVertices[1]);
		quickHull2DStack.push({ QH2DData::Add, iSimplexVertices[0], iSimplexVertices[1], halfVertexIndices1 });

		while (!quickHull2DStack.empty()) {
			QH2DData& currentQH2DData = quickHull2DStack.top();
			switch (currentQH2DData.state) {
			case QH2DData::Search: {
				if (!currentQH2DData.outsideVertices.empty()) {
					// 1. Get the furthest vertex from the edge between the vertex 1 and 2
					int iFurthestVertex = getFurthestVertexFromEdge(
						meshData.vertices, currentQH2DData.outsideVertices,
						currentQH2DData.iVertex1, currentQH2DData.iVertex2
					);

					// 4. Search the next convex hull vertex in the set of vertices
					// outside the edge furthest-vertex2
					auto outsideVertices2 = filterOutsideVertices(meshData.vertices, currentQH2DData.outsideVertices, planeNormal, iFurthestVertex, currentQH2DData.iVertex2);
					quickHull2DStack.push({ QH2DData::Add, iFurthestVertex, currentQH2DData.iVertex2, outsideVertices2 });

					// 2. Search the next convex hull vertex in the set of vertices
					// outside the edge vertex1-furthest
					auto outsideVertices1 = filterOutsideVertices(meshData.vertices, currentQH2DData.outsideVertices, planeNormal, currentQH2DData.iVertex1, iFurthestVertex);
					quickHull2DStack.push({ QH2DData::Search, currentQH2DData.iVertex1, iFurthestVertex, outsideVertices1 });
				}
				currentQH2DData.state = QH2DData::End;
				break;
			}
			case QH2DData::Add:
				// 3. Add the furthest vertex (iV1) to the convex hull
				faceIndices.push_back( addVertex(mConvexHull, meshData.vertices[currentQH2DData.iVertex1].location) );
				currentQH2DData.state = QH2DData::Search;	// Continue searching
				break;
			case QH2DData::End:
				quickHull2DStack.pop();
				break;
			}
		}

		// Add the convex hull face
		int iNewFace = addFace(mConvexHull, faceIndices);
		mFaceNormals.emplace(iNewFace, calculateFaceNormal(mConvexHull, iNewFace));
	}


	int QuickHull::getFurthestVertexFromEdge(
		const ContiguousVector<HEVertex>& vertices,
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
				return glm::length(vertices[iV1].location - proj1) < glm::length(vertices[iV2].location - proj2);
			}
		);
	}


	std::vector<int> QuickHull::filterOutsideVertices(
		const ContiguousVector<HEVertex>& vertices, const std::vector<int>& vertexIndices,
		const glm::vec3& planeNormal, int iVertex1, int iVertex2
	) const
	{
		std::vector<int> filteredVertices;

		glm::vec3 p1 = vertices[iVertex1].location, p2 = vertices[iVertex2].location;
		glm::vec3 dirP1P2 = p2 - p1, dirOutside = glm::cross(dirP1P2, planeNormal);

		std::copy_if(
			vertexIndices.begin(), vertexIndices.end(), std::back_inserter(filteredVertices),
			[&](int iVertex) { return glm::dot(vertices[iVertex].location - p1, dirOutside) > mEpsilon; }
		);

		return filteredVertices;
	}


	void QuickHull::calculateQuickHull3D(const HalfEdgeMesh& meshData, const std::vector<int>& iSimplexVertices)
	{
		createInitial3DConvexHull(meshData, iSimplexVertices);

		// 1. Find a convex hull face with a non empty outside vertices vector
		auto itFace = mFaceOutsideVertices.begin();
		while ((itFace = std::find_if(
					mFaceOutsideVertices.begin(), mFaceOutsideVertices.end(),
					[](const std::pair<int, std::vector<int>>& pair) { return !pair.second.empty(); }
				)
			) != mFaceOutsideVertices.end()
		) {
			// 2. Get the furthest HEVertex in the direction of the face normal
			int iEyeVertex = getFurthestVertexInDirection(itFace->second, meshData, mFaceNormals[itFace->first]);
			glm::vec3 eyePoint = meshData.vertices[iEyeVertex].location;

			// 3. Check if the eyePoint is already inside in the convex hull
			int iEyeVertexConvexHull;
			auto itVertex = mVertexIndexMap.find(iEyeVertex);
			if (itVertex == mVertexIndexMap.end()) {
				// 3.1 Add the eyePoint to the convex hull
				iEyeVertexConvexHull = addVertex(mConvexHull, eyePoint);
				mVertexIndexMap.emplace(iEyeVertex, iEyeVertexConvexHull);

				// 3.2. Calculate the horizon HEEdges and HEFaces to remove from
				// the current eyePoint perspective
				std::vector<int> horizon, facesToRemove;
				std::tie(horizon, facesToRemove) = calculateHorizon(mConvexHull, mFaceNormals, eyePoint, itFace->first);

				// 3.3. Remove the HEFaces seen from the current eyePoint and
				// collect all their outside HEVertices
				std::vector<int> allOutsideVertices;
				for (int iFaceToRemove : facesToRemove) {
					removeFace(mConvexHull, iFaceToRemove);
					mFaceNormals.erase(iFaceToRemove);

					auto itFOutsideVertices = mFaceOutsideVertices.find(iFaceToRemove);
					if (itFOutsideVertices != mFaceOutsideVertices.end()) {
						std::vector<int> joinedOutsideVertices;
						std::set_union(
							allOutsideVertices.begin(), allOutsideVertices.end(),
							itFOutsideVertices->second.begin(), itFOutsideVertices->second.end(),
							std::back_inserter(joinedOutsideVertices)
						);
						allOutsideVertices = joinedOutsideVertices;

						mFaceOutsideVertices.erase(itFOutsideVertices);
					}
				}

				// 3.4. Create new HEFaces by joining the edges of the horizon
				// with the convex hull eyePoint
				for (int iHorizonEdge : horizon) {
					const HEEdge& currentEdge	= mConvexHull.edges[iHorizonEdge];
					const HEEdge& oppositeEdge	= mConvexHull.edges[currentEdge.oppositeEdge];

					// Create the new HEFace
					int iV0 = oppositeEdge.vertex, iV1 = currentEdge.vertex;
					int iNewFace = addFace(mConvexHull, { iV0, iV1, iEyeVertexConvexHull });
					mFaceNormals.emplace(iNewFace, calculateFaceNormal(mConvexHull, iNewFace));
					mFaceOutsideVertices.emplace(iNewFace, getVerticesOutside(allOutsideVertices, meshData, iNewFace));

					// Merge the coplanar faces
					mergeCoplanarFaces(iNewFace);
				}
			}
		}
	}


	void QuickHull::createInitial3DConvexHull(const HalfEdgeMesh& meshData, const std::vector<int>& iSimplexVertices)
	{
		// Add the vertices to the convex hull
		std::vector<int> chVertexIndices;
		for (int iMeshVertex : iSimplexVertices) {
			int iConvexHullVertex = addVertex(mConvexHull, meshData.vertices[iMeshVertex].location);
			mVertexIndexMap.emplace(iMeshVertex, iConvexHullVertex);
			chVertexIndices.push_back(iConvexHullVertex);
		}

		// Add the faces to the convex hull, 
		const glm::vec3 p0 = mConvexHull.vertices[chVertexIndices[0]].location,
						p1 = mConvexHull.vertices[chVertexIndices[1]].location,
						p2 = mConvexHull.vertices[chVertexIndices[2]].location,
						p3 = mConvexHull.vertices[chVertexIndices[3]].location;
		int iF0, iF1, iF2, iF3;
		const glm::vec3 tNormal = glm::cross(p1 - p0, p2 - p0);
		if (glm::dot(p3 - p0, tNormal) <= 0.0f) {
			iF0 = addFace(mConvexHull, { chVertexIndices[0], chVertexIndices[1], chVertexIndices[2] });
			iF1 = addFace(mConvexHull, { chVertexIndices[0], chVertexIndices[3], chVertexIndices[1] });
			iF2 = addFace(mConvexHull, { chVertexIndices[0], chVertexIndices[2], chVertexIndices[3] });
			iF3 = addFace(mConvexHull, { chVertexIndices[1], chVertexIndices[3], chVertexIndices[2] });
		}
		else {
			iF0 = addFace(mConvexHull, { chVertexIndices[0], chVertexIndices[2], chVertexIndices[1] });
			iF1 = addFace(mConvexHull, { chVertexIndices[0], chVertexIndices[1], chVertexIndices[3] });
			iF2 = addFace(mConvexHull, { chVertexIndices[0], chVertexIndices[3], chVertexIndices[2] });
			iF3 = addFace(mConvexHull, { chVertexIndices[1], chVertexIndices[2], chVertexIndices[3] });
		}

		// Add the HEFaces normals
		mFaceNormals.emplace(iF0, calculateFaceNormal(mConvexHull, iF0));
		mFaceNormals.emplace(iF1, calculateFaceNormal(mConvexHull, iF1));
		mFaceNormals.emplace(iF2, calculateFaceNormal(mConvexHull, iF2));
		mFaceNormals.emplace(iF3, calculateFaceNormal(mConvexHull, iF3));

		// Get all the vertex indices from the original mesh sorted ascendently
		std::vector<int> meshVertexIndices;
		for (auto it = meshData.vertices.begin(); it != meshData.vertices.end(); ++it) {
			meshVertexIndices.push_back(it.getIndex());
		}
		std::sort(meshVertexIndices.begin(), meshVertexIndices.end());

		// Add the HEFaces outside vertices
		mFaceOutsideVertices.emplace(iF0, getVerticesOutside(meshVertexIndices, meshData, iF0));
		mFaceOutsideVertices.emplace(iF1, getVerticesOutside(meshVertexIndices, meshData, iF1));
		mFaceOutsideVertices.emplace(iF2, getVerticesOutside(meshVertexIndices, meshData, iF2));
		mFaceOutsideVertices.emplace(iF3, getVerticesOutside(meshVertexIndices, meshData, iF3));
	}


	std::vector<int> QuickHull::getVerticesOutside(
		const std::vector<int>& vertexIndices, const HalfEdgeMesh& meshData,
		int iFace
	) const
	{
		std::vector<int> verticesOutside;
		if (!mConvexHull.faces.isActive(iFace)) { return verticesOutside; }

		// Get the face data from the convex hull
		const HEFace& face = mConvexHull.faces[iFace];
		const glm::vec3 faceNormal = mFaceNormals.at(iFace);
		const HEVertex& faceVertex = mConvexHull.vertices[ mConvexHull.edges[face.edge].vertex ];

		std::copy_if(
			vertexIndices.begin(), vertexIndices.end(), std::back_inserter(verticesOutside),
			[&](int i) { return glm::dot(meshData.vertices[i].location - faceVertex.location, faceNormal) > mEpsilon; }
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
		HEFace inputFace = mConvexHull.faces[iFace];
		glm::vec3 inputFaceNormal = mFaceNormals[iFace];

		// Calculate the HEFaces to merge by testing all the HEFaces of the
		// HEEdge loop of the current HEFace
		std::set<int> facesToMerge;
		int iInitialEdge = inputFace.edge;
		int iCurrentEdge = iInitialEdge;
		do {
			const HEEdge& currentEdge	= mConvexHull.edges[iCurrentEdge];
			const HEEdge& oppositeEdge	= mConvexHull.edges[currentEdge.oppositeEdge];

			// Check if the opposite HEFace exists
			if (oppositeEdge.face >= 0) {
				// Test if the current HEFace is coplanar with the opposite
				// one by the current edge
				glm::vec3 oppositeFaceNormal = mFaceNormals[oppositeEdge.face];
				if (inputFaceNormal == oppositeFaceNormal) {
					facesToMerge.insert(oppositeEdge.face);
				}
			}

			iCurrentEdge = currentEdge.nextEdge;
		}
		while (iCurrentEdge != iInitialEdge);

		// Merge the HEFaces
		std::vector<int> allOutsideVertices = mFaceOutsideVertices[iFace];
		for (int iMergedFace : facesToMerge) {
			// Merge the two HEFaces into the current one
			mergeFaces(mConvexHull, iFace, iMergedFace);

			// Remove the opposite HEFace normal
			mFaceNormals.erase(iMergedFace);

			// Collect and remove all the opposite HEFace outside HEVertices
			auto itFOutsideVertices = mFaceOutsideVertices.find(iMergedFace);
			if (itFOutsideVertices != mFaceOutsideVertices.end()) {
				std::vector<int> joinedOutsideVertices;
				std::set_union(
					allOutsideVertices.begin(), allOutsideVertices.end(),
					itFOutsideVertices->second.begin(), itFOutsideVertices->second.end(),
					std::back_inserter(joinedOutsideVertices)
				);
				allOutsideVertices = joinedOutsideVertices;

				mFaceOutsideVertices.erase(itFOutsideVertices);
			}
		}

		mFaceOutsideVertices[iFace] = allOutsideVertices;
	}

}}
