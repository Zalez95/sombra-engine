#include <tuple>
#include <gtest/gtest.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fe/collision/HalfEdgeMesh.h>
#include "TestMeshes.h"

#define TOLERANCE 0.000001f

TEST(HalfEdgeMesh, getFurthestVertexInDirection1)
{
	fe::collision::HalfEdgeMesh meshData;
	std::map<int, glm::vec3> normals;
	std::tie(meshData, normals) = createTestMesh3();
	const glm::vec3 direction(-0.549725532f, -0.870150089f, 1.638233065f);
	int iExpectedVertex = 19;
	
	int iVertex = fe::collision::getFurthestVertexInDirection(meshData, direction);
	EXPECT_EQ(iVertex, iExpectedVertex);
}


TEST(HalfEdgeMesh, getFaceIndices1)
{
	int iFace = 6;
	std::vector<int> expectedFaceIndices = { 2, 17, 19 };

	fe::collision::HalfEdgeMesh meshData;
	std::map<int, glm::vec3> normals;
	std::tie(meshData, normals) = createTestMesh3();

	std::vector<int> faceIndices = fe::collision::getFaceIndices(meshData, iFace);
	EXPECT_EQ(faceIndices, expectedFaceIndices);
}


TEST(HalfEdgeMesh, mergeFace1)
{
	fe::collision::HalfEdgeMesh meshData;
	std::map<int, glm::vec3> normals;
	std::tie(meshData, normals) = createTestMesh3();

	const std::vector<int> expectedVertices = { 19, 18, 2, 17 };
	const int iMergedFace1 = 17, iMergedFace2 = 6;

	std::size_t nVertices = 0;
	int iJoinedFace = fe::collision::mergeFaces(meshData, iMergedFace1, iMergedFace2);

	ASSERT_EQ(iJoinedFace, iMergedFace1);
	int iInitialEdge = meshData.faces[iJoinedFace].edge;
	int iCurrentEdge = iInitialEdge;
	do {
		auto currentEdge = meshData.edges[iCurrentEdge];
		int iCurrentVertex = currentEdge.vertex;
		EXPECT_TRUE(std::find(expectedVertices.begin(), expectedVertices.end(), iCurrentVertex) != expectedVertices.end());
		iCurrentEdge = currentEdge.nextEdge;
		nVertices++;
	}
	while (iCurrentEdge != iInitialEdge);
	EXPECT_EQ(nVertices, expectedVertices.size());
}


TEST(HalfEdgeMesh, triangulateFaces1)
{
	const std::vector<std::vector<int>> expectedFaceIndices = { { 0, 1, 2 }, { 0, 2, 3 }, { 0, 3, 4 }, { 0, 4, 5 } };

	fe::collision::HalfEdgeMesh meshData;
	std::vector<int> vertexIndices;
	vertexIndices.push_back( fe::collision::addVertex(meshData, glm::vec3( 0.0f,  1.0f,  0.0f)) );
	vertexIndices.push_back( fe::collision::addVertex(meshData, glm::vec3(-0.7f,  0.7f,  0.0f)) );
	vertexIndices.push_back( fe::collision::addVertex(meshData, glm::vec3(-0.7f, -0.7f,  0.0f)) );
	vertexIndices.push_back( fe::collision::addVertex(meshData, glm::vec3( 0.0f, -1.0f,  0.0f)) );
	vertexIndices.push_back( fe::collision::addVertex(meshData, glm::vec3( 0.7f, -0.7f,  0.0f)) );
	vertexIndices.push_back( fe::collision::addVertex(meshData, glm::vec3( 0.7f,  0.7f,  0.0f)) );
	fe::collision::addFace(meshData, vertexIndices);
	fe::collision::triangulateFaces(meshData);

	ASSERT_EQ(static_cast<int>(meshData.faces.size()), 4);
	for (int iFace = 0; iFace < 4; ++iFace) {
		std::vector<int> currentFaceIndices = fe::collision::getFaceIndices(meshData, iFace);
		for (int i = 0; i < 3; ++i) {
			EXPECT_EQ(currentFaceIndices[i], expectedFaceIndices[iFace][i]);
		}
	}
}


TEST(HalfEdgeMesh, calculateFaceNormal1)
{
	fe::collision::HalfEdgeMesh meshData;

	fe::collision::addVertex(meshData, { 1.25f,  1.0f, -2.75f });
	fe::collision::addVertex(meshData, { 1.25f, -1.0f, -2.75f });
	fe::collision::addVertex(meshData, { -0.25f, -1.0f, -2.75f });
	fe::collision::addFace(meshData, { 0, 1, 2 });

	const glm::vec3 expectedNormal(0.0f, 0.0f, -1.0f);
	glm::vec3 normal = fe::collision::calculateFaceNormal(meshData, 0);
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(normal[i], expectedNormal[i], TOLERANCE);
	}
}


TEST(HalfEdgeMesh, calculateFaceNormal2)
{
	fe::collision::HalfEdgeMesh meshData;
	fe::collision::addVertex(meshData, { 0.117263972f,  0.704151272f, -3.100874185f });
	fe::collision::addVertex(meshData, { 0.965986073f, -0.263351202f, -0.244983732f });
	fe::collision::addVertex(meshData, { 0.965986073f, -2.136411190f,  1.768507480f });
	fe::collision::addVertex(meshData, { 0.117263972f, -3.041968584f,  0.926108181f });
	fe::collision::addFace(meshData, { 0, 1, 2, 3 });

	const glm::vec3 expectedNormal(0.824532389f, -0.414277464f, -0.385383605f);
	glm::vec3 normal = fe::collision::calculateFaceNormal(meshData, 0);
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(normal[i], expectedNormal[i], TOLERANCE);
	}
}


TEST(HalfEdgeMesh, calculateHorizon1)
{
	fe::collision::HalfEdgeMesh meshData;
	std::map<int, glm::vec3> normals;
	std::tie(meshData, normals) = createTestMesh3();

	const glm::vec3 eyePoint(-3.49067f, 2.15318f, 1.14567f);
	const std::vector<int> expectedHorizonVertices = { 2, 13, 8, 0, 11, 17 };
	const std::vector<int> expectedFaces = { 4, 25, 26, 31, 32, 33 };

	std::vector<int> horizon, facesToRemove;
	std::tie(horizon, facesToRemove) = calculateHorizon(meshData, normals, eyePoint, 4);

	EXPECT_EQ(horizon.size(), expectedHorizonVertices.size());
	for (int iEdge : horizon) {
		int iCurrentVertex = meshData.edges[iEdge].vertex;
		EXPECT_TRUE(std::find(expectedHorizonVertices.begin(), expectedHorizonVertices.end(), iCurrentVertex)
			!= expectedHorizonVertices.end()
		);
	}
	EXPECT_EQ(facesToRemove.size(), expectedFaces.size());
	for (int iFace : facesToRemove) {
		EXPECT_TRUE(std::find(expectedFaces.begin(), expectedFaces.end(), iFace) != expectedFaces.end());
	}
}
