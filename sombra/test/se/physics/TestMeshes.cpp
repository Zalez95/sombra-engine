#include <array>
#include <vector>
#include "TestMeshes.h"
#include <se/physics/collision/HalfEdgeMeshExt.h>

using namespace se::physics;


HalfEdgeMesh createTestPolyhedron1()
{
	HalfEdgeMesh meshData;
	std::array<int, 8> vertexIndices = {
		addVertex(meshData, {  1.0f,  1.0f, -1.0f }),
		addVertex(meshData, {  1.0f, -1.0f, -1.0f }),
		addVertex(meshData, { -1.0f, -1.0f, -1.0f }),
		addVertex(meshData, { -1.0f,  1.0f, -1.0f }),
		addVertex(meshData, {  1.0f,  1.0f,  1.0f }),
		addVertex(meshData, {  0.0f, -1.0f,  1.0f }),
		addVertex(meshData, { -1.0f, -1.0f,  1.0f }),
		addVertex(meshData, { -1.0f,  1.0f,  1.0f })
	};
	std::array<std::vector<int>, 7> faceIndices = {{
		{{ vertexIndices[3], vertexIndices[2], vertexIndices[6], vertexIndices[7] }},
		{{ vertexIndices[2], vertexIndices[1], vertexIndices[5], vertexIndices[6] }},
		{{ vertexIndices[1], vertexIndices[4], vertexIndices[5] }},
		{{ vertexIndices[1], vertexIndices[0], vertexIndices[4] }},
		{{ vertexIndices[0], vertexIndices[3], vertexIndices[7], vertexIndices[4] }},
		{{ vertexIndices[4], vertexIndices[7], vertexIndices[6], vertexIndices[5] }},
		{{ vertexIndices[0], vertexIndices[1], vertexIndices[2], vertexIndices[3] }}
	}};
	for (const auto& face : faceIndices) {
		addFace(meshData, face.begin(), face.end());
	}

	return meshData;
}


HalfEdgeMesh createTestPolyhedron2()
{
	HalfEdgeMesh meshData;
	std::array<int, 5> vertexIndices = {
		addVertex(meshData, { 0.0f, 0.0f, 0.0f }),
		addVertex(meshData, { 1.0f, 0.0f, 0.0f }),
		addVertex(meshData, { 1.0f, 1.0f, 0.0f }),
		addVertex(meshData, { 0.0f, 1.0f, 0.0f }),
		addVertex(meshData, { 0.5f, 0.5f, 1.6f })
	};
	std::array<std::vector<int>, 5> faceIndices = {{
		{{ vertexIndices[3], vertexIndices[2], vertexIndices[1], vertexIndices[0] }},
		{{ vertexIndices[0], vertexIndices[1], vertexIndices[4] }},
		{{ vertexIndices[1], vertexIndices[2], vertexIndices[4] }},
		{{ vertexIndices[2], vertexIndices[3], vertexIndices[4] }},
		{{ vertexIndices[3], vertexIndices[0], vertexIndices[4] }}
	}};
	for (const auto& face : faceIndices) {
		addFace(meshData, face.begin(), face.end());
	}

	return meshData;
}


HalfEdgeMesh createTestPolyhedron3()
{
	HalfEdgeMesh meshData;
	std::array<int, 5> vertexIndices = {
		addVertex(meshData, { -1.0f, -0.5f,  0.5f }),
		addVertex(meshData, { -1.0f, -0.5f, -0.5f }),
		addVertex(meshData, { -1.0f,  0.5f, -0.5f }),
		addVertex(meshData, { -1.0f,  0.5f,  0.5f }),
		addVertex(meshData, {  1.0f,  0.0f,  0.0f })
	};
	std::array<std::vector<int>, 5> faceIndices = {{
		{{ vertexIndices[3], vertexIndices[2], vertexIndices[1], vertexIndices[0] }},
		{{ vertexIndices[0], vertexIndices[1], vertexIndices[4] }},
		{{ vertexIndices[1], vertexIndices[2], vertexIndices[4] }},
		{{ vertexIndices[2], vertexIndices[3], vertexIndices[4] }},
		{{ vertexIndices[3], vertexIndices[0], vertexIndices[4] }}
	}};
	for (const auto& face : faceIndices) {
		addFace(meshData, face.begin(), face.end());
	}

	return meshData;
}


std::pair<HalfEdgeMesh, se::utils::PackedVector<glm::vec3>> createTestMesh1()
{
	HalfEdgeMesh meshData;
	se::utils::PackedVector<glm::vec3> normals;

	std::array<int, 9> vertexIndices = {
		addVertex(meshData, {  1.25f,  1.0f, -2.75f }),
		addVertex(meshData, {  1.25f, -1.0f, -2.75f }),
		addVertex(meshData, { -0.25f, -1.0f, -2.75f }),
		addVertex(meshData, { -0.25f,  1.0f,  0.0f  }),
		addVertex(meshData, {  1.25f,  1.0f,  2.75f }),
		addVertex(meshData, {  1.25f, -1.0f,  2.75f }),
		addVertex(meshData, { -0.25f, -1.0f,  0.0f  }),
		addVertex(meshData, { -0.25f,  1.0f,  2.75f }),
		addVertex(meshData, {  0.25f,  0.0f,  0.0f  })
	};
	std::array<std::array<int, 3>, 14> faceIndices = {{
		{{ vertexIndices[0], vertexIndices[1], vertexIndices[2] }},
		{{ vertexIndices[7], vertexIndices[6], vertexIndices[5] }},
		{{ vertexIndices[1], vertexIndices[5], vertexIndices[6] }},
		{{ vertexIndices[6], vertexIndices[7], vertexIndices[3] }},
		{{ vertexIndices[4], vertexIndices[0], vertexIndices[3] }},
		{{ vertexIndices[0], vertexIndices[4], vertexIndices[8] }},
		{{ vertexIndices[1], vertexIndices[0], vertexIndices[8] }},
		{{ vertexIndices[4], vertexIndices[5], vertexIndices[8] }},
		{{ vertexIndices[5], vertexIndices[1], vertexIndices[8] }},
		{{ vertexIndices[3], vertexIndices[0], vertexIndices[2] }},
		{{ vertexIndices[4], vertexIndices[7], vertexIndices[5] }},
		{{ vertexIndices[2], vertexIndices[1], vertexIndices[6] }},
		{{ vertexIndices[2], vertexIndices[6], vertexIndices[3] }},
		{{ vertexIndices[7], vertexIndices[4], vertexIndices[3] }}
	}};
	for (const auto& face : faceIndices) {
		int iFace = addFace(meshData, face.begin(), face.end());
		normals.emplace( calculateFaceNormal(meshData, iFace) );
	}

	return std::make_pair(meshData, normals);
}


