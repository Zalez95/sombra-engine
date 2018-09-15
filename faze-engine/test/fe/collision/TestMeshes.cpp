#include "TestMeshes.h"

fe::collision::HalfEdgeMesh createTestPolyhedron1()
{
	fe::collision::HalfEdgeMesh meshData;
	fe::collision::addVertex(meshData, {  1.0f,  1.0f, -1.0f });
	fe::collision::addVertex(meshData, {  1.0f, -1.0f, -1.0f });
	fe::collision::addVertex(meshData, { -1.0f, -1.0f, -1.0f });
	fe::collision::addVertex(meshData, { -1.0f,  1.0f, -1.0f });
	fe::collision::addVertex(meshData, {  1.0f,  1.0f,  1.0f });
	fe::collision::addVertex(meshData, {  0.0f, -1.0f,  1.0f });
	fe::collision::addVertex(meshData, { -1.0f, -1.0f,  1.0f });
	fe::collision::addVertex(meshData, { -1.0f,  1.0f,  1.0f });
	fe::collision::addFace(meshData, { 3, 2, 6, 7 });
	fe::collision::addFace(meshData, { 2, 1, 5, 6 });
	fe::collision::addFace(meshData, { 1, 4, 5 });
	fe::collision::addFace(meshData, { 1, 0, 4 });
	fe::collision::addFace(meshData, { 0, 3, 7, 4 });
	fe::collision::addFace(meshData, { 4, 7, 6, 5 });
	fe::collision::addFace(meshData, { 0, 1, 2, 3 });
	return meshData;
}


fe::collision::HalfEdgeMesh createTestPolyhedron2()
{
	fe::collision::HalfEdgeMesh meshData;
	fe::collision::addVertex(meshData, { 0.0f, 0.0f, 0.0f });
	fe::collision::addVertex(meshData, { 1.0f, 0.0f, 0.0f });
	fe::collision::addVertex(meshData, { 1.0f, 1.0f, 0.0f });
	fe::collision::addVertex(meshData, { 0.0f, 1.0f, 0.0f });
	fe::collision::addVertex(meshData, { 0.5f, 0.5f, 1.6f });
	fe::collision::addFace(meshData, { 3, 2, 1, 0 });
	fe::collision::addFace(meshData, { 0, 1, 4 });
	fe::collision::addFace(meshData, { 1, 2, 4 });
	fe::collision::addFace(meshData, { 2, 3, 4 });
	fe::collision::addFace(meshData, { 3, 0, 4 });
	return meshData;
}


fe::collision::HalfEdgeMesh createTestPolyhedron3()
{
	fe::collision::HalfEdgeMesh meshData;
	fe::collision::addVertex(meshData, { -1.0f, -0.5f,  0.5f });
	fe::collision::addVertex(meshData, { -1.0f, -0.5f, -0.5f });
	fe::collision::addVertex(meshData, { -1.0f,  0.5f, -0.5f });
	fe::collision::addVertex(meshData, { -1.0f,  0.5f,  0.5f });
	fe::collision::addVertex(meshData, {  1.0f,  0.0f,  0.0f });
	fe::collision::addFace(meshData, { 3, 2, 1, 0 });
	fe::collision::addFace(meshData, { 0, 1, 4 });
	fe::collision::addFace(meshData, { 1, 2, 4 });
	fe::collision::addFace(meshData, { 2, 3, 4 });
	fe::collision::addFace(meshData, { 3, 0, 4 });
	return meshData;
}


fe::collision::HalfEdgeMesh createTestMesh1()
{
	fe::collision::HalfEdgeMesh meshData;
	fe::collision::addVertex(meshData, {  1.25f,  1.0f, -2.75f });
	fe::collision::addVertex(meshData, {  1.25f, -1.0f, -2.75f });
	fe::collision::addVertex(meshData, { -0.25f, -1.0f, -2.75f });
	fe::collision::addVertex(meshData, { -0.25f,  1.0f,  0.0f  });
	fe::collision::addVertex(meshData, {  1.25f,  1.0f,  2.75f });
	fe::collision::addVertex(meshData, {  1.25f, -1.0f,  2.75f });
	fe::collision::addVertex(meshData, { -0.25f, -1.0f,  0.0f  });
	fe::collision::addVertex(meshData, { -0.25f,  1.0f,  2.75f });
	fe::collision::addVertex(meshData, {  0.25f,  0.0f,  0.0f  });
	fe::collision::addFace(meshData, { 0, 1, 2 });
	fe::collision::addFace(meshData, { 7, 6, 5 });
	fe::collision::addFace(meshData, { 1, 5, 6 });
	fe::collision::addFace(meshData, { 6, 7, 3 });
	fe::collision::addFace(meshData, { 4, 0, 3 });
	fe::collision::addFace(meshData, { 0, 4, 8 });
	fe::collision::addFace(meshData, { 1, 0, 8 });
	fe::collision::addFace(meshData, { 4, 5, 8 });
	fe::collision::addFace(meshData, { 5, 1, 8 });
	fe::collision::addFace(meshData, { 3, 0, 2 });
	fe::collision::addFace(meshData, { 4, 7, 5 });
	fe::collision::addFace(meshData, { 2, 1, 6 });
	fe::collision::addFace(meshData, { 2, 6, 3 });
	fe::collision::addFace(meshData, { 7, 4, 3 });
	return meshData;
}


