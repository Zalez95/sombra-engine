#include <fe/collision/HalfEdgeMesh.h>

fe::collision::HalfEdgeMesh createTestPolyhedron1();
fe::collision::HalfEdgeMesh createTestPolyhedron2();
fe::collision::HalfEdgeMesh createTestPolyhedron3();
fe::collision::HalfEdgeMesh createTestMesh1();
fe::collision::HalfEdgeMesh createTestMesh2();
std::pair<fe::collision::HalfEdgeMesh, std::map<int, glm::vec3>> createTestMesh3();
fe::collision::HalfEdgeMesh createTestPlane1();
fe::collision::HalfEdgeMesh createTestPlane2();
fe::collision::HalfEdgeMesh createTestTube1();
std::vector<fe::collision::HalfEdgeMesh> createTestTube2();