std::pair<HalfEdgeMesh, se::utils::PackedVector<glm::vec3>> createTestMesh2()
{
	HalfEdgeMesh meshData;
	se::utils::PackedVector<glm::vec3> normals;

	std::array<int, 8> vertexIndices = {
		addVertex(meshData, { 1.25f,  1.0f, -2.75f }),
		addVertex(meshData, { 1.25f, -1.0f, -2.75f }),
		addVertex(meshData, { -0.25f, -1.0f, -2.75f }),
		addVertex(meshData, { -0.25f,  1.0f,  0.0f }),
		addVertex(meshData, { 1.25f,  1.0f,  2.75f }),
		addVertex(meshData, { 1.25f, -1.0f,  2.75f }),
		addVertex(meshData, { -0.25f, -1.0f,  0.0f }),
		addVertex(meshData, { -0.25f,  1.0f,  2.75f })
	};
	std::array<std::vector<int>, 8> faceIndices = {{
		{{ vertexIndices[0], vertexIndices[1], vertexIndices[2] }},
		{{ vertexIndices[0], vertexIndices[2], vertexIndices[3] }},
		{{ vertexIndices[3], vertexIndices[2], vertexIndices[6], vertexIndices[7] }},
		{{ vertexIndices[7], vertexIndices[6], vertexIndices[5] }},
		{{ vertexIndices[7], vertexIndices[5], vertexIndices[4] }},
		{{ vertexIndices[2], vertexIndices[1], vertexIndices[5], vertexIndices[6] }},
		{{ vertexIndices[1], vertexIndices[0], vertexIndices[4], vertexIndices[5] }},
		{{ vertexIndices[0], vertexIndices[3], vertexIndices[7], vertexIndices[4] }}
	}};
	for (const auto& face : faceIndices) {
		int iFace = addFace(meshData, face.begin(), face.end());
		normals.emplace(calculateFaceNormal(meshData, iFace));
	}

	return std::make_pair(meshData, normals);
}


std::vector<HalfEdgeMesh> createTestMesh3()
{
	HalfEdgeMesh m1;
	std::array<int, 6> vertexIndices1 = {
		addVertex(m1, { -0.25f,  1.0f, 0.0f }),
		addVertex(m1, { 1.25f,  1.0f, 2.75f }),
		addVertex(m1, { 1.25f, -1.0f, 2.75f }),
		addVertex(m1, { -0.25f, -1.0f, 0.0f }),
		addVertex(m1, { -0.25f,  1.0f, 2.75f }),
		addVertex(m1, { 0.25f,  0.0f, 0.0f })
	};
	std::array<std::array<int, 3>, 8> faceIndices1 = {{
		{{ vertexIndices1[4], vertexIndices1[3], vertexIndices1[2] }},
		{{ vertexIndices1[3], vertexIndices1[4], vertexIndices1[0] }},
		{{ vertexIndices1[1], vertexIndices1[2], vertexIndices1[5] }},
		{{ vertexIndices1[1], vertexIndices1[4], vertexIndices1[2] }},
		{{ vertexIndices1[4], vertexIndices1[1], vertexIndices1[0] }},
		{{ vertexIndices1[3], vertexIndices1[5], vertexIndices1[2] }},
		{{ vertexIndices1[3], vertexIndices1[0], vertexIndices1[5] }},
		{{ vertexIndices1[1], vertexIndices1[5], vertexIndices1[0] }}
	}};
	for (const auto& face : faceIndices1) {
		addFace(m1, face.begin(), face.end());
	}

	HalfEdgeMesh m2;
	std::array<int, 4> vertexIndices2 = {
		addVertex(m2, { 0.25f,  0.0f,  0.0f }),
		addVertex(m2, { 1.25f,  1.0f,  2.75f }),
		addVertex(m2, { -0.25f,  1.0f,  0.0f }),
		addVertex(m2, { 1.25f,  1.0f, -2.75f })
	};
	std::array<std::array<int, 3>, 4> faceIndices2 = {{
		{{ vertexIndices2[3], vertexIndices2[1], vertexIndices2[0] }},
		{{ vertexIndices2[1], vertexIndices2[3], vertexIndices2[2] }},
		{{ vertexIndices2[1], vertexIndices2[2], vertexIndices2[0] }},
		{{ vertexIndices2[0], vertexIndices2[2], vertexIndices2[3] }}
	}};
	for (const auto& face : faceIndices2) {
		addFace(m2, face.begin(), face.end());
	}

	HalfEdgeMesh m3;
	std::array<int, 4> vertexIndices3 = {
		addVertex(m3, { 1.25f, -1.0f, -2.75f }),
		addVertex(m3, { 1.25f, -1.0f,  2.75f }),
		addVertex(m3, { -0.25f, -1.0f,  0.0f }),
		addVertex(m3, { 0.25f,  0.0f,  0.0f })
	};
	std::array<std::array<int, 3>, 4> faceIndices3 = {{
		{{ vertexIndices3[0], vertexIndices3[1], vertexIndices3[2] }},
		{{ vertexIndices3[1], vertexIndices3[0], vertexIndices3[3] }},
		{{ vertexIndices3[2], vertexIndices3[1], vertexIndices3[3] }},
		{{ vertexIndices3[2], vertexIndices3[3], vertexIndices3[0] }}
	}};
	for (const auto& face : faceIndices3) {
		addFace(m3, face.begin(), face.end());
	}

	HalfEdgeMesh m4;
	std::array<int, 6> vertexIndices4 = {
		addVertex(m4, { 1.25f,  1.0f, -2.75f }),
		addVertex(m4, { 1.25f, -1.0f, -2.75f }),
		addVertex(m4, { -0.25f, -1.0f, -2.75f }),
		addVertex(m4, { -0.25f,  1.0f,  0.0f }),
		addVertex(m4, { -0.25f, -1.0f,  0.0f }),
		addVertex(m4, { 0.25f,  0.0f,  0.0f })
	};
	std::array<std::array<int, 3>, 8> faceIndices4 = {{
		{{ vertexIndices4[0], vertexIndices4[1], vertexIndices4[2] }},
		{{ vertexIndices4[1], vertexIndices4[0], vertexIndices4[5] }},
		{{ vertexIndices4[3], vertexIndices4[0], vertexIndices4[2] }},
		{{ vertexIndices4[2], vertexIndices4[1], vertexIndices4[4] }},
		{{ vertexIndices4[2], vertexIndices4[4], vertexIndices4[3] }},
		{{ vertexIndices4[3], vertexIndices4[5], vertexIndices4[0] }},
		{{ vertexIndices4[4], vertexIndices4[1], vertexIndices4[5] }},
		{{ vertexIndices4[4], vertexIndices4[5], vertexIndices4[3] }}
	}};
	for (const auto& face : faceIndices4) {
		addFace(m4, face.begin(), face.end());
	}

	return { m1, m2, m3, m4 };
}


