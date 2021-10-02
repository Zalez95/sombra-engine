#include <array>
#include <tuple>
#include <gtest/gtest.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <se/physics/collision/AABB.h>
#include <se/physics/collision/HalfEdgeMesh.h>
#include <se/physics/collision/HalfEdgeMeshExt.h>
#include "TestMeshes.h"

using namespace se::physics;
static constexpr float kTolerance = 0.000001f;

TEST(HalfEdgeMesh, mergeFace1)
{
	auto meshData = std::get<0>( createTestMesh4() );

	const std::vector<int> expectedVertices = { 19, 18, 2, 17 };
	const int iMergedFace1 = 17, iMergedFace2 = 6;

	std::size_t nVertices = 0;
	int iJoinedFace = mergeFaces(meshData, iMergedFace1, iMergedFace2);

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


TEST(HalfEdgeMesh, getFaceIndices1)
{
	int iFace = 6;
	std::vector<int> expectedFaceIndices = { 2, 17, 19 };

	auto meshData = std::get<0>( createTestMesh4() );

	std::vector<int> faceIndices;
	getFaceIndices(meshData, iFace, std::back_inserter(faceIndices));
	EXPECT_EQ(faceIndices, expectedFaceIndices);
}


TEST(HalfEdgeMesh, triangulateFaces1)
{
	const std::vector<std::vector<int>> expectedFaceIndices = { { 0, 1, 2 }, { 0, 2, 3 }, { 0, 3, 4 }, { 0, 4, 5 } };

	HalfEdgeMesh meshData;
	std::array<int, 6> vertexIndices = {
		addVertex(meshData, glm::vec3( 0.0f,  1.0f,  0.0f)),
		addVertex(meshData, glm::vec3(-0.7f,  0.7f,  0.0f)),
		addVertex(meshData, glm::vec3(-0.7f, -0.7f,  0.0f)),
		addVertex(meshData, glm::vec3( 0.0f, -1.0f,  0.0f)),
		addVertex(meshData, glm::vec3( 0.7f, -0.7f,  0.0f)),
		addVertex(meshData, glm::vec3( 0.7f,  0.7f,  0.0f))
	};
	addFace(meshData, vertexIndices.begin(), vertexIndices.end());
	meshData = triangulateFaces(meshData);

	ASSERT_EQ(static_cast<int>(meshData.faces.size()), 4);
	for (int iFace = 0; iFace < 4; ++iFace) {
		std::vector<int> currentFaceIndices;
		getFaceIndices(meshData, iFace, std::back_inserter(currentFaceIndices));
		for (int i = 0; i < 3; ++i) {
			EXPECT_EQ(currentFaceIndices[i], expectedFaceIndices[iFace][i]);
		}
	}
}


TEST(HalfEdgeMesh, calculateVertexNormal1)
{
	auto [meshData, normals] = createTestMesh1();

	const glm::vec3 expectedNormal(1.0f, 0.0f, 0.0f);
	glm::vec3 normal = calculateVertexNormal(meshData, normals, 8);
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(normal[i], expectedNormal[i], kTolerance);
	}
}


TEST(HalfEdgeMesh, calculateVertexNormal2)
{
	HalfEdgeMesh meshData;
	se::utils::PackedVector<glm::vec3> normals;

	std::array<int, 5> vertexIndices = {
		addVertex(meshData, glm::vec3(-5.035281181f, 2.496228456f, 2.278198242f)),
		addVertex(meshData, glm::vec3(-5.734357833f, 2.502610445f, 0.927823066f)),
		addVertex(meshData, glm::vec3(-3.627435207f, 2.880870103f, 2.705149173f)),
		addVertex(meshData, glm::vec3(-6.365145683f, 3.229807853f, 2.352669477f)),
		addVertex(meshData, glm::vec3(-5.062996387f, 3.463579893f, 3.451099872f))
	};
	std::array<std::array<int, 3>, 3> faceIndices = {{
		{{ vertexIndices[0], vertexIndices[3], vertexIndices[1] }},
		{{ vertexIndices[0], vertexIndices[4], vertexIndices[3] }},
		{{ vertexIndices[0], vertexIndices[2], vertexIndices[4] }}
	}};
	for (const auto& face : faceIndices) {
		int iFace = addFace(meshData, face.begin(), face.end());
		normals.emplace( calculateFaceNormal(meshData, iFace) );
	}

	const glm::vec3 expectedNormal(-0.280267089f, -0.815811336f, 0.505867838f);
	glm::vec3 normal = calculateVertexNormal(meshData, normals, 0);
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(normal[i], expectedNormal[i], kTolerance);
	}
}