fe::collision::HalfEdgeMesh createTestMesh2()
{
	fe::collision::HalfEdgeMesh meshData;
	fe::collision::addVertex(meshData, { 1.25f,  1.0f, -2.75f });
	fe::collision::addVertex(meshData, { 1.25f, -1.0f, -2.75f });
	fe::collision::addVertex(meshData, { -0.25f, -1.0f, -2.75f });
	fe::collision::addVertex(meshData, { -0.25f,  1.0f,  0.0f });
	fe::collision::addVertex(meshData, { 1.25f,  1.0f,  2.75f });
	fe::collision::addVertex(meshData, { 1.25f, -1.0f,  2.75f });
	fe::collision::addVertex(meshData, { -0.25f, -1.0f,  0.0f });
	fe::collision::addVertex(meshData, { -0.25f,  1.0f,  2.75f });
	fe::collision::addFace(meshData, { 0, 1, 2 });
	fe::collision::addFace(meshData, { 0, 2, 3 });
	fe::collision::addFace(meshData, { 3, 2, 6, 7 });
	fe::collision::addFace(meshData, { 7, 6, 5 });
	fe::collision::addFace(meshData, { 7, 5, 4 });
	fe::collision::addFace(meshData, { 2, 1, 5, 6 });
	fe::collision::addFace(meshData, { 1, 0, 4, 5 });
	fe::collision::addFace(meshData, { 0, 3, 7, 4 });
	return meshData;
}