std::pair<HalfEdgeMesh, se::utils::PackedVector<glm::vec3>> createTestMesh4()
{
	HalfEdgeMesh meshData;
	se::utils::PackedVector<glm::vec3> normals;

	std::array<int, 22> vertexIndices = {
		addVertex(meshData, { -3.208401441f,  2.893295764f,  0.028006464f }),
		addVertex(meshData, { -6.086990833f,  3.260166883f, -0.342617660f }),
		addVertex(meshData, { -5.035281181f,  2.496228456f,  2.278198242f }),
		addVertex(meshData, { -7.160157680f,  4.497337341f,  2.081477642f }),
		addVertex(meshData, { -2.502496957f,  3.903687715f,  2.681146383f }),
		addVertex(meshData, { -4.944808959f,  4.895039081f,  3.950236082f }),
		addVertex(meshData, { -5.245877265f,  4.056465148f, -1.327844977f }),
		addVertex(meshData, { -3.553897380f,  3.840824604f, -1.109999060f }),
		addVertex(meshData, { -4.660541534f,  2.736007690f, -0.394804030f }),
		addVertex(meshData, { -2.251753091f,  4.074582576f, -0.011565566f }),
		addVertex(meshData, { -6.681442260f,  4.639166355f, -0.581894993f }),
		addVertex(meshData, { -2.553625583f,  3.114257574f,  1.382524967f }),
		addVertex(meshData, { -1.836821079f,  4.668467521f,  1.547912478f }),
		addVertex(meshData, { -5.734357833f,  2.502610445f,  0.927823066f }),
		addVertex(meshData, { -4.042359828f,  2.286959648f,  1.145670175f }),
		addVertex(meshData, { -7.312233448f,  5.366353988f,  0.842946588f }),
		addVertex(meshData, { -6.983338832f,  3.678830623f,  0.812192678f }),
		addVertex(meshData, { -3.627435207f,  2.880870103f,  2.705149173f }),
		addVertex(meshData, { -6.365145683f,  3.229807853f,  2.352669477f }),
		addVertex(meshData, { -5.062996387f,  3.463579893f,  3.451099872f }),
		addVertex(meshData, { -3.574266433f,  4.290853500f,  3.687945365f }),
		addVertex(meshData, { -6.311958789f,  4.639792919f,  3.335471153f }),
	};
	std::array<std::array<int, 3>, 40> faceIndices = {{
		{{ vertexIndices[6], vertexIndices[3], vertexIndices[4] }},
		{{ vertexIndices[0], vertexIndices[7], vertexIndices[9] }},
		{{ vertexIndices[0], vertexIndices[9], vertexIndices[11] }},
		{{ vertexIndices[1], vertexIndices[8], vertexIndices[13] }},
		{{ vertexIndices[0], vertexIndices[11], vertexIndices[14] }},
		{{ vertexIndices[1], vertexIndices[13], vertexIndices[16] }},
		{{ vertexIndices[2], vertexIndices[17], vertexIndices[19] }},
		{{ vertexIndices[3], vertexIndices[18], vertexIndices[21] }},
		{{ vertexIndices[10], vertexIndices[15], vertexIndices[3] }},
		{{ vertexIndices[4], vertexIndices[12], vertexIndices[9] }},
		{{ vertexIndices[6], vertexIndices[10], vertexIndices[3] }},
		{{ vertexIndices[9], vertexIndices[7], vertexIndices[4] }},
		{{ vertexIndices[21], vertexIndices[5], vertexIndices[20] }},
		{{ vertexIndices[7], vertexIndices[6], vertexIndices[4] }},
		{{ vertexIndices[20], vertexIndices[4], vertexIndices[3] }},
		{{ vertexIndices[21], vertexIndices[19], vertexIndices[5] }},
		{{ vertexIndices[21], vertexIndices[18], vertexIndices[19] }},
		{{ vertexIndices[18], vertexIndices[2], vertexIndices[19] }},
		{{ vertexIndices[19], vertexIndices[20], vertexIndices[5] }},
		{{ vertexIndices[19], vertexIndices[17], vertexIndices[20] }},
		{{ vertexIndices[17], vertexIndices[4], vertexIndices[20] }},
		{{ vertexIndices[3], vertexIndices[21], vertexIndices[20] }},
		{{ vertexIndices[16], vertexIndices[18], vertexIndices[3] }},
		{{ vertexIndices[16], vertexIndices[13], vertexIndices[18] }},
		{{ vertexIndices[13], vertexIndices[2], vertexIndices[18] }},
		{{ vertexIndices[14], vertexIndices[17], vertexIndices[2] }},
		{{ vertexIndices[14], vertexIndices[11], vertexIndices[17] }},
		{{ vertexIndices[11], vertexIndices[4], vertexIndices[17] }},
		{{ vertexIndices[15], vertexIndices[16], vertexIndices[3] }},
		{{ vertexIndices[15], vertexIndices[10], vertexIndices[16] }},
		{{ vertexIndices[10], vertexIndices[1], vertexIndices[16] }},
		{{ vertexIndices[13], vertexIndices[14], vertexIndices[2] }},
		{{ vertexIndices[13], vertexIndices[8], vertexIndices[14] }},
		{{ vertexIndices[8], vertexIndices[0], vertexIndices[14] }},
		{{ vertexIndices[11], vertexIndices[12], vertexIndices[4] }},
		{{ vertexIndices[11], vertexIndices[9], vertexIndices[12] }},
		{{ vertexIndices[6], vertexIndices[1], vertexIndices[10] }},
		{{ vertexIndices[6], vertexIndices[8], vertexIndices[1] }},
		{{ vertexIndices[6], vertexIndices[7], vertexIndices[8] }},
		{{ vertexIndices[7], vertexIndices[0], vertexIndices[8] }}
	}};
	for (const auto& face : faceIndices) {
		int iFace = addFace(meshData, face.begin(), face.end());
		normals.emplace( calculateFaceNormal(meshData, iFace) );
	}

	return std::make_pair(meshData, normals);
}


HalfEdgeMesh createTestMesh5()
{
	HalfEdgeMesh meshData;

	std::array<int, 8> vertexIndices = {
		addVertex(meshData, { 0.0f, 1.0f, 0.0f }),
		addVertex(meshData, { 0.866025328f, -0.5f, 0.0f }),
		addVertex(meshData, { -0.866025447f, -0.5f, 0.0f }),
		addVertex(meshData, { 0.0f, 0.0f, 2.0f }),
		addVertex(meshData, { 0.0f, 0.5f, 0.0f }),
		addVertex(meshData, { 0.433012664f, -0.25f, 0.0f }),
		addVertex(meshData, { -0.433012723f, -0.25f, 0.0f }),
		addVertex(meshData, { 0.0f, 0.0f, 1.0f })
	};
	std::array<std::vector<int>, 9> faceIndices = {{
		{ vertexIndices[0], vertexIndices[3], vertexIndices[1] },
		{ vertexIndices[1], vertexIndices[3], vertexIndices[2] },
		{ vertexIndices[2], vertexIndices[3], vertexIndices[0] },
		{ vertexIndices[0], vertexIndices[4], vertexIndices[6], vertexIndices[2] },
		{ vertexIndices[4], vertexIndices[5], vertexIndices[7] },
		{ vertexIndices[5], vertexIndices[6], vertexIndices[7] },
		{ vertexIndices[6], vertexIndices[4], vertexIndices[7] },
		{ vertexIndices[1], vertexIndices[5], vertexIndices[4], vertexIndices[0] },
		{ vertexIndices[2], vertexIndices[6], vertexIndices[5], vertexIndices[1] }
	}};
	for (const auto& face : faceIndices) {
		addFace(meshData, face.begin(), face.end());
	}

	return meshData;
}


