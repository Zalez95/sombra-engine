#include <gtest/gtest.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <se/physics/collision/TriangleMeshCollider.h>
#include <se/physics/collision/TriangleCollider.h>
#include "TestMeshes.h"

using namespace se::physics;
static constexpr float kTolerance = 0.000001f;

TEST(TriangleMeshCollider, getAABB1)
{
	const glm::vec3 expectedMinimum(-1.568295598f, -1.456306695f, -1.216818571f);
	const glm::vec3 expectedMaximum(1.209428787f, 1.568309783f, 1.263301849f);

	auto [vertices, indices] = createTestTriangleMesh();
	TriangleMeshCollider tm1(vertices.data(), vertices.size(), indices.data(), indices.size());

	AABB aabb1 = tm1.getAABB();
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(aabb1.minimum[i], expectedMinimum[i], kTolerance);
		EXPECT_NEAR(aabb1.maximum[i], expectedMaximum[i], kTolerance);
	}
}


TEST(TriangleMeshCollider, getAABBTransforms1)
{
	const glm::vec3 expectedMinimum(1.686079740f, -3.096512794f, -3.178431987f);
	const glm::vec3 expectedMaximum(3.719449758f, -1.429600834f, -1.414804577f);

	const glm::vec3 translation(2.865250587f, -2.368927478f, -2.282903194f);
	const glm::quat rotation = { 0.928554952f, -0.294283181f, 0.006446061f, -0.226145476f };
	const glm::vec3 scale(0.754146635f, 0.702531874f, 0.627422273f);
	glm::mat4 s = glm::scale(glm::mat4(1.0f), scale);
	glm::mat4 r = glm::mat4_cast(rotation);
	glm::mat4 t = glm::translate(glm::mat4(1.0f), translation);
	glm::mat4 transforms = t * r * s;

	auto [vertices, indices] = createTestTriangleMesh();
	TriangleMeshCollider tm1(vertices.data(), vertices.size(), indices.data(), indices.size());
	tm1.setTransforms(transforms);

	AABB aabb1 = tm1.getAABB();
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(aabb1.minimum[i], expectedMinimum[i], kTolerance);
		EXPECT_NEAR(aabb1.maximum[i], expectedMaximum[i], kTolerance);
	}
}


TEST(TriangleMeshCollider, updated)
{
	auto [vertices, indices] = createTestTriangleMesh();
	TriangleMeshCollider tm1(vertices.data(), vertices.size(), indices.data(), indices.size());

	EXPECT_TRUE(tm1.updated());
	tm1.resetUpdatedState();
	EXPECT_FALSE(tm1.updated());
	tm1.setTransforms(glm::mat4(1.0f));
	EXPECT_TRUE(tm1.updated());
	tm1.resetUpdatedState();
	EXPECT_FALSE(tm1.updated());
	tm1.resetUpdatedState();
	EXPECT_FALSE(tm1.updated());
}


TEST(TriangleMeshCollider, getOverlapingParts1)
{
	const AABB aabb1{
		{ 3.325768232f, -2.174721717f, -3.304069757f },
		{ 5.325768470f, -0.174721673f, -1.304069757f }
	};

	const glm::vec3 translation(2.865250587f, -2.368927478f, -2.282903194f);
	const glm::quat rotation = { 0.928554952f, -0.294283181f, 0.006446061f, -0.226145476f };
	const glm::vec3 scale(0.754146635f, 0.702531874f, 0.627422273f);
	glm::mat4 s = glm::scale(glm::mat4(1.0f), scale);
	glm::mat4 r = glm::mat4_cast(rotation);
	glm::mat4 t = glm::translate(glm::mat4(1.0f), translation);
	glm::mat4 transforms = t * r * s;

	auto [vertices, indices] = createTestTriangleMesh();
	TriangleMeshCollider tm1(vertices.data(), vertices.size(), indices.data(), indices.size());
	tm1.setTransforms(transforms);

	std::vector<TriangleCollider> expectedRes = {
		TriangleCollider({
			glm::vec3(0.1718623340f, -0.0852808654f, 0.626737713f),
			glm::vec3(1.2094287872f, 0.1224686279f, -0.003602489f),
			glm::vec3(0.0532155819f, 0.7636756300f, 0.238783344f)
		}),
		TriangleCollider({
			glm::vec3(0.0532155819f, 0.7636756300f, 0.238783344f),
			glm::vec3(1.2094287872f, 0.1224686279f, -0.003602489f),
			glm::vec3(0.8282703161f, 0.9357475638f, -0.078342802f)
		}),
		TriangleCollider({
			glm::vec3(0.0532155819f, 0.7636756300f, 0.238783344f),
			glm::vec3(0.8282703161f, 0.9357475638f, -0.078342802f),
			glm::vec3(0.7073457241f, 1.1211483478f, -0.237675473f)
		}),
		TriangleCollider({
			glm::vec3(0.0532155819f, 0.7636756300f, 0.238783344f),
			glm::vec3(0.7073457241f, 1.1211483478f, -0.237675473f),
			glm::vec3(-0.3182429075f, 1.2670570611f, 0.564987242f)
		}),
		TriangleCollider({
			glm::vec3(-0.1741758286f, 1.5683097839f, -0.071535445f),
			glm::vec3(-0.3182429075f, 1.2670570611f, 0.564987242f),
			glm::vec3(0.7073457241f, 1.1211483478f, -0.237675473f)
		}),
		TriangleCollider({
			glm::vec3(-0.0348360687f, 1.2009259462f, -0.826681196f),
			glm::vec3(-0.1741758286f, 1.5683097839f, -0.071535445f),
			glm::vec3(0.7073457241f, 1.1211483478f, -0.237675473f)
		}),
		TriangleCollider({
			glm::vec3(0.6154503822f, 0.7604663372f, -0.866529464f),
			glm::vec3(0.7073457241f, 1.1211483478f, -0.237675473f),
			glm::vec3(0.8282703161f, 0.9357475638f, -0.078342802f)
		}),
		TriangleCollider({
			glm::vec3(-0.0348360687f, 1.2009259462f, -0.826681196f),
			glm::vec3(0.7073457241f, 1.1211483478f, -0.237675473f),
			glm::vec3(0.6154503822f, 0.7604663372f, -0.866529464f)
		}),
		TriangleCollider({
			glm::vec3(0.615450382f, 0.760466337f, -0.866529464f),
			glm::vec3(0.147921025f, 0.426297962f, -1.191849231f),
			glm::vec3(-0.034836068f, 1.200925946f, -0.826681196f)
		})
	};
	for (TriangleCollider& cp : expectedRes) {
		cp.setTransforms(transforms);
	}

	std::size_t numTris = 0;
	tm1.processOverlapingParts(aabb1, kTolerance, [&](const ConvexCollider& part) {
		auto tri1 = dynamic_cast<const TriangleCollider*>(&part);
		ASSERT_TRUE(tri1);

		auto tri2 = std::find_if(expectedRes.begin(), expectedRes.end(), [&](const TriangleCollider& tri) {
			return se::utils::compareTriangles(tri1->getLocalVertices(), tri.getLocalVertices(), kTolerance);
		});
		EXPECT_TRUE(tri2 != expectedRes.end());

		numTris++;
	});
	EXPECT_EQ(expectedRes.size(), numTris);
}


