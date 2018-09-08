#include <gtest/gtest.h>
#include <fe/collision/QuickHull.h>
#include <fe/collision/HACD.h>
#include "TestMeshes.h"

#define TOLERANCE 0.000001f

bool compareFaces(
	const fe::collision::HEFace& f1, const fe::collision::HalfEdgeMesh& mesh1,
	const fe::collision::HEFace& f2, const fe::collision::HalfEdgeMesh& mesh2
) {
	auto f1Indices = fe::collision::getFaceIndices(mesh1, mesh1.edges[f1.edge].face);
	auto f2Indices = fe::collision::getFaceIndices(mesh2, mesh2.edges[f2.edge].face);
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


bool compareMeshes(const fe::collision::HalfEdgeMesh& mesh1, const fe::collision::HalfEdgeMesh& mesh2)
{
	return (
		(mesh1.vertices.size() == mesh2.vertices.size())
		&& std::all_of(
			mesh1.vertices.begin(), mesh1.vertices.end(),
			[&](const fe::collision::HEVertex& v1) {
				return std::any_of(
					mesh2.vertices.begin(), mesh2.vertices.end(),
					[&v1](const fe::collision::HEVertex& v2) { return v1.location == v2.location; }
				);
			}
		)
		&& (mesh1.faces.size() == mesh2.faces.size())
		&& std::all_of(
			mesh1.faces.begin(), mesh1.faces.end(),
			[&](const fe::collision::HEFace& f1) {
				return std::any_of(
					mesh2.faces.begin(), mesh2.faces.end(),
					[&](const fe::collision::HEFace& f2) { return compareFaces(f1, mesh1, f2, mesh2); }
				);
			}
		)
	);
}


TEST(MeshGeneration, calculateQuickHull1)
{
	fe::collision::HalfEdgeMesh originalMesh = createTestMesh1();
	fe::collision::HalfEdgeMesh expectedMesh = createTestMesh2();

	fe::collision::QuickHull qh(0.0001f);
	qh.calculate(originalMesh);
	EXPECT_TRUE( compareMeshes(expectedMesh, qh.getMesh()) );
}


TEST(MeshGeneration, calculateHACD1)
{
	fe::collision::HACD hacd(0.03f, 0.0001f);
	hacd.calculate( createTestTube1() );

	auto resultMeshes = hacd.getMeshes();
	auto expectedMeshes = createTestTube2();

	EXPECT_TRUE(
		(resultMeshes.size() == expectedMeshes.size())
		&& std::all_of(
			resultMeshes.begin(), resultMeshes.end(),
			[&](const fe::collision::HalfEdgeMesh& mesh1) {
				return std::any_of(
					expectedMeshes.begin(), expectedMeshes.end(),
					[&](const fe::collision::HalfEdgeMesh& mesh2) { return compareMeshes(mesh1, mesh2); }
				);
			}
		)
	);
}