std::vector<HalfEdgeMesh> createTestMesh6()
{
	HalfEdgeMesh m0;
	std::array<int, 6> vertexIndices0 = {
		addVertex(m0, { 0.0f, 1.0f, 0.0f }),
		addVertex(m0, { 0.866025328f, -0.5f, 0.0f }),
		addVertex(m0, { 0.0f, 0.0f, 2.0f }),
		addVertex(m0, { 0.0f, 0.5f, 0.0f }),
		addVertex(m0, { 0.433012664f, -0.25f, 0.0f }),
		addVertex(m0, { 0.0f, 0.0f, 1.0f })
	};
	std::array<std::vector<int>, 5> faceIndices0 = {{
		{{ vertexIndices0[0], vertexIndices0[2], vertexIndices0[1] }},
		{{ vertexIndices0[3], vertexIndices0[4], vertexIndices0[5] }},
		{{ vertexIndices0[5], vertexIndices0[4], vertexIndices0[1], vertexIndices0[2] }},
		{{ vertexIndices0[0], vertexIndices0[3], vertexIndices0[5], vertexIndices0[2] }},
		{{ vertexIndices0[1], vertexIndices0[4], vertexIndices0[3], vertexIndices0[0] }}
	}};
	for (const auto& face : faceIndices0) {
		addFace(m0, face.begin(), face.end());
	}

	HalfEdgeMesh m1;
	std::array<int, 6> vertexIndices1 = {
		addVertex(m1, { 0.0f, 1.0f, 0.0f }),
		addVertex(m1, { -0.866025447f, -0.5f, 0.0f }),
		addVertex(m1, { 0.0f, 0.0f, 2.0f }),
		addVertex(m1, { 0.0f, 0.5f, 0.0f }),
		addVertex(m1, { -0.433012723f, -0.25f, 0.0f }),
		addVertex(m1, { 0.0f, 0.0f, 1.0f })
	};
	std::array<std::vector<int>, 5> faceIndices1 = {{
		{{ vertexIndices1[1], vertexIndices1[2], vertexIndices1[0] }},
		{{ vertexIndices1[0], vertexIndices1[3], vertexIndices1[4], vertexIndices1[1] }},
		{{ vertexIndices1[0], vertexIndices1[2], vertexIndices1[5], vertexIndices1[3] }},
		{{ vertexIndices1[1], vertexIndices1[4], vertexIndices1[5], vertexIndices1[2] }},
		{{ vertexIndices1[4], vertexIndices1[3], vertexIndices1[5] }}
	}};
	for (const auto& face : faceIndices1) {
		addFace(m1, face.begin(), face.end());
	}

	HalfEdgeMesh m2;
	std::array<int, 6> vertexIndices2 = {
		addVertex(m2, { 0.866025328f, -0.5f, 0.0 }),
		addVertex(m2, { -0.866025447f, -0.5f, 0.0 }),
		addVertex(m2, { 0.0f, 0.0f, 2.0 }),
		addVertex(m2, { 0.433012664f, -0.25f, 0.0 }),
		addVertex(m2, { -0.433012723f, -0.25f, 0.0 }),
		addVertex(m2, { 0.0f, 0.0f, 1.0 })
	};
	std::array<std::vector<int>, 5> faceIndices2 = {{
		{{ vertexIndices2[0], vertexIndices2[2], vertexIndices2[1] }},
		{{ vertexIndices2[4], vertexIndices2[1], vertexIndices2[2], vertexIndices2[5] }},
		{{ vertexIndices2[3], vertexIndices2[4], vertexIndices2[5] }},
		{{ vertexIndices2[5], vertexIndices2[2], vertexIndices2[0], vertexIndices2[3] }},
		{{ vertexIndices2[1], vertexIndices2[4], vertexIndices2[3], vertexIndices2[0] }}
	}};
	for (const auto& face : faceIndices2) {
		addFace(m2, face.begin(), face.end());
	}

	return { m0, m1, m2 };
}


HalfEdgeMesh createTestPlane1()
{
	HalfEdgeMesh meshData;
	std::array<int, 6> vertexIndices = {
		addVertex(meshData, { -17.2071228f, -10.5431643f, 5.25000095f }),
		addVertex(meshData, { -17.2071228f, -8.54316425f, 2.31236959f }),
		addVertex(meshData, { -17.2071228f, -10.5431643f, 2.31236959f }),
		addVertex(meshData, { -17.2071228f, -10.5431643f, 7.00000095f }),
		addVertex(meshData, { -17.2071228f, -8.54316425f, 5.25000095f }),
		addVertex(meshData, { -17.2071228f, -8.54316425f, 7.00000095f })
	};
	std::array<std::array<int, 3>, 4> faceIndices = {{
		{{ vertexIndices[0], vertexIndices[1], vertexIndices[2] }},
		{{ vertexIndices[3], vertexIndices[4], vertexIndices[0] }},
		{{ vertexIndices[0], vertexIndices[4], vertexIndices[1] }},
		{{ vertexIndices[3], vertexIndices[5], vertexIndices[4] }}
	}};
	for (const auto& face : faceIndices) {
		addFace(meshData, face.begin(), face.end());
	}

	return meshData;
}


HalfEdgeMesh createTestPlane2()
{
	HalfEdgeMesh meshData;
	std::array<int, 4> vertexIndices = {
		addVertex(meshData, { -17.2071228f, -8.54316425f, 2.31236959f }),
		addVertex(meshData, { -17.2071228f, -10.5431643f, 2.31236959f }),
		addVertex(meshData, { -17.2071228f, -10.5431643f, 7.00000095f }),
		addVertex(meshData, { -17.2071228f, -8.54316425f, 7.00000095f })
	};
	std::array<int, 4> faceIndices = { vertexIndices[0], vertexIndices[1], vertexIndices[2], vertexIndices[3] };
	addFace(meshData, faceIndices.begin(), faceIndices.end());
	return meshData;
}