TEST(HalfEdgeMesh, calculateFaceNormal1)
{
	HalfEdgeMesh meshData;

	std::array<int, 3> vertexIndices = {
		addVertex(meshData, { 1.25f,  1.0f, -2.75f }),
		addVertex(meshData, { 1.25f, -1.0f, -2.75f }),
		addVertex(meshData, {-0.25f, -1.0f, -2.75f })
	};
	addFace(meshData, vertexIndices.begin(), vertexIndices.end());

	const glm::vec3 expectedNormal(0.0f, 0.0f, -1.0f);
	glm::vec3 normal = calculateFaceNormal(meshData, 0);
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(normal[i], expectedNormal[i], kTolerance);
	}
}


TEST(HalfEdgeMesh, calculateFaceNormal2)
{
	HalfEdgeMesh meshData;
	std::array<int, 4> vertexIndices = {
		addVertex(meshData, { 0.117263972f,  0.704151272f, -3.100874185f }),
		addVertex(meshData, { 0.965986073f, -0.263351202f, -0.244983732f }),
		addVertex(meshData, { 0.965986073f, -2.136411190f,  1.768507480f }),
		addVertex(meshData, { 0.117263972f, -3.041968584f,  0.926108181f })
	};
	addFace(meshData, vertexIndices.begin(), vertexIndices.end());

	const glm::vec3 expectedNormal(0.824532389f, -0.414277464f, -0.385383605f);
	glm::vec3 normal = calculateFaceNormal(meshData, 0);
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(normal[i], expectedNormal[i], kTolerance);
	}
}


TEST(HalfEdgeMesh, calculateFaceNormal3)
{
	HalfEdgeMesh meshData;
	std::array<int, 3> vertexIndices = {
		addVertex(meshData, { -2.0f, -1.0f, 7.0f }),
		addVertex(meshData, { -2.0f, -1.0f, 2.3f }),
		addVertex(meshData, { -2.0f, -1.0f, 5.0f })
	};
	addFace(meshData, vertexIndices.begin(), vertexIndices.end());

	const glm::vec3 expectedNormal(0.0f);
	glm::vec3 normal = calculateFaceNormal(meshData, 0);
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(normal[i], expectedNormal[i], kTolerance);
	}
}


TEST(HalfEdgeMesh, calculateFaceCentroid1)
{
	HalfEdgeMesh meshData;
	std::array<int, 3> vertexIndices = {
		addVertex(meshData, { -2.0f, -1.0f, 7.0f }),
		addVertex(meshData, { -2.0f, -1.0f, 2.3f }),
		addVertex(meshData, { -2.0f, -1.0f, 5.0f })
	};
	addFace(meshData, vertexIndices.begin(), vertexIndices.end());

	const glm::vec3 expectedCentroid(0-2.0f, -1.0f, 4.766666666f);
	glm::vec3 centroid = calculateFaceCentroid(meshData, 0);
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(centroid[i], expectedCentroid[i], kTolerance);
	}
}


TEST(HalfEdgeMesh, calculateFaceArea1)
{
	HalfEdgeMesh meshData;
	std::array<int, 3> vertexIndices = {
		addVertex(meshData, { 1.25f,  1.0f, -2.75f }),
		addVertex(meshData, { 1.25f, -1.0f, -2.75f }),
		addVertex(meshData, {-0.25f, -1.0f, -2.75f })
	};
	addFace(meshData, vertexIndices.begin(), vertexIndices.end());

	const float expectedArea = 1.5f;
	float area = calculateFaceArea(meshData, 0);
	EXPECT_NEAR(area, expectedArea, kTolerance);
}


