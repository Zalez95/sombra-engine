#include <gtest/gtest.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fe/collision/HalfEdgeMesh.h>

#define TOLERANCE 0.000001f


void createTestMesh(fe::collision::HalfEdgeMesh& meshData, std::map<int, glm::vec3>& normals)
{
	int iFace;
	meshData.addVertex({ -3.208401441f,  2.893295764f,  0.028006464f });
	meshData.addVertex({ -6.086990833f,  3.260166883f, -0.342617660f });
	meshData.addVertex({ -5.035281181f,  2.496228456f,  2.278198242f });
	meshData.addVertex({ -7.160157680f,  4.497337341f,  2.081477642f });
	meshData.addVertex({ -2.502496957f,  3.903687715f,  2.681146383f });
	meshData.addVertex({ -4.944808959f,  4.895039081f,  3.950236082f });
	meshData.addVertex({ -5.245877265f,  4.056465148f, -1.327844977f });
	meshData.addVertex({ -3.553897380f,  3.840824604f, -1.109999060f });
	meshData.addVertex({ -4.660541534f,  2.736007690f, -0.394804030f });
	meshData.addVertex({ -2.251753091f,  4.074582576f, -0.011565566f });
	meshData.addVertex({ -6.681442260f,  4.639166355f, -0.581894993f });
	meshData.addVertex({ -2.553625583f,  3.114257574f,  1.382524967f });
	meshData.addVertex({ -1.836821079f,  4.668467521f,  1.547912478f });
	meshData.addVertex({ -5.734357833f,  2.502610445f,  0.927823066f });
	meshData.addVertex({ -4.042359828f,  2.286959648f,  1.145670175f });
	meshData.addVertex({ -7.312233448f,  5.366353988f,  0.842946588f });
	meshData.addVertex({ -6.983338832f,  3.678830623f,  0.812192678f });
	meshData.addVertex({ -3.627435207f,  2.880870103f,  2.705149173f });
	meshData.addVertex({ -6.365145683f,  3.229807853f,  2.352669477f });
	meshData.addVertex({ -5.062996387f,  3.463579893f,  3.451099872f });
	meshData.addVertex({ -3.574266433f,  4.290853500f,  3.687945365f });
	meshData.addVertex({ -6.311958789f,  4.639792919f,  3.335471153f });
	iFace = meshData.addFace({ 6, 3, 4, });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
	iFace = meshData.addFace({ 0, 7, 9 });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
	iFace = meshData.addFace({ 0, 9, 11 });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
	iFace = meshData.addFace({ 1, 8, 13 });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
	iFace = meshData.addFace({ 0, 11, 14 });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
	iFace = meshData.addFace({ 1, 13, 16 });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
	iFace = meshData.addFace({ 2, 17, 19 });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
	iFace = meshData.addFace({ 3, 18, 21 });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
	iFace = meshData.addFace({ 10, 15, 3 });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
	iFace = meshData.addFace({ 4, 12, 9 });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
	iFace = meshData.addFace({ 6, 10, 3 });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
	iFace = meshData.addFace({ 9, 7, 4 });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
	iFace = meshData.addFace({ 21, 5, 20 });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
	iFace = meshData.addFace({ 7, 6, 4 });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
	iFace = meshData.addFace({ 20, 4, 3 });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
	iFace = meshData.addFace({ 21, 19, 5 });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
	iFace = meshData.addFace({ 21, 18, 19 });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
	iFace = meshData.addFace({ 18, 2, 19 });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
	iFace = meshData.addFace({ 19, 20, 5 });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
	iFace = meshData.addFace({ 19, 17, 20 });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
	iFace = meshData.addFace({ 17, 4, 20 });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
	iFace = meshData.addFace({ 3, 21, 20 });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
	iFace = meshData.addFace({ 16, 18, 3 });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
	iFace = meshData.addFace({ 16, 13, 18 });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
	iFace = meshData.addFace({ 13, 2, 18 });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
	iFace = meshData.addFace({ 14, 17, 2 });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
	iFace = meshData.addFace({ 14, 11, 17 });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
	iFace = meshData.addFace({ 11, 4, 17 });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
	iFace = meshData.addFace({ 15, 16, 3 });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
	iFace = meshData.addFace({ 15, 10, 16 });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
	iFace = meshData.addFace({ 10, 1, 16 });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
	iFace = meshData.addFace({ 13, 14, 2 });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
	iFace = meshData.addFace({ 13, 8, 14 });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
	iFace = meshData.addFace({ 8, 0, 14 });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
	iFace = meshData.addFace({ 11, 12, 4 });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
	iFace = meshData.addFace({ 11, 9, 12 });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
	iFace = meshData.addFace({ 6, 1, 10 });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
	iFace = meshData.addFace({ 6, 8, 1 });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
	iFace = meshData.addFace({ 6, 7, 8 });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
	iFace = meshData.addFace({ 7, 0, 8 });
	normals[iFace] = fe::collision::calculateFaceNormal(iFace, meshData);
}