HalfEdgeMesh createTestTube1()
{
	HalfEdgeMesh meshData;
	std::array<int, 48> vertexIndices = {
		addVertex(meshData, { -0.000000014f, 0.499999761f, -1.0f }),
		addVertex(meshData, { -0.000000014f, 0.499999761f, 1.0f }),
		addVertex(meshData, { 0.249999970f, 0.433012485f, -1.0f }),
		addVertex(meshData, { 0.249999970f, 0.433012485f, 1.0f }),
		addVertex(meshData, { 0.433012694f, 0.249999791f, -1.0f }),
		addVertex(meshData, { 0.433012694f, 0.249999791f, 1.0f }),
		addVertex(meshData, { 0.5f, -0.000000210f, -1.0f }),
		addVertex(meshData, { 0.5f, -0.000000210f, 1.0f }),
		addVertex(meshData, { 0.433012694f, -0.250000208f, -1.0f }),
		addVertex(meshData, { 0.433012694f, -0.250000208f, 1.0f }),
		addVertex(meshData, { 0.250000029f, -0.433012902f, -1.0f }),
		addVertex(meshData, { 0.250000029f, -0.433012902f, 1.0f }),
		addVertex(meshData, { 0.00000006f, -0.500000178f, -1.0f }),
		addVertex(meshData, { 0.00000006f, -0.500000178f, 1.0f }),
		addVertex(meshData, { -0.249999910f, -0.433012962f, -1.0f }),
		addVertex(meshData, { -0.249999910f, -0.433012962f, 1.0f }),
		addVertex(meshData, { -0.433012634f, -0.250000357f, -1.0f }),
		addVertex(meshData, { -0.433012634f, -0.250000357f, 1.0f }),
		addVertex(meshData, { -0.5f, -0.000000421f, -1.0f }),
		addVertex(meshData, { -0.5f, -0.000000421f, 1.0f }),
		addVertex(meshData, { -0.433012872f, 0.249999567f, -1.0f }),
		addVertex(meshData, { -0.433012872f, 0.249999567f, 1.0f }),
		addVertex(meshData, { -0.250000327f, 0.433012336f, -1.0f }),
		addVertex(meshData, { -0.250000327f, 0.433012336f, 1.0f }),
		addVertex(meshData, { 0.0f, 1.0f, -1.0f }),
		addVertex(meshData, { 0.0f, 1.0f, 1.0f }),
		addVertex(meshData, { 0.5f, 0.866025388f, -1.0f }),
		addVertex(meshData, { 0.5f, 0.866025388f, 1.0f }),
		addVertex(meshData, { 0.866025447f, 0.499999970f, -1.0f }),
		addVertex(meshData, { 0.866025447f, 0.499999970f, 1.0f }),
		addVertex(meshData, { 1.0f, -0.000000043f, -1.0f }),
		addVertex(meshData, { 1.0f, -0.000000043f, 1.0f }),
		addVertex(meshData, { 0.866025388f, -0.500000059f, -1.0f }),
		addVertex(meshData, { 0.866025388f, -0.500000059f, 1.0f }),
		addVertex(meshData, { 0.500000059f, -0.866025388f, -1.0f }),
		addVertex(meshData, { 0.500000059f, -0.866025388f, 1.0f }),
		addVertex(meshData, { 0.00000015f, -1.0f, -1.0f }),
		addVertex(meshData, { 0.00000015f, -1.0f, 1.0f }),
		addVertex(meshData, { -0.499999791f, -0.866025507f, -1.0f }),
		addVertex(meshData, { -0.499999791f, -0.866025507f, 1.0f }),
		addVertex(meshData, { -0.866025209f, -0.500000298f, -1.0f }),
		addVertex(meshData, { -0.866025209f, -0.500000298f, 1.0f }),
		addVertex(meshData, { -1.0f, -0.000000464f, -1.0f }),
		addVertex(meshData, { -1.0f, -0.000000464f, 1.0f }),
		addVertex(meshData, { -0.866025686f, 0.499999493f, -1.0f }),
		addVertex(meshData, { -0.866025686f, 0.499999493f, 1.0f }),
		addVertex(meshData, { -0.500000596f, 0.866025090f, -1.0f }),
		addVertex(meshData, { -0.500000596f, 0.866025090f, 1.0f })
	};
	std::array<std::array<int, 4>, 48> faceIndices = {{
		{{ vertexIndices[0], vertexIndices[2], vertexIndices[3], vertexIndices[1] }},
		{{ vertexIndices[2], vertexIndices[4], vertexIndices[5], vertexIndices[3] }},
		{{ vertexIndices[4], vertexIndices[6], vertexIndices[7], vertexIndices[5] }},
		{{ vertexIndices[6], vertexIndices[8], vertexIndices[9], vertexIndices[7] }},
		{{ vertexIndices[8], vertexIndices[10], vertexIndices[11], vertexIndices[9] }},
		{{ vertexIndices[10], vertexIndices[12], vertexIndices[13], vertexIndices[11] }},
		{{ vertexIndices[12], vertexIndices[14], vertexIndices[15], vertexIndices[13] }},
		{{ vertexIndices[14], vertexIndices[16], vertexIndices[17], vertexIndices[15] }},
		{{ vertexIndices[16], vertexIndices[18], vertexIndices[19], vertexIndices[17] }},
		{{ vertexIndices[18], vertexIndices[20], vertexIndices[21], vertexIndices[19] }},
		{{ vertexIndices[20], vertexIndices[22], vertexIndices[23], vertexIndices[21] }},
		{{ vertexIndices[22], vertexIndices[0], vertexIndices[1], vertexIndices[23] }},
		{{ vertexIndices[24], vertexIndices[25], vertexIndices[27], vertexIndices[26] }},
		{{ vertexIndices[26], vertexIndices[27], vertexIndices[29], vertexIndices[28] }},
		{{ vertexIndices[28], vertexIndices[29], vertexIndices[31], vertexIndices[30] }},
		{{ vertexIndices[30], vertexIndices[31], vertexIndices[33], vertexIndices[32] }},
		{{ vertexIndices[32], vertexIndices[33], vertexIndices[35], vertexIndices[34] }},
		{{ vertexIndices[34], vertexIndices[35], vertexIndices[37], vertexIndices[36] }},
		{{ vertexIndices[36], vertexIndices[37], vertexIndices[39], vertexIndices[38] }},
		{{ vertexIndices[38], vertexIndices[39], vertexIndices[41], vertexIndices[40] }},
		{{ vertexIndices[40], vertexIndices[41], vertexIndices[43], vertexIndices[42] }},
		{{ vertexIndices[42], vertexIndices[43], vertexIndices[45], vertexIndices[44] }},
		{{ vertexIndices[44], vertexIndices[45], vertexIndices[47], vertexIndices[46] }},
		{{ vertexIndices[46], vertexIndices[47], vertexIndices[25], vertexIndices[24] }},
		{{ vertexIndices[13], vertexIndices[15], vertexIndices[39], vertexIndices[37] }},
		{{ vertexIndices[37], vertexIndices[35], vertexIndices[11], vertexIndices[13] }},
		{{ vertexIndices[35], vertexIndices[33], vertexIndices[9], vertexIndices[11] }},
		{{ vertexIndices[33], vertexIndices[31], vertexIndices[7], vertexIndices[9] }},
		{{ vertexIndices[31], vertexIndices[29], vertexIndices[5], vertexIndices[7] }},
		{{ vertexIndices[29], vertexIndices[27], vertexIndices[3], vertexIndices[5] }},
		{{ vertexIndices[27], vertexIndices[25], vertexIndices[1], vertexIndices[3] }},
		{{ vertexIndices[25], vertexIndices[47], vertexIndices[23], vertexIndices[1] }},
		{{ vertexIndices[47], vertexIndices[45], vertexIndices[21], vertexIndices[23] }},
		{{ vertexIndices[45], vertexIndices[43], vertexIndices[19], vertexIndices[21] }},
		{{ vertexIndices[43], vertexIndices[41], vertexIndices[17], vertexIndices[19] }},
		{{ vertexIndices[41], vertexIndices[39], vertexIndices[15], vertexIndices[17] }},
		{{ vertexIndices[20], vertexIndices[18], vertexIndices[42], vertexIndices[44] }},
		{{ vertexIndices[16], vertexIndices[40], vertexIndices[42], vertexIndices[18] }},
		{{ vertexIndices[14], vertexIndices[38], vertexIndices[40], vertexIndices[16] }},
		{{ vertexIndices[12], vertexIndices[36], vertexIndices[38], vertexIndices[14] }},
		{{ vertexIndices[10], vertexIndices[34], vertexIndices[36], vertexIndices[12] }},
		{{ vertexIndices[8], vertexIndices[32], vertexIndices[34], vertexIndices[10] }},
		{{ vertexIndices[6], vertexIndices[30], vertexIndices[32], vertexIndices[8] }},
		{{ vertexIndices[4], vertexIndices[28], vertexIndices[30], vertexIndices[6] }},
		{{ vertexIndices[2], vertexIndices[26], vertexIndices[28], vertexIndices[4] }},
		{{ vertexIndices[0], vertexIndices[24], vertexIndices[26], vertexIndices[2] }},
		{{ vertexIndices[22], vertexIndices[46], vertexIndices[24], vertexIndices[0] }},
		{{ vertexIndices[20], vertexIndices[44], vertexIndices[46], vertexIndices[22] }}
	}};
	for (const auto& face : faceIndices) {
		addFace(meshData, face.begin(), face.end());
	}

	return meshData;
}


