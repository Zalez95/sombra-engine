#include <gtest/gtest.h>
#include <se/collision/QuickHull.h>
#include <se/collision/HACD.h>
#include "TestMeshes.h"

using namespace se::collision;
static constexpr float kTolerance = 0.000001f;

bool compareFaces(
	const HEFace& f1, const HalfEdgeMesh& mesh1,
	const HEFace& f2, const HalfEdgeMesh& mesh2
) {
	std::vector<int> f1Indices, f2Indices;
	getFaceIndices(mesh1, mesh1.edges[f1.edge].face, std::back_inserter(f1Indices));
	getFaceIndices(mesh2, mesh2.edges[f2.edge].face, std::back_inserter(f2Indices));
	if (f1Indices.size() != f2Indices.size()) {
		return false;
	}

	auto itV2 = std::find_if(
		f2Indices.begin(), f2Indices.end(),
		[&](int iV2) { return mesh1.vertices[f1Indices[0]].location == mesh2.vertices[iV2].location; }
	);
	if (itV2 == f2Indices.end()) {
		return false;
	}

	for (auto itV1 = f1Indices.begin(); itV1 != f1Indices.end();) {
		if (mesh1.vertices[*itV1].location != mesh2.vertices[*itV2].location) {
			return false;
		}

		++itV1;
		if (++itV2 == f2Indices.end()) { itV2 = f2Indices.begin(); }
	}

	return true;
}


bool compareMeshes(const HalfEdgeMesh& mesh1, const HalfEdgeMesh& mesh2)
{
	return (
		(mesh1.vertices.size() == mesh2.vertices.size())
		&& std::all_of(
			mesh1.vertices.begin(), mesh1.vertices.end(),
			[&](const HEVertex& v1) {
				return std::any_of(
					mesh2.vertices.begin(), mesh2.vertices.end(),
					[&v1](const HEVertex& v2) { return v1.location == v2.location; }
				);
			}
		)
		&& (mesh1.faces.size() == mesh2.faces.size())
		&& std::all_of(
			mesh1.faces.begin(), mesh1.faces.end(),
			[&](const HEFace& f1) {
				return std::any_of(
					mesh2.faces.begin(), mesh2.faces.end(),
					[&](const HEFace& f2) { return compareFaces(f1, mesh1, f2, mesh2); }
				);
			}
		)
	);
}


TEST(MeshGeneration, calculateQuickHull1)
{
	HalfEdgeMesh originalMesh = createTestMesh1().first;
	HalfEdgeMesh expectedMesh = createTestMesh2().first;

	QuickHull qh(0.0001f);
	qh.calculate(originalMesh);
	EXPECT_TRUE( compareMeshes(expectedMesh, qh.getMesh()) );
}


TEST(MeshGeneration, calculateQuickHull2)
{
	HalfEdgeMesh originalMesh = createTestPlane1();
	HalfEdgeMesh expectedMesh = createTestPlane2();

	QuickHull qh(0.0001f);
	qh.calculate(originalMesh);
	EXPECT_TRUE(compareMeshes(expectedMesh, qh.getMesh()));
}


TEST(MeshGeneration, calculateHACD1)
{
	HACD hacd(0.03f, 0.0001f);
	hacd.calculate( createTestMesh1().first );

	auto result = hacd.getMeshes();
	auto expectedMeshes = createTestMesh3();

	ASSERT_EQ(result.size(), expectedMeshes.size());
	EXPECT_TRUE(std::all_of(
		result.begin(), result.end(),
		[&](const std::pair<HalfEdgeMesh, ContiguousVector<glm::vec3>>& pair) {
			return std::any_of(
				expectedMeshes.begin(), expectedMeshes.end(),
				[&](const HalfEdgeMesh& mesh2) { return compareMeshes(pair.first, mesh2); }
			);
		}
	));
}


TEST(MeshGeneration, calculateHACD2)
{
	HACD hacd(0.03f, 0.0001f);
	hacd.calculate( createTestMesh5() );

	auto result = hacd.getMeshes();
	auto expectedMeshes = createTestMesh6();

	ASSERT_EQ(result.size(), expectedMeshes.size());
	EXPECT_TRUE(std::all_of(
		result.begin(), result.end(),
		[&](const std::pair<HalfEdgeMesh, ContiguousVector<glm::vec3>>& pair) {
			return std::any_of(
				expectedMeshes.begin(), expectedMeshes.end(),
				[&](const HalfEdgeMesh& mesh2) { return compareMeshes(pair.first, mesh2); }
			);
		}
	));
}


TEST(MeshGeneration, calculateHACD3)
{
	HACD hacd(0.03f, 0.0001f);
	hacd.calculate( createTestTube1() );

	auto result = hacd.getMeshes();
	auto expectedMeshes = createTestTube2();

	ASSERT_EQ(result.size(), expectedMeshes.size());
	EXPECT_TRUE(std::all_of(
		result.begin(), result.end(),
		[&](const std::pair<HalfEdgeMesh, ContiguousVector<glm::vec3>>& pair) {
			return std::any_of(
				expectedMeshes.begin(), expectedMeshes.end(),
				[&](const HalfEdgeMesh& mesh2) { return compareMeshes(pair.first, mesh2); }
			);
		}
	));
}