TEST(TriangleMeshCollider, processIntersectingParts)
{
	const Ray ray1({ 5.609230995f, -0.387843430f, -1.344119668f }, { -0.772185385f, -0.550935864f, -0.316543042f });

	const glm::vec3 translation(2.865250587f, -2.368927478f, -2.282903194f);
	const glm::quat rotation = { 0.928554952f, -0.294283181f, 0.006446061f, -0.226145476f };
	const glm::vec3 scale(0.754146635f, 0.702531874f, 0.627422273f);
	glm::mat4 s = glm::scale(glm::mat4(1.0f), scale);
	glm::mat4 r = glm::mat4_cast(rotation);
	glm::mat4 t = glm::translate(glm::mat4(1.0f), translation);
	glm::mat4 transforms = t * r * s;

	auto [vertices, indices] = createTestTriangleMesh();
	TriangleMeshCollider tm1(vertices.data(), vertices.size(), indices.data(), indices.size());
	tm1.setTransforms(transforms);

	std::vector<TriangleCollider> expectedRes = {
		TriangleCollider({
			glm::vec3(-0.222157642f, 1.022119164f, 0.925747632f),
			glm::vec3(0.171862334f, -0.085280865f, 0.626737713f),
			glm::vec3(0.053215581f, 0.763675630f, 0.238783344f)
		}),
		TriangleCollider({
			glm::vec3(0.171862334f, -0.085280865f, 0.626737713f),
			glm::vec3(1.209428787f, 0.122468627f, -0.003602489f),
			glm::vec3(0.053215581f, 0.763675630f, 0.238783344f)
		}),
		TriangleCollider({
			glm::vec3(0.053215581f, 0.763675630f, 0.238783344f),
			glm::vec3(1.209428787f, 0.122468627f, -0.003602489f),
			glm::vec3(0.828270316f, 0.935747563f, -0.078342802f)
		}),
		TriangleCollider({
			glm::vec3(-1.175907135f, 0.038690738f, -0.654182493f),
			glm::vec3(-0.108820162f, -0.884682297f, -0.771282374f),
			glm::vec3(-1.010702490f, -0.736890316f, -0.437148213f)
		}),
		TriangleCollider({
			glm::vec3(-1.175907135f, 0.038690738f, -0.654182493f),
			glm::vec3(0.017661752f, -0.138503223f, -1.216818571f),
			glm::vec3(-0.108820162f, -0.884682297f, -0.771282374f)
		}),
		TriangleCollider({
			glm::vec3(0.147921025f, 0.426297962f, -1.191849231f),
			glm::vec3(0.017661752f, -0.138503223f, -1.216818571f),
			glm::vec3(-1.175907135f, 0.038690738f, -0.654182493f)
		})
	};
	for (TriangleCollider& cp : expectedRes) {
		cp.setTransforms(transforms);
	}

	std::size_t numTris = 0;
	tm1.processIntersectingParts(ray1, kTolerance, [&](const ConvexCollider& part) {
		auto tri1 = dynamic_cast<const TriangleCollider*>(&part);
		ASSERT_TRUE(tri1);

		auto tri2 = std::find_if(expectedRes.begin(), expectedRes.end(), [&](const TriangleCollider& tri) {
			return se::utils::compareTriangles(tri1->getLocalVertices(), tri.getLocalVertices(), kTolerance);
		});
		EXPECT_TRUE(tri2 != expectedRes.end());

		numTris++;
	});
	EXPECT_EQ(expectedRes.size(), numTris);
}