std::pair<fe::collision::HalfEdgeMesh, std::map<int, glm::vec3>> createTestMesh3()
{
	fe::collision::HalfEdgeMesh meshData;
	std::map<int, glm::vec3> normals;

	int iFace;
	fe::collision::addVertex(meshData, { -3.208401441f,  2.893295764f,  0.028006464f });
	fe::collision::addVertex(meshData, { -6.086990833f,  3.260166883f, -0.342617660f });
	fe::collision::addVertex(meshData, { -5.035281181f,  2.496228456f,  2.278198242f });
	fe::collision::addVertex(meshData, { -7.160157680f,  4.497337341f,  2.081477642f });
	fe::collision::addVertex(meshData, { -2.502496957f,  3.903687715f,  2.681146383f });
	fe::collision::addVertex(meshData, { -4.944808959f,  4.895039081f,  3.950236082f });
	fe::collision::addVertex(meshData, { -5.245877265f,  4.056465148f, -1.327844977f });
	fe::collision::addVertex(meshData, { -3.553897380f,  3.840824604f, -1.109999060f });
	fe::collision::addVertex(meshData, { -4.660541534f,  2.736007690f, -0.394804030f });
	fe::collision::addVertex(meshData, { -2.251753091f,  4.074582576f, -0.011565566f });
	fe::collision::addVertex(meshData, { -6.681442260f,  4.639166355f, -0.581894993f });
	fe::collision::addVertex(meshData, { -2.553625583f,  3.114257574f,  1.382524967f });
	fe::collision::addVertex(meshData, { -1.836821079f,  4.668467521f,  1.547912478f });
	fe::collision::addVertex(meshData, { -5.734357833f,  2.502610445f,  0.927823066f });
	fe::collision::addVertex(meshData, { -4.042359828f,  2.286959648f,  1.145670175f });
	fe::collision::addVertex(meshData, { -7.312233448f,  5.366353988f,  0.842946588f });
	fe::collision::addVertex(meshData, { -6.983338832f,  3.678830623f,  0.812192678f });
	fe::collision::addVertex(meshData, { -3.627435207f,  2.880870103f,  2.705149173f });
	fe::collision::addVertex(meshData, { -6.365145683f,  3.229807853f,  2.352669477f });
	fe::collision::addVertex(meshData, { -5.062996387f,  3.463579893f,  3.451099872f });
	fe::collision::addVertex(meshData, { -3.574266433f,  4.290853500f,  3.687945365f });
	fe::collision::addVertex(meshData, { -6.311958789f,  4.639792919f,  3.335471153f });
	iFace = fe::collision::addFace(meshData, { 6, 3, 4, });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);
	iFace = fe::collision::addFace(meshData, { 0, 7, 9 });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);
	iFace = fe::collision::addFace(meshData, { 0, 9, 11 });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);
	iFace = fe::collision::addFace(meshData, { 1, 8, 13 });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);
	iFace = fe::collision::addFace(meshData, { 0, 11, 14 });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);
	iFace = fe::collision::addFace(meshData, { 1, 13, 16 });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);
	iFace = fe::collision::addFace(meshData, { 2, 17, 19 });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);
	iFace = fe::collision::addFace(meshData, { 3, 18, 21 });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);
	iFace = fe::collision::addFace(meshData, { 10, 15, 3 });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);
	iFace = fe::collision::addFace(meshData, { 4, 12, 9 });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);
	iFace = fe::collision::addFace(meshData, { 6, 10, 3 });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);
	iFace = fe::collision::addFace(meshData, { 9, 7, 4 });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);
	iFace = fe::collision::addFace(meshData, { 21, 5, 20 });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);
	iFace = fe::collision::addFace(meshData, { 7, 6, 4 });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);
	iFace = fe::collision::addFace(meshData, { 20, 4, 3 });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);
	iFace = fe::collision::addFace(meshData, { 21, 19, 5 });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);
	iFace = fe::collision::addFace(meshData, { 21, 18, 19 });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);
	iFace = fe::collision::addFace(meshData, { 18, 2, 19 });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);
	iFace = fe::collision::addFace(meshData, { 19, 20, 5 });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);
	iFace = fe::collision::addFace(meshData, { 19, 17, 20 });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);
	iFace = fe::collision::addFace(meshData, { 17, 4, 20 });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);
	iFace = fe::collision::addFace(meshData, { 3, 21, 20 });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);
	iFace = fe::collision::addFace(meshData, { 16, 18, 3 });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);
	iFace = fe::collision::addFace(meshData, { 16, 13, 18 });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);
	iFace = fe::collision::addFace(meshData, { 13, 2, 18 });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);
	iFace = fe::collision::addFace(meshData, { 14, 17, 2 });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);
	iFace = fe::collision::addFace(meshData, { 14, 11, 17 });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);
	iFace = fe::collision::addFace(meshData, { 11, 4, 17 });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);
	iFace = fe::collision::addFace(meshData, { 15, 16, 3 });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);
	iFace = fe::collision::addFace(meshData, { 15, 10, 16 });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);
	iFace = fe::collision::addFace(meshData, { 10, 1, 16 });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);
	iFace = fe::collision::addFace(meshData, { 13, 14, 2 });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);
	iFace = fe::collision::addFace(meshData, { 13, 8, 14 });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);
	iFace = fe::collision::addFace(meshData, { 8, 0, 14 });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);
	iFace = fe::collision::addFace(meshData, { 11, 12, 4 });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);
	iFace = fe::collision::addFace(meshData, { 11, 9, 12 });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);
	iFace = fe::collision::addFace(meshData, { 6, 1, 10 });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);
	iFace = fe::collision::addFace(meshData, { 6, 8, 1 });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);
	iFace = fe::collision::addFace(meshData, { 6, 7, 8 });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);
	iFace = fe::collision::addFace(meshData, { 7, 0, 8 });
	normals[iFace] = fe::collision::calculateFaceNormal(meshData, iFace);

	return std::make_pair(meshData, normals);
}


fe::collision::HalfEdgeMesh createTestPlane1()
{
	fe::collision::HalfEdgeMesh meshData;
	fe::collision::addVertex(meshData, { -17.2071228f, -10.5431643f, 5.25000095f });
	fe::collision::addVertex(meshData, { -17.2071228f, -8.54316425f, 2.31236959f });
	fe::collision::addVertex(meshData, { -17.2071228f, -10.5431643f, 2.31236959f });
	fe::collision::addVertex(meshData, { -17.2071228f, -10.5431643f, 7.00000095f });
	fe::collision::addVertex(meshData, { -17.2071228f, -8.54316425f, 5.25000095f });
	fe::collision::addVertex(meshData, { -17.2071228f, -8.54316425f, 7.00000095f });
	fe::collision::addFace(meshData, { 0, 1, 2 });
	fe::collision::addFace(meshData, { 3, 4, 0 });
	fe::collision::addFace(meshData, { 0, 4, 1 });
	fe::collision::addFace(meshData, { 3, 5, 4 });
	return meshData;
}


fe::collision::HalfEdgeMesh createTestPlane2()
{
	fe::collision::HalfEdgeMesh meshData;
	fe::collision::addVertex(meshData, { -17.2071228f, -8.54316425f, 2.31236959f });
	fe::collision::addVertex(meshData, { -17.2071228f, -10.5431643f, 2.31236959f });
	fe::collision::addVertex(meshData, { -17.2071228f, -10.5431643f, 7.00000095f });
	fe::collision::addVertex(meshData, { -17.2071228f, -8.54316425f, 7.00000095f });
	fe::collision::addFace(meshData, { 0, 1, 3, 2 });
	return meshData;
}