std::vector<HalfEdgeMesh> createTestTube2()
{
	HalfEdgeMesh m0;
	std::array<int, 8> vertexIndices0 = {
		addVertex(m0, { -0.000000014f, 0.499999761f, -1.0f }),
		addVertex(m0, { -0.000000014f, 0.499999761f, 1.0f }),
		addVertex(m0, { 0.24999997f, 0.433012485f, -1.0f }),
		addVertex(m0, { 0.24999997f, 0.433012485f, 1.0f }),
		addVertex(m0, { 0.0f, 1.0f, -1.0f }),
		addVertex(m0, { 0.0f, 1.0f, 1.0f }),
		addVertex(m0, { 0.5f, 0.866025388f, -1.0f }),
		addVertex(m0, { 0.5f, 0.866025388f, 1.0f })
	};
	std::array<std::array<int, 4>, 6> faceIndices0 = {{
		{{ vertexIndices0[0], vertexIndices0[2], vertexIndices0[3], vertexIndices0[1] }},
		{{ vertexIndices0[5], vertexIndices0[7], vertexIndices0[6], vertexIndices0[4] }},
		{{ vertexIndices0[0], vertexIndices0[1], vertexIndices0[5], vertexIndices0[4] }},
		{{ vertexIndices0[3], vertexIndices0[7], vertexIndices0[5], vertexIndices0[1] }},
		{{ vertexIndices0[2], vertexIndices0[0], vertexIndices0[4], vertexIndices0[6] }},
		{{ vertexIndices0[2], vertexIndices0[6], vertexIndices0[7], vertexIndices0[3] }}
	}};
	for (const auto& face : faceIndices0) {
		addFace(m0, face.begin(), face.end());
	}

	HalfEdgeMesh m1;
	std::array<int, 8> vertexIndices1 = {
		addVertex(m1, { 0.249999970f, 0.433012455f, -1.0f }),
		addVertex(m1, { 0.249999970f, 0.433012455f, 1.0f }),
		addVertex(m1, { 0.433012664f, 0.249999776f, -1.0f }),
		addVertex(m1, { 0.433012664f, 0.249999776f, 1.0f }),
		addVertex(m1, { 0.500000119f, 0.866025328f, -1.0f }),
		addVertex(m1, { 0.500000119f, 0.866025328f, 1.0f }),
		addVertex(m1, { 0.866025507f, 0.49999991f, -1.0f }),
		addVertex(m1, { 0.866025507f, 0.49999991f, 1.0f })
	};
	std::array<std::array<int, 4>, 6> faceIndices1 = {{
		{{ vertexIndices1[5], vertexIndices1[7], vertexIndices1[6], vertexIndices1[4] }},
		{{ vertexIndices1[0], vertexIndices1[2], vertexIndices1[3], vertexIndices1[1] }},
		{{ vertexIndices1[0], vertexIndices1[1], vertexIndices1[5], vertexIndices1[4] }},
		{{ vertexIndices1[3], vertexIndices1[7], vertexIndices1[5], vertexIndices1[1] }},
		{{ vertexIndices1[2], vertexIndices1[0], vertexIndices1[4], vertexIndices1[6] }},
		{{ vertexIndices1[2], vertexIndices1[6], vertexIndices1[7], vertexIndices1[3] }}
	}};
	for (const auto& face : faceIndices1) {
		addFace(m1, face.begin(), face.end());
	}

	HalfEdgeMesh m2;
	std::array<int, 8> vertexIndices2 = {
		addVertex(m2, { 0.433012694f, 0.249999791f, -1.0f }),
		addVertex(m2, { 0.433012694f, 0.249999791f, 1.0f }),
		addVertex(m2, { 0.5f, -0.000000209f, -1.0f }),
		addVertex(m2, { 0.5f, -0.000000209f, 1.0f }),
		addVertex(m2, { 0.866025447f, 0.49999997f, -1.0f }),
		addVertex(m2, { 0.866025447f, 0.49999997f, 1.0f }),
		addVertex(m2, { 1.0f, -0.000000043f, -1.0f }),
		addVertex(m2, { 1.0f, -0.000000043f, 1.0f })
	};
	std::array<std::array<int, 4>, 6> faceIndices2 = {{
		{{ vertexIndices2[0], vertexIndices2[2], vertexIndices2[3], vertexIndices2[1] }},
		{{ vertexIndices2[7], vertexIndices2[6], vertexIndices2[4], vertexIndices2[5] }},
		{{ vertexIndices2[0], vertexIndices2[1], vertexIndices2[5], vertexIndices2[4] }},
		{{ vertexIndices2[3], vertexIndices2[7], vertexIndices2[5], vertexIndices2[1] }},
		{{ vertexIndices2[4], vertexIndices2[6], vertexIndices2[2], vertexIndices2[0] }},
		{{ vertexIndices2[2], vertexIndices2[6], vertexIndices2[7], vertexIndices2[3] }}
	}};
	for (const auto& face : faceIndices2) {
		addFace(m2, face.begin(), face.end());
	}

	HalfEdgeMesh m3;
	std::array<int, 8> vertexIndices3 = {
		addVertex(m3, { 0.499999970f, -0.000000238f, -1.0f }),
		addVertex(m3, { 0.499999970f, -0.000000238f, 1.0f }),
		addVertex(m3, { 0.433012723f, -0.250000238f, -1.0f }),
		addVertex(m3, { 0.433012723f, -0.250000238f, 1.0f }),
		addVertex(m3, { 1.000000119f, -0.000000149f, -1.0f }),
		addVertex(m3, { 1.000000119f, -0.000000149f, 1.0f }),
		addVertex(m3, { 0.866025388f, -0.500000059f, -1.0f }),
		addVertex(m3, { 0.866025388f, -0.500000059f, 1.0f })
	};
	std::array<std::array<int, 4>, 6> faceIndices3 = {{
		{{ vertexIndices3[7], vertexIndices3[6], vertexIndices3[4], vertexIndices3[5] }},
		{{ vertexIndices3[0], vertexIndices3[2], vertexIndices3[3], vertexIndices3[1] }},
		{{ vertexIndices3[0], vertexIndices3[1], vertexIndices3[5], vertexIndices3[4] }},
		{{ vertexIndices3[3], vertexIndices3[7], vertexIndices3[5], vertexIndices3[1] }},
		{{ vertexIndices3[4], vertexIndices3[6], vertexIndices3[2], vertexIndices3[0] }},
		{{ vertexIndices3[2], vertexIndices3[6], vertexIndices3[7], vertexIndices3[3] }}
	}};
	for (const auto& face : faceIndices3) {
		addFace(m3, face.begin(), face.end());
	}

	HalfEdgeMesh m4;
	std::array<int, 8> vertexIndices4 = {
		addVertex(m4, { 0.433012694f, -0.250000208f, -1.0f }),
		addVertex(m4, { 0.433012694f, -0.250000208f, 1.0f }),
		addVertex(m4, { 0.250000029f, -0.433012902f, -1.0f }),
		addVertex(m4, { 0.250000029f, -0.433012902f, 1.0f }),
		addVertex(m4, { 0.866025388f, -0.500000059f, -1.0f }),
		addVertex(m4, { 0.866025388f, -0.500000059f, 1.0f }),
		addVertex(m4, { 0.500000059f, -0.866025388f, -1.0f }),
		addVertex(m4, { 0.500000059f, -0.866025388f, 1.0f })
	};
	std::array<std::array<int, 4>, 6> faceIndices4 = {{
		{{ vertexIndices4[0], vertexIndices4[2], vertexIndices4[3], vertexIndices4[1] }},
		{{ vertexIndices4[2], vertexIndices4[6], vertexIndices4[7], vertexIndices4[3] }},
		{{ vertexIndices4[5], vertexIndices4[7], vertexIndices4[6], vertexIndices4[4] }},
		{{ vertexIndices4[3], vertexIndices4[7], vertexIndices4[5], vertexIndices4[1] }},
		{{ vertexIndices4[0], vertexIndices4[1], vertexIndices4[5], vertexIndices4[4] }},
		{{ vertexIndices4[4], vertexIndices4[6], vertexIndices4[2], vertexIndices4[0] }}
	}};
	for (const auto& face : faceIndices4) {
		addFace(m4, face.begin(), face.end());
	}

	HalfEdgeMesh m5;
	std::array<int, 8> vertexIndices5 = {
		addVertex(m5, { 0.24999997f, -0.433012902f, -1.0f }),
		addVertex(m5, { 0.24999997f, -0.433012902f, 1.0f }),
		addVertex(m5, { 0.0f, -0.500000238f, -1.0f }),
		addVertex(m5, { 0.0f, -0.500000238f, 1.0f }),
		addVertex(m5, { 0.5f, -0.866025567f, -1.0f }),
		addVertex(m5, { 0.5f, -0.866025567f, 1.0f }),
		addVertex(m5, { 0.00000017f, -1.0f, -1.0f }),
		addVertex(m5, { 0.00000017f, -1.0f, 1.0f })
	};
	std::array<std::array<int, 4>, 6> faceIndices5 = {{
		{{ vertexIndices5[5], vertexIndices5[7], vertexIndices5[6], vertexIndices5[4] }},
		{{ vertexIndices5[2], vertexIndices5[6], vertexIndices5[7], vertexIndices5[3] }},
		{{ vertexIndices5[0], vertexIndices5[2], vertexIndices5[3], vertexIndices5[1] }},
		{{ vertexIndices5[3], vertexIndices5[7], vertexIndices5[5], vertexIndices5[1] }},
		{{ vertexIndices5[0], vertexIndices5[1], vertexIndices5[5], vertexIndices5[4] }},
		{{ vertexIndices5[4], vertexIndices5[6], vertexIndices5[2], vertexIndices5[0] }}
	}};
	for (const auto& face : faceIndices5) {
		addFace(m5, face.begin(), face.end());
	}

	HalfEdgeMesh m6;
	std::array<int, 8> vertexIndices6 = {
		addVertex(m6, { 0.000000059f, -0.500000178f, -1.0f }),
		addVertex(m6, { 0.000000059f, -0.500000178f, 1.0f }),
		addVertex(m6, { -0.249999910f, -0.433012962f, -1.0f }),
		addVertex(m6, { -0.249999910f, -0.433012962f, 1.0f }),
		addVertex(m6, { 0.000000150f, -1.0f, -1.0f }),
		addVertex(m6, { 0.000000150f, -1.0f, 1.0f }),
		addVertex(m6, { -0.499999791f, -0.866025507f, -1.0f }),
		addVertex(m6, { -0.499999791f, -0.866025507f, 1.0f })
	};
	std::array<std::array<int, 4>, 6> faceIndices6 = {{
		{{ vertexIndices6[5], vertexIndices6[7], vertexIndices6[6], vertexIndices6[4] }},
		{{ vertexIndices6[0], vertexIndices6[2], vertexIndices6[3], vertexIndices6[1] }},
		{{ vertexIndices6[3], vertexIndices6[7], vertexIndices6[5], vertexIndices6[1] }},
		{{ vertexIndices6[2], vertexIndices6[6], vertexIndices6[7], vertexIndices6[3] }},
		{{ vertexIndices6[0], vertexIndices6[1], vertexIndices6[5], vertexIndices6[4] }},
		{{ vertexIndices6[4], vertexIndices6[6], vertexIndices6[2], vertexIndices6[0] }}
	}};
	for (const auto& face : faceIndices6) {
		addFace(m6, face.begin(), face.end());
	}

	HalfEdgeMesh m7;
	std::array<int, 8> vertexIndices7 = {
		addVertex(m7, { -0.249999925f, -0.433012932f, -1.0f }),
		addVertex(m7, { -0.249999925f, -0.433012932f, 1.0f }),
		addVertex(m7, { -0.433012634f, -0.250000298f, -1.0f }),
		addVertex(m7, { -0.433012634f, -0.250000298f, 1.0f }),
		addVertex(m7, { -0.499999761f, -0.866025567f, -1.0f }),
		addVertex(m7, { -0.499999761f, -0.866025567f, 1.0f }),
		addVertex(m7, { -0.866025149f, -0.500000238f, -1.0f }),
		addVertex(m7, { -0.866025149f, -0.500000238f, 1.0f })
	};
	std::array<std::array<int, 4>, 6> faceIndices7 = {{
		{{ vertexIndices7[5], vertexIndices7[7], vertexIndices7[6], vertexIndices7[4] }},
		{{ vertexIndices7[0], vertexIndices7[2], vertexIndices7[3], vertexIndices7[1] }},
		{{ vertexIndices7[3], vertexIndices7[7], vertexIndices7[5], vertexIndices7[1] }},
		{{ vertexIndices7[2], vertexIndices7[6], vertexIndices7[7], vertexIndices7[3] }},
		{{ vertexIndices7[0], vertexIndices7[1], vertexIndices7[5], vertexIndices7[4] }},
		{{ vertexIndices7[4], vertexIndices7[6], vertexIndices7[2], vertexIndices7[0] }}
	}};
	for (const auto& face : faceIndices7) {
		addFace(m7, face.begin(), face.end());
	}

	HalfEdgeMesh m8;
	std::array<int, 8> vertexIndices8 = {
		addVertex(m8, { -0.433012634f, -0.250000357f, -1.0f }),
		addVertex(m8, { -0.433012634f, -0.250000357f, 1.0f }),
		addVertex(m8, { -0.5f, -0.000000421f, -1.0f }),
		addVertex(m8, { -0.5f, -0.000000421f, 1.0f }),
		addVertex(m8, { -0.866025209f, -0.500000298f, -1.0f }),
		addVertex(m8, { -0.866025209f, -0.500000298f, 1.0f }),
		addVertex(m8, { -1.0f, -0.000000464f, -1.0f }),
		addVertex(m8, { -1.0f, -0.000000464f, 1.0f })
	};
	std::array<std::array<int, 4>, 6> faceIndices8 = {{
		{{ vertexIndices8[0], vertexIndices8[2], vertexIndices8[3], vertexIndices8[1] }},
		{{ vertexIndices8[5], vertexIndices8[7], vertexIndices8[6], vertexIndices8[4] }},
		{{ vertexIndices8[3], vertexIndices8[7], vertexIndices8[5], vertexIndices8[1] }},
		{{ vertexIndices8[2], vertexIndices8[6], vertexIndices8[7], vertexIndices8[3] }},
		{{ vertexIndices8[0], vertexIndices8[1], vertexIndices8[5], vertexIndices8[4] }},
		{{ vertexIndices8[4], vertexIndices8[6], vertexIndices8[2], vertexIndices8[0] }}
	}};
	for (const auto& face : faceIndices8) {
		addFace(m8, face.begin(), face.end());
	}

	HalfEdgeMesh m9;
	std::array<int, 8> vertexIndices9 = {
		addVertex(m9, { -0.500000059f, -0.0000004f, -1.0f }),
		addVertex(m9, { -0.500000059f, -0.0000004f, 1.0f }),
		addVertex(m9, { -0.433012783f, 0.249999582f, -1.0f }),
		addVertex(m9, { -0.433012783f, 0.249999582f, 1.0f }),
		addVertex(m9, { -0.999999821f, -0.000000387f, -1.0f }),
		addVertex(m9, { -0.999999821f, -0.000000387f, 1.0f }),
		addVertex(m9, { -0.866025507f, 0.499999523f, -1.0f }),
		addVertex(m9, { -0.866025507f, 0.499999523f, 1.0f })
	};
	std::array<std::array<int, 4>, 6> faceIndices9 = {{
		{{ vertexIndices9[5], vertexIndices9[7], vertexIndices9[6], vertexIndices9[4] }},
		{{ vertexIndices9[0], vertexIndices9[2], vertexIndices9[3], vertexIndices9[1] }},
		{{ vertexIndices9[3], vertexIndices9[7], vertexIndices9[5], vertexIndices9[1] }},
		{{ vertexIndices9[2], vertexIndices9[6], vertexIndices9[7], vertexIndices9[3] }},
		{{ vertexIndices9[0], vertexIndices9[1], vertexIndices9[5], vertexIndices9[4] }},
		{{ vertexIndices9[4], vertexIndices9[6], vertexIndices9[2], vertexIndices9[0] }}
	}};
	for (const auto& face : faceIndices9) {
		addFace(m9, face.begin(), face.end());
	}

	HalfEdgeMesh m10;
	std::array<int, 8> vertexIndices10 = {
		addVertex(m10, { -0.433012872f, 0.249999567f, -1.0f }),
		addVertex(m10, { -0.433012872f, 0.249999567f, 1.0f }),
		addVertex(m10, { -0.250000327f, 0.433012336f, -1.0f }),
		addVertex(m10, { -0.250000327f, 0.433012336f, 1.0f }),
		addVertex(m10, { -0.866025686f, 0.499999493f, -1.0f }),
		addVertex(m10, { -0.866025686f, 0.499999493f, 1.0f }),
		addVertex(m10, { -0.500000596f, 0.866025090f, -1.0f }),
		addVertex(m10, { -0.500000596f, 0.866025090f, 1.0f })
	};
	std::array<std::array<int, 4>, 6> faceIndices10 = {{
		{{ vertexIndices10[0], vertexIndices10[2], vertexIndices10[3], vertexIndices10[1] }},
		{{ vertexIndices10[5], vertexIndices10[7], vertexIndices10[6], vertexIndices10[4] }},
		{{ vertexIndices10[3], vertexIndices10[7], vertexIndices10[5], vertexIndices10[1] }},
		{{ vertexIndices10[2], vertexIndices10[6], vertexIndices10[7], vertexIndices10[3] }},
		{{ vertexIndices10[0], vertexIndices10[1], vertexIndices10[5], vertexIndices10[4] }},
		{{ vertexIndices10[4], vertexIndices10[6], vertexIndices10[2], vertexIndices10[0] }}
	}};
	for (const auto& face : faceIndices10) {
		addFace(m10, face.begin(), face.end());
	}

	HalfEdgeMesh m11;
	std::array<int, 8> vertexIndices11 = {
		addVertex(m11, { -0.250000268f, 0.433012396f, -1.0f }),
		addVertex(m11, { -0.250000268f, 0.433012396f, 1.0f }),
		addVertex(m11, { -0.000000357f, 0.499999791f, -1.0f }),
		addVertex(m11, { -0.000000357f, 0.499999791f, 1.0f }),
		addVertex(m11, { -0.500000417f, 0.866024971f, -1.0f }),
		addVertex(m11, { -0.500000417f, 0.866024971f, 1.0f }),
		addVertex(m11, { -0.000000536f, 1.0f, -1.0f }),
		addVertex(m11, { -0.000000536f, 1.0f, 1.0f })
	};
	std::array<std::array<int, 4>, 6> faceIndices11 = {{
		{{ vertexIndices11[5], vertexIndices11[7], vertexIndices11[6], vertexIndices11[4] }},
		{{ vertexIndices11[0], vertexIndices11[2], vertexIndices11[3], vertexIndices11[1] }},
		{{ vertexIndices11[3], vertexIndices11[7], vertexIndices11[5], vertexIndices11[1] }},
		{{ vertexIndices11[2], vertexIndices11[6], vertexIndices11[7], vertexIndices11[3] }},
		{{ vertexIndices11[0], vertexIndices11[1], vertexIndices11[5], vertexIndices11[4] }},
		{{ vertexIndices11[4], vertexIndices11[6], vertexIndices11[2], vertexIndices11[0] }}
	}};
	for (const auto& face : faceIndices11) {
		addFace(m11, face.begin(), face.end());
	}

	return { m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11 };
}