TEST(HalfEdgeMesh, mergeFace1)
{
	fe::collision::HalfEdgeMesh meshData;
	std::map<int, glm::vec3> normals;
	createTestMesh(meshData, normals);

	const std::vector<int> expectedVertices = { 19, 18, 2, 17 };
	const int iMergedEdge = 35;

	std::size_t nVertices = 0;
	int iMergedFace = meshData.getEdge(iMergedEdge).face;
	meshData.mergeFace(iMergedEdge);
	int iInitialEdge = meshData.getFace(iMergedFace).edge;
	int iCurrentEdge = iInitialEdge;
	do {
		auto currentEdge = meshData.getEdge(iCurrentEdge);
		int iCurrentVertex = currentEdge.vertex;
		EXPECT_TRUE(std::find(expectedVertices.begin(), expectedVertices.end(), iCurrentVertex) != expectedVertices.end());
		iCurrentEdge = currentEdge.nextEdge;
		nVertices++;
	}
	while (iCurrentEdge != iInitialEdge);
	EXPECT_EQ(nVertices, expectedVertices.size());
}


TEST(HalfEdgeMesh, calculateFaceNormal1)
{
	fe::collision::HalfEdgeMesh meshData;

	meshData.addVertex({  1.25f,  1.0f, -2.75f });
	meshData.addVertex({  1.25f, -1.0f, -2.75f });
	meshData.addVertex({ -0.25f, -1.0f, -2.75f });
	meshData.addFace({ 0, 1, 2 });

	const glm::vec3 expectedNormal(0.0f, 0.0f, -1.0f);
	glm::vec3 normal = fe::collision::calculateFaceNormal(0, meshData);
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(normal[i], expectedNormal[i], TOLERANCE);
	}
}


TEST(HalfEdgeMesh, calculateFaceNormal2)
{
	fe::collision::HalfEdgeMesh meshData;

	meshData.addVertex({ 0.117263972f,  0.704151272f, -3.100874185f });
	meshData.addVertex({ 0.965986073f, -0.263351202f, -0.244983732f });
	meshData.addVertex({ 0.965986073f, -2.136411190f,  1.768507480f });
	meshData.addVertex({ 0.117263972f, -3.041968584f,  0.926108181f });

	meshData.addFace({ 0, 1, 2, 3 });

	const glm::vec3 expectedNormal(0.824532389f, -0.414277464f, -0.385383605f);
	glm::vec3 normal = fe::collision::calculateFaceNormal(0, meshData);
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(normal[i], expectedNormal[i], TOLERANCE);
	}
}


TEST(HalfEdgeMesh, getFurthestVertexInDirection1)
{
	fe::collision::HalfEdgeMesh meshData;
	std::map<int, glm::vec3> normals;
	createTestMesh(meshData, normals);
	const glm::vec3 direction(-0.549725532f, -0.870150089f, 1.638233065f);
	int iExpectedVertex = 19;
	
	int iVertex = fe::collision::getFurthestVertexInDirection(direction, meshData);
	EXPECT_EQ(iVertex, iExpectedVertex);
}


TEST(HalfEdgeMesh, calculateHorizon1)
{
	fe::collision::HalfEdgeMesh meshData;
	std::map<int, glm::vec3> normals;
	createTestMesh(meshData, normals);

	const glm::vec3 eyePoint(-3.49067f, 2.15318f, 1.14567f);
	const std::vector<int> expectedHorizonVertices = { 2, 13, 8, 0, 11, 17 };
	const std::vector<int> expectedFaces = { 4, 25, 26, 31, 32, 33 };

	std::vector<int> horizon, facesToRemove;
	std::tie(horizon, facesToRemove) = calculateHorizon(eyePoint, 4, meshData, normals);

	EXPECT_EQ(horizon.size(), expectedHorizonVertices.size());
	for (int iEdge : horizon) {
		int iCurrentVertex = meshData.getEdge(iEdge).vertex;
		EXPECT_TRUE(std::find(expectedHorizonVertices.begin(), expectedHorizonVertices.end(), iCurrentVertex)
			!= expectedHorizonVertices.end()
		);
	}
	EXPECT_EQ(facesToRemove.size(), expectedFaces.size());
	for (int iFace : facesToRemove) {
		EXPECT_TRUE(std::find(expectedFaces.begin(), expectedFaces.end(), iFace) != expectedFaces.end());
	}
}