fe::collision::HalfEdgeMesh createTestTube1()
{
	fe::collision::HalfEdgeMesh meshData;
	fe::collision::addVertex(meshData, { -0.000000014f, 0.499999761f, -1.0f });
	fe::collision::addVertex(meshData, { -0.000000014f, 0.499999761f, 1.0f });
	fe::collision::addVertex(meshData, { 0.249999970f, 0.433012485f, -1.0f });
	fe::collision::addVertex(meshData, { 0.249999970f, 0.433012485f, 1.0f });
	fe::collision::addVertex(meshData, { 0.433012694f, 0.249999791f, -1.0f });
	fe::collision::addVertex(meshData, { 0.433012694f, 0.249999791f, 1.0f });
	fe::collision::addVertex(meshData, { 0.5f, -0.000000210f, -1.0f });
	fe::collision::addVertex(meshData, { 0.5f, -0.000000210f, 1.0f });
	fe::collision::addVertex(meshData, { 0.433012694f, -0.250000208f, -1.0f });
	fe::collision::addVertex(meshData, { 0.433012694f, -0.250000208f, 1.0f });
	fe::collision::addVertex(meshData, { 0.250000029f, -0.433012902f, -1.0f });
	fe::collision::addVertex(meshData, { 0.250000029f, -0.433012902f, 1.0f });
	fe::collision::addVertex(meshData, { 0.00000006f, -0.500000178f, -1.0f });
	fe::collision::addVertex(meshData, { 0.00000006f, -0.500000178f, 1.0f });
	fe::collision::addVertex(meshData, { -0.249999910f, -0.433012962f, -1.0f });
	fe::collision::addVertex(meshData, { -0.249999910f, -0.433012962f, 1.0f });
	fe::collision::addVertex(meshData, { -0.433012634f, -0.250000357f, -1.0f });
	fe::collision::addVertex(meshData, { -0.433012634f, -0.250000357f, 1.0f });
	fe::collision::addVertex(meshData, { -0.5f, -0.000000421f, -1.0f });
	fe::collision::addVertex(meshData, { -0.5f, -0.000000421f, 1.0f });
	fe::collision::addVertex(meshData, { -0.433012872f, 0.249999567f, -1.0f });
	fe::collision::addVertex(meshData, { -0.433012872f, 0.249999567f, 1.0f });
	fe::collision::addVertex(meshData, { -0.250000327f, 0.433012336f, -1.0f });
	fe::collision::addVertex(meshData, { -0.250000327f, 0.433012336f, 1.0f });
	fe::collision::addVertex(meshData, { 0.0f, 1.0f, -1.0f });
	fe::collision::addVertex(meshData, { 0.0f, 1.0f, 1.0f });
	fe::collision::addVertex(meshData, { 0.5f, 0.866025388f, -1.0f });
	fe::collision::addVertex(meshData, { 0.5f, 0.866025388f, 1.0f });
	fe::collision::addVertex(meshData, { 0.866025447f, 0.499999970f, -1.0f });
	fe::collision::addVertex(meshData, { 0.866025447f, 0.499999970f, 1.0f });
	fe::collision::addVertex(meshData, { 1.0f, -0.000000043f, -1.0f });
	fe::collision::addVertex(meshData, { 1.0f, -0.000000043f, 1.0f });
	fe::collision::addVertex(meshData, { 0.866025388f, -0.500000059f, -1.0f });
	fe::collision::addVertex(meshData, { 0.866025388f, -0.500000059f, 1.0f });
	fe::collision::addVertex(meshData, { 0.500000059f, -0.866025388f, -1.0f });
	fe::collision::addVertex(meshData, { 0.500000059f, -0.866025388f, 1.0f });
	fe::collision::addVertex(meshData, { 0.00000015f, -1.0f, -1.0f });
	fe::collision::addVertex(meshData, { 0.00000015f, -1.0f, 1.0f });
	fe::collision::addVertex(meshData, { -0.499999791f, -0.866025507f, -1.0f });
	fe::collision::addVertex(meshData, { -0.499999791f, -0.866025507f, 1.0f });
	fe::collision::addVertex(meshData, { -0.866025209f, -0.500000298f, -1.0f });
	fe::collision::addVertex(meshData, { -0.866025209f, -0.500000298f, 1.0f });
	fe::collision::addVertex(meshData, { -1.0f, -0.000000464f, -1.0f });
	fe::collision::addVertex(meshData, { -1.0f, -0.000000464f, 1.0f });
	fe::collision::addVertex(meshData, { -0.866025686f, 0.499999493f, -1.0f });
	fe::collision::addVertex(meshData, { -0.866025686f, 0.499999493f, 1.0f });
	fe::collision::addVertex(meshData, { -0.500000596f, 0.866025090f, -1.0f });
	fe::collision::addVertex(meshData, { -0.500000596f, 0.866025090f, 1.0f });
	fe::collision::addFace(meshData, { 0, 2, 3, 1 });
	fe::collision::addFace(meshData, { 2, 4, 5, 3 });
	fe::collision::addFace(meshData, { 4, 6, 7, 5 });
	fe::collision::addFace(meshData, { 6, 8, 9, 7 });
	fe::collision::addFace(meshData, { 8, 10, 11, 9 });
	fe::collision::addFace(meshData, { 10, 12, 13, 11 });
	fe::collision::addFace(meshData, { 12, 14, 15, 13 });
	fe::collision::addFace(meshData, { 14, 16, 17, 15 });
	fe::collision::addFace(meshData, { 16, 18, 19, 17 });
	fe::collision::addFace(meshData, { 18, 20, 21, 19 });
	fe::collision::addFace(meshData, { 20, 22, 23, 21 });
	fe::collision::addFace(meshData, { 22, 0, 1, 23 });
	fe::collision::addFace(meshData, { 24, 25, 27, 26 });
	fe::collision::addFace(meshData, { 26, 27, 29, 28 });
	fe::collision::addFace(meshData, { 28, 29, 31, 30 });
	fe::collision::addFace(meshData, { 30, 31, 33, 32 });
	fe::collision::addFace(meshData, { 32, 33, 35, 34 });
	fe::collision::addFace(meshData, { 34, 35, 37, 36 });
	fe::collision::addFace(meshData, { 36, 37, 39, 38 });
	fe::collision::addFace(meshData, { 38, 39, 41, 40 });
	fe::collision::addFace(meshData, { 40, 41, 43, 42 });
	fe::collision::addFace(meshData, { 42, 43, 45, 44 });
	fe::collision::addFace(meshData, { 44, 45, 47, 46 });
	fe::collision::addFace(meshData, { 46, 47, 25, 24 });
	fe::collision::addFace(meshData, { 13, 15, 39, 37 });
	fe::collision::addFace(meshData, { 37, 35, 11, 13 });
	fe::collision::addFace(meshData, { 35, 33, 9, 11 });
	fe::collision::addFace(meshData, { 33, 31, 7, 9 });
	fe::collision::addFace(meshData, { 31, 29, 5, 7 });
	fe::collision::addFace(meshData, { 29, 27, 3, 5 });
	fe::collision::addFace(meshData, { 27, 25, 1, 3 });
	fe::collision::addFace(meshData, { 25, 47, 23, 1 });
	fe::collision::addFace(meshData, { 47, 45, 21, 23 });
	fe::collision::addFace(meshData, { 45, 43, 19, 21 });
	fe::collision::addFace(meshData, { 43, 41, 17, 19 });
	fe::collision::addFace(meshData, { 41, 39, 15, 17 });
	fe::collision::addFace(meshData, { 20, 18, 42, 44 });
	fe::collision::addFace(meshData, { 16, 40, 42, 18 });
	fe::collision::addFace(meshData, { 14, 38, 40, 16 });
	fe::collision::addFace(meshData, { 12, 36, 38, 14 });
	fe::collision::addFace(meshData, { 10, 34, 36, 12 });
	fe::collision::addFace(meshData, { 8, 32, 34, 10 });
	fe::collision::addFace(meshData, { 6, 30, 32, 8 });
	fe::collision::addFace(meshData, { 4, 28, 30, 6 });
	fe::collision::addFace(meshData, { 2, 26, 28, 4 });
	fe::collision::addFace(meshData, { 0, 24, 26, 2 });
	fe::collision::addFace(meshData, { 22, 46, 24, 0 });
	fe::collision::addFace(meshData, { 20, 44, 46, 22 });
	return meshData;
}