TEST(HalfEdgeMesh, calculateFaceArea2)
{
	HalfEdgeMesh meshData;
	std::array<int, 4> vertexIndices = {
		addVertex(meshData, { 0.117263972f,  0.704151272f, -3.100874185f }),
		addVertex(meshData, { 0.965986073f, -0.263351202f, -0.244983732f }),
		addVertex(meshData, { 0.965986073f, -2.136411190f,  1.768507480f }),
		addVertex(meshData, { 0.117263972f, -3.041968584f,  0.926108181f })
	};
	addFace(meshData, vertexIndices.begin(), vertexIndices.end());

	const float expectedArea = 6.187500476f;
	float area = calculateFaceArea(meshData, 0);
	EXPECT_NEAR(area, expectedArea, kTolerance);
}


TEST(HalfEdgeMesh, calculateFaceArea3)
{
	HalfEdgeMesh meshData;
	std::array<int, 3> vertexIndices = {
		addVertex(meshData, { -2.0f, -1.0f, 7.0f }),
		addVertex(meshData, { -2.0f, -1.0f, 2.3f }),
		addVertex(meshData, { -2.0f, -1.0f, 5.0f })
	};
	addFace(meshData, vertexIndices.begin(), vertexIndices.end());

	const float expectedArea = 0.0f;
	float area = calculateFaceArea(meshData, 0);
	EXPECT_NEAR(area, expectedArea, kTolerance);
}


TEST(HalfEdgeMesh, calculateAABB1)
{
	HalfEdgeMesh meshData = createTestMesh2().first;

	const AABB expectedAABB = { glm::vec3(-0.25f, -1.0f, -2.75f), glm::vec3(1.25f, 1.0f, 2.75f) };
	AABB aabb = calculateAABB(meshData);
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(aabb.minimum[i], expectedAABB.minimum[i], kTolerance);
		EXPECT_NEAR(aabb.maximum[i], expectedAABB.maximum[i], kTolerance);
	}
}


TEST(HalfEdgeMesh, calculateCentroid1)
{
	HalfEdgeMesh meshData = createTestMesh2().first;

	const glm::vec3 expectedCentroid(0.5f, 0.0f, 0.0f);
	glm::vec3 centroid = calculateCentroid(meshData);
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(centroid[i], expectedCentroid[i], kTolerance);
	}
}


TEST(HalfEdgeMesh, calculateCentroid2)
{
	HalfEdgeMesh meshData;

	const glm::vec3 expectedCentroid(0.0f);
	glm::vec3 centroid = calculateCentroid(meshData);
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(centroid[i], expectedCentroid[i], kTolerance);
	}
}


TEST(HalfEdgeMesh, calculateArea1)
{
	HalfEdgeMesh meshData = createTestMesh2().first;

	const float expectedArea = 39.37604141f;
	float area = calculateArea(meshData);
	EXPECT_NEAR(area, expectedArea, kTolerance);
}


TEST(HalfEdgeMesh, calculateVolume1)
{
	auto [meshData, normals] = createTestMesh2();

	const float expectedVolume = 13.75000095f;
	float volume = calculateVolume(meshData, normals);
	EXPECT_NEAR(volume, expectedVolume, kTolerance);
}


TEST(HalfEdgeMesh, getFurthestVertexInDirection1)
{
	auto meshData = std::get<0>( createTestMesh4() );
	const glm::vec3 direction(-0.549725532f, -0.870150089f, 1.638233065f);
	int iExpectedVertex = 19;

	int iVertex = getFurthestVertexInDirection(meshData, direction);
	EXPECT_EQ(iVertex, iExpectedVertex);
}


TEST(HalfEdgeMesh, calculateHorizon1)
{
	auto [meshData, normals] = createTestMesh4();

	const glm::vec3 eyePoint(-3.49067f, 2.15318f, 1.14567f);
	const std::vector<int> expectedHorizonVertices = { 2, 13, 8, 0, 11, 17 };
	const std::vector<int> expectedFaces = { 4, 25, 26, 31, 32, 33 };

	std::vector<int> horizon, facesToRemove;
	calculateHorizon(meshData, normals, eyePoint, 4, horizon, facesToRemove);
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