std::pair<std::vector<glm::vec3>, std::vector<unsigned short>> createTestTriangleMesh()
{
	return {
		{
			{ 0.615450382f, 0.760466337f, -0.866529464f },
			{ 0.807382106f, -0.887271940f, 0.685045242f },
			{ -0.385906398f, -0.833566009f, 1.067358016f },
			{ -1.010702490f, -0.736890316f, -0.437148213f },
			{ -0.034836068f, 1.200925946f, -0.826681196f },
			{ -1.140288949f, 0.928207159f, 0.294779688f },
			{ 0.967018067f, -0.225285902f, 0.718770503f },
			{ 0.816902518f, 0.153803467f, -0.838943421f },
			{ -1.568295598f, 0.090358749f, 0.006510018f },
			{ -0.125880688f, -1.354099631f, -0.057367064f },
			{ -0.650661885f, 0.648258745f, 0.964299917f },
			{ 0.171862334f, -0.085280865f, 0.626737713f },
			{ -1.175329923f, -1.196049213f, 0.224538952f },
			{ 0.896988928f, -0.878769040f, 0.018664546f },
			{ 0.050400324f, -1.053213119f, 0.880978882f },
			{ -0.108820162f, -0.884682297f, -0.771282374f },
			{ -0.176819637f, -0.308181762f, 1.263301849f },
			{ -0.222157642f, 1.022119164f, 0.925747632f },
			{ 0.707345724f, 1.121148347f, -0.237675473f },
			{ -0.174175828f, 1.568309783f, -0.071535445f },
			{ 0.828270316f, 0.935747563f, -0.078342802f },
			{ 1.209428787f, 0.122468627f, -0.003602489f },
			{ -1.175907135f, 0.038690738f, -0.654182493f },
			{ 0.017661752f, -0.138503223f, -1.216818571f },
			{ -0.663747549f, -1.456306695f, 0.084276482f },
			{ 0.328168809f, -1.017542481f, -0.023794194f },
			{ -0.142584607f, -1.252702951f, 0.640521585f },
			{ 0.200249865f, -0.072437934f, 0.912739932f },
			{ 0.519573390f, -0.271621108f, 0.736040890f },
			{ -1.507794857f, -0.612749576f, 0.227235898f },
			{ -1.558209896f, 0.007886608f, -0.388035446f },
			{ 0.147921025f, 0.426297962f, -1.191849231f },
			{ -0.857593059f, 0.789759397f, -0.587218880f },
			{ 0.512674510f, -0.740344703f, -0.531411349f },
			{ -0.318242907f, 1.267057061f, 0.564987242f },
			{ 0.053215581f, 0.763675630f, 0.238783344f },
			{ 1.186141967f, -0.346210807f, 0.077312335f }
		},
		{
			16, 2, 14,		26, 2, 12,		6, 13, 36,		31, 22, 32,
			31, 7, 23,		11, 27, 28,		17, 10, 27,		28, 1, 6,
			25, 26, 9,		1, 14, 13,		33, 25, 15,		24, 3, 15,
			2, 29, 12,		3, 30, 22,		12, 29, 30,		22, 30, 32,
			0, 31, 4,		11, 21, 35,		0, 20, 7,		5, 34, 19,
			10, 34, 5,		32, 19, 4,		4, 18, 0,		35, 20, 18,
			34, 18, 19,		17, 35, 34,		6, 36, 21,		23, 33, 15,
			29, 10, 5,		9, 26, 24,		27, 16, 14,		26, 14, 2,
			6, 1, 13,		33, 36, 13,		21, 28, 6,		31, 23, 22,
			30, 8, 5,		30, 29, 8,		27, 10, 16,		11, 17, 27,
			28, 14, 1,		28, 27, 14,		25, 14, 26,		13, 14, 25,
			15, 25, 9,		33, 13, 25,		24, 12, 3,		9, 24, 15,
			2, 16, 29,		3, 12, 30,		32, 30, 5,		0, 7, 31,
			4, 31, 32,		22, 15, 3,		22, 23, 15,		35, 21, 20,
			11, 28, 21,		7, 20, 21,		0, 18, 20,		7, 21, 33,
			21, 36, 33,		10, 17, 34,		32, 5, 19,		4, 19, 18,
			34, 35, 18,		17, 11, 35,		23, 7, 33,		8, 29, 5,
			29, 16, 10,		24, 26, 12
		}
	};
}