std::vector<fe::collision::HalfEdgeMesh> createTestTube2()
{
	fe::collision::HalfEdgeMesh m0;
	fe::collision::addVertex(m0, { -0.000000014f, 0.499999761f, -1.0f });
	fe::collision::addVertex(m0, { -0.000000014f, 0.499999761f, 1.0f });
	fe::collision::addVertex(m0, { 0.24999997f, 0.433012485f, -1.0f });
	fe::collision::addVertex(m0, { 0.24999997f, 0.433012485f, 1.0f });
	fe::collision::addVertex(m0, { 0.0f, 1.0f, -1.0f });
	fe::collision::addVertex(m0, { 0.0f, 1.0f, 1.0f });
	fe::collision::addVertex(m0, { 0.5f, 0.866025388f, -1.0f });
	fe::collision::addVertex(m0, { 0.5f, 0.866025388f, 1.0f });
	fe::collision::addFace(m0, { 0, 2, 3, 1 });
	fe::collision::addFace(m0, { 5, 7, 6, 4 });
	fe::collision::addFace(m0, { 0, 1, 5, 4 });
	fe::collision::addFace(m0, { 3, 7, 5, 1 });
	fe::collision::addFace(m0, { 2, 0, 4, 6 });
	fe::collision::addFace(m0, { 2, 6, 7, 3 });

	fe::collision::HalfEdgeMesh m1;
	fe::collision::addVertex(m1, { 0.249999970f, 0.433012455f, -1.0f });
	fe::collision::addVertex(m1, { 0.249999970f, 0.433012455f, 1.0f });
	fe::collision::addVertex(m1, { 0.433012664f, 0.249999776f, -1.0f });
	fe::collision::addVertex(m1, { 0.433012664f, 0.249999776f, 1.0f });
	fe::collision::addVertex(m1, { 0.500000119f, 0.866025328f, -1.0f });
	fe::collision::addVertex(m1, { 0.500000119f, 0.866025328f, 1.0f });
	fe::collision::addVertex(m1, { 0.866025507f, 0.49999991f, -1.0f });
	fe::collision::addVertex(m1, { 0.866025507f, 0.49999991f, 1.0f });
	fe::collision::addFace(m1, { 5, 7, 6, 4 });
	fe::collision::addFace(m1, { 0, 2, 3, 1 });
	fe::collision::addFace(m1, { 0, 1, 5, 4 });
	fe::collision::addFace(m1, { 3, 7, 5, 1 });
	fe::collision::addFace(m1, { 2, 0, 4, 6 });
	fe::collision::addFace(m1, { 2, 6, 7, 3 });

	fe::collision::HalfEdgeMesh m2;
	fe::collision::addVertex(m2, { 0.433012694f, 0.249999791f, -1.0f });
	fe::collision::addVertex(m2, { 0.433012694f, 0.249999791f, 1.0f });
	fe::collision::addVertex(m2, { 0.5f, -0.000000209f, -1.0f });
	fe::collision::addVertex(m2, { 0.5f, -0.000000209f, 1.0f });
	fe::collision::addVertex(m2, { 0.866025447f, 0.49999997f, -1.0f });
	fe::collision::addVertex(m2, { 0.866025447f, 0.49999997f, 1.0f });
	fe::collision::addVertex(m2, { 1.0f, -0.000000043f, -1.0f });
	fe::collision::addVertex(m2, { 1.0f, -0.000000043f, 1.0f });
	fe::collision::addFace(m2, { 0, 2, 3, 1 });
	fe::collision::addFace(m2, { 7, 6, 4, 5 });
	fe::collision::addFace(m2, { 0, 1, 5, 4 });
	fe::collision::addFace(m2, { 3, 7, 5, 1 });
	fe::collision::addFace(m2, { 4, 6, 2, 0 });
	fe::collision::addFace(m2, { 2, 6, 7, 3 });

	fe::collision::HalfEdgeMesh m3;
	fe::collision::addVertex(m3, { 0.499999970f, -0.000000238f, -1.0f });
	fe::collision::addVertex(m3, { 0.499999970f, -0.000000238f, 1.0f });
	fe::collision::addVertex(m3, { 0.433012723f, -0.250000238f, -1.0f });
	fe::collision::addVertex(m3, { 0.433012723f, -0.250000238f, 1.0f });
	fe::collision::addVertex(m3, { 1.000000119f, -0.000000149f, -1.0f });
	fe::collision::addVertex(m3, { 1.000000119f, -0.000000149f, 1.0f });
	fe::collision::addVertex(m3, { 0.866025388f, -0.500000059f, -1.0f });
	fe::collision::addVertex(m3, { 0.866025388f, -0.500000059f, 1.0f });
	fe::collision::addFace(m3, { 7, 6, 4, 5 });
	fe::collision::addFace(m3, { 0, 2, 3, 1 });
	fe::collision::addFace(m3, { 0, 1, 5, 4 });
	fe::collision::addFace(m3, { 3, 7, 5, 1 });
	fe::collision::addFace(m3, { 4, 6, 2, 0 });
	fe::collision::addFace(m3, { 2, 6, 7, 3 });

	fe::collision::HalfEdgeMesh m4;
	fe::collision::addVertex(m4, { 0.433012694f, -0.250000208f, -1.0f });
	fe::collision::addVertex(m4, { 0.433012694f, -0.250000208f, 1.0f });
	fe::collision::addVertex(m4, { 0.250000029f, -0.433012902f, -1.0f });
	fe::collision::addVertex(m4, { 0.250000029f, -0.433012902f, 1.0f });
	fe::collision::addVertex(m4, { 0.866025388f, -0.500000059f, -1.0f });
	fe::collision::addVertex(m4, { 0.866025388f, -0.500000059f, 1.0f });
	fe::collision::addVertex(m4, { 0.500000059f, -0.866025388f, -1.0f });
	fe::collision::addVertex(m4, { 0.500000059f, -0.866025388f, 1.0f });
	fe::collision::addFace(m4, { 0, 2, 3, 1 });
	fe::collision::addFace(m4, { 2, 6, 7, 3 });
	fe::collision::addFace(m4, { 5, 7, 6, 4 });
	fe::collision::addFace(m4, { 3, 7, 5, 1 });
	fe::collision::addFace(m4, { 0, 1, 5, 4 });
	fe::collision::addFace(m4, { 4, 6, 2, 0 });

	fe::collision::HalfEdgeMesh m5;
	fe::collision::addVertex(m5, { 0.24999997f, -0.433012902f, -1.0f });
	fe::collision::addVertex(m5, { 0.24999997f, -0.433012902f, 1.0f });
	fe::collision::addVertex(m5, { 0.0f, -0.500000238f, -1.0f });
	fe::collision::addVertex(m5, { 0.0f, -0.500000238f, 1.0f });
	fe::collision::addVertex(m5, { 0.5f, -0.866025567f, -1.0f });
	fe::collision::addVertex(m5, { 0.5f, -0.866025567f, 1.0f });
	fe::collision::addVertex(m5, { 0.00000017f, -1.0f, -1.0f });
	fe::collision::addVertex(m5, { 0.00000017f, -1.0f, 1.0f });
	fe::collision::addFace(m5, { 5, 7, 6, 4 });
	fe::collision::addFace(m5, { 2, 6, 7, 3 });
	fe::collision::addFace(m5, { 0, 2, 3, 1 });
	fe::collision::addFace(m5, { 3, 7, 5, 1 });
	fe::collision::addFace(m5, { 0, 1, 5, 4 });
	fe::collision::addFace(m5, { 4, 6, 2, 0 });

	fe::collision::HalfEdgeMesh m6;
	fe::collision::addVertex(m6, { 0.000000059f, -0.500000178f, -1.0f });
	fe::collision::addVertex(m6, { 0.000000059f, -0.500000178f, 1.0f });
	fe::collision::addVertex(m6, { -0.249999910f, -0.433012962f, -1.0f });
	fe::collision::addVertex(m6, { -0.249999910f, -0.433012962f, 1.0f });
	fe::collision::addVertex(m6, { 0.000000150f, -1.0f, -1.0f });
	fe::collision::addVertex(m6, { 0.000000150f, -1.0f, 1.0f });
	fe::collision::addVertex(m6, { -0.499999791f, -0.866025507f, -1.0f });
	fe::collision::addVertex(m6, { -0.499999791f, -0.866025507f, 1.0f });
	fe::collision::addFace(m6, { 5, 7, 6, 4 });
	fe::collision::addFace(m6, { 0, 2, 3, 1 });
	fe::collision::addFace(m6, { 3, 7, 5, 1 });
	fe::collision::addFace(m6, { 2, 6, 7, 3 });
	fe::collision::addFace(m6, { 0, 1, 5, 4 });
	fe::collision::addFace(m6, { 4, 6, 2, 0 });

	fe::collision::HalfEdgeMesh m7;
	fe::collision::addVertex(m7, { -0.249999925f, -0.433012932f, -1.0f });
	fe::collision::addVertex(m7, { -0.249999925f, -0.433012932f, 1.0f });
	fe::collision::addVertex(m7, { -0.433012634f, -0.250000298f, -1.0f });
	fe::collision::addVertex(m7, { -0.433012634f, -0.250000298f, 1.0f });
	fe::collision::addVertex(m7, { -0.499999761f, -0.866025567f, -1.0f });
	fe::collision::addVertex(m7, { -0.499999761f, -0.866025567f, 1.0f });
	fe::collision::addVertex(m7, { -0.866025149f, -0.500000238f, -1.0f });
	fe::collision::addVertex(m7, { -0.866025149f, -0.500000238f, 1.0f });
	fe::collision::addFace(m7, { 5, 7, 6, 4 });
	fe::collision::addFace(m7, { 0, 2, 3, 1 });
	fe::collision::addFace(m7, { 3, 7, 5, 1 });
	fe::collision::addFace(m7, { 2, 6, 7, 3 });
	fe::collision::addFace(m7, { 0, 1, 5, 4 });
	fe::collision::addFace(m7, { 4, 6, 2, 0 });

	fe::collision::HalfEdgeMesh m8;
	fe::collision::addVertex(m8, { -0.433012634f, -0.250000357f, -1.0f });
	fe::collision::addVertex(m8, { -0.433012634f, -0.250000357f, 1.0f });
	fe::collision::addVertex(m8, { -0.5f, -0.000000421f, -1.0f });
	fe::collision::addVertex(m8, { -0.5f, -0.000000421f, 1.0f });
	fe::collision::addVertex(m8, { -0.866025209f, -0.500000298f, -1.0f });
	fe::collision::addVertex(m8, { -0.866025209f, -0.500000298f, 1.0f });
	fe::collision::addVertex(m8, { -1.0f, -0.000000464f, -1.0f });
	fe::collision::addVertex(m8, { -1.0f, -0.000000464f, 1.0f });
	fe::collision::addFace(m8, { 0, 2, 3, 1 });
	fe::collision::addFace(m8, { 5, 7, 6, 4 });
	fe::collision::addFace(m8, { 3, 7, 5, 1 });
	fe::collision::addFace(m8, { 2, 6, 7, 3 });
	fe::collision::addFace(m8, { 0, 1, 5, 4 });
	fe::collision::addFace(m8, { 4, 6, 2, 0 });

	fe::collision::HalfEdgeMesh m9;
	fe::collision::addVertex(m9, { -0.500000059f, -0.0000004f, -1.0f });
	fe::collision::addVertex(m9, { -0.500000059f, -0.0000004f, 1.0f });
	fe::collision::addVertex(m9, { -0.433012783f, 0.249999582f, -1.0f });
	fe::collision::addVertex(m9, { -0.433012783f, 0.249999582f, 1.0f });
	fe::collision::addVertex(m9, { -0.999999821f, -0.000000387f, -1.0f });
	fe::collision::addVertex(m9, { -0.999999821f, -0.000000387f, 1.0f });
	fe::collision::addVertex(m9, { -0.866025507f, 0.499999523f, -1.0f });
	fe::collision::addVertex(m9, { -0.866025507f, 0.499999523f, 1.0f });
	fe::collision::addFace(m9, { 5, 7, 6, 4 });
	fe::collision::addFace(m9, { 0, 2, 3, 1 });
	fe::collision::addFace(m9, { 3, 7, 5, 1 });
	fe::collision::addFace(m9, { 2, 6, 7, 3 });
	fe::collision::addFace(m9, { 0, 1, 5, 4 });
	fe::collision::addFace(m9, { 4, 6, 2, 0 });

	fe::collision::HalfEdgeMesh m10;
	fe::collision::addVertex(m10, { -0.433012872f, 0.249999567f, -1.0f });
	fe::collision::addVertex(m10, { -0.433012872f, 0.249999567f, 1.0f });
	fe::collision::addVertex(m10, { -0.250000327f, 0.433012336f, -1.0f });
	fe::collision::addVertex(m10, { -0.250000327f, 0.433012336f, 1.0f });
	fe::collision::addVertex(m10, { -0.866025686f, 0.499999493f, -1.0f });
	fe::collision::addVertex(m10, { -0.866025686f, 0.499999493f, 1.0f });
	fe::collision::addVertex(m10, { -0.500000596f, 0.866025090f, -1.0f });
	fe::collision::addVertex(m10, { -0.500000596f, 0.866025090f, 1.0f });
	fe::collision::addFace(m10, { 0, 2, 3, 1 });
	fe::collision::addFace(m10, { 5, 7, 6, 4 });
	fe::collision::addFace(m10, { 3, 7, 5, 1 });
	fe::collision::addFace(m10, { 2, 6, 7, 3 });
	fe::collision::addFace(m10, { 0, 1, 5, 4 });
	fe::collision::addFace(m10, { 4, 6, 2, 0 });

	fe::collision::HalfEdgeMesh m11;
	fe::collision::addVertex(m11, { -0.250000268f, 0.433012396f, -1.0f });
	fe::collision::addVertex(m11, { -0.250000268f, 0.433012396f, 1.0f });
	fe::collision::addVertex(m11, { -0.000000357f, 0.499999791f, -1.0f });
	fe::collision::addVertex(m11, { -0.000000357f, 0.499999791f, 1.0f });
	fe::collision::addVertex(m11, { -0.500000417f, 0.866024971f, -1.0f });
	fe::collision::addVertex(m11, { -0.500000417f, 0.866024971f, 1.0f });
	fe::collision::addVertex(m11, { -0.000000536f, 1.0f, -1.0f });
	fe::collision::addVertex(m11, { -0.000000536f, 1.0f, 1.0f });
	fe::collision::addFace(m11, { 5, 7, 6, 4 });
	fe::collision::addFace(m11, { 0, 2, 3, 1 });
	fe::collision::addFace(m11, { 3, 7, 5, 1 });
	fe::collision::addFace(m11, { 2, 6, 7, 3 });
	fe::collision::addFace(m11, { 0, 1, 5, 4 });
	fe::collision::addFace(m11, { 4, 6, 2, 0 });

	return { m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11 };
}
