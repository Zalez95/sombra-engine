#include <fe/collision/MeshCollider.h>
#include <fe/collision/ConvexPolyhedron.h>
#include <gtest/gtest.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define TOLERANCE 0.000000001


TEST(MeshCollider, getAABB)
{
	const std::vector<glm::vec3> vertices = {
		{  1.25f,  1.0f, -2.75f },
		{  1.25f, -1.0f, -2.75f },
		{ -0.25f, -1.0f, -2.75f },
		{ -0.25f,  1.0f,  0.0f  },
		{  1.25f,  1.0f,  2.75f },
		{  1.25f, -1.0f,  2.75f },
		{ -0.25f, -1.0f,  0.0f  },
		{ -0.25f,  1.0f,  2.75f },
		{  0.25f,  0.0f,  0.0f  }
	};
	const std::vector<unsigned short> indices = {
		0, 1, 2,
		7, 6, 5,
		1, 5, 6,
		6, 7, 3,
		4, 0, 3,
		0, 4, 8,
		1, 0, 8,
		4, 5, 8,
		5, 1, 8,
		3, 0, 2,
		4, 7, 5,
		2, 1, 6,
		2, 6, 3,
		7, 4, 3
	};

	const fe::collision::MeshCollider mc1(vertices, indices);
	const fe::collision::AABB aabb1 = mc1.getAABB();
	EXPECT_EQ(aabb1.mMinimum, glm::vec3(-0.25f, -1.0f, -2.75f));
	EXPECT_EQ(aabb1.mMaximum, glm::vec3(1.25f, 1.0, 2.75f));
}


TEST(MeshCollider, getAABBTransforms)
{
	const glm::vec3 translation(5.0f, -1.0f, -10.0f);
	const glm::quat rotation = glm::angleAxis(glm::pi<float>()/3, glm::vec3(2/3.0f, -2/3.0f, 1/3.0f));
	const std::vector<glm::vec3> vertices = {
		{  1.25f,  1.0f, -2.75f },
		{  1.25f, -1.0f, -2.75f },
		{ -0.25f, -1.0f, -2.75f },
		{ -0.25f,  1.0f,  0.0f  },
		{  1.25f,  1.0f,  2.75f },
		{  1.25f, -1.0f,  2.75f },
		{ -0.25f, -1.0f,  0.0f  },
		{ -0.25f,  1.0f,  2.75f },
		{  0.25f,  0.0f,  0.0f  }
	};
	const std::vector<unsigned short> indices = {
		0, 1, 2,
		7, 6, 5,
		1, 5, 6,
		6, 7, 3,
		4, 0, 3,
		0, 4, 8,
		1, 0, 8,
		4, 5, 8,
		5, 1, 8,
		3, 0, 2,
		4, 7, 5,
		2, 1, 6,
		2, 6, 3,
		7, 4, 3
	};

	fe::collision::MeshCollider mc1(vertices, indices);
	glm::mat4 r = glm::mat4_cast(rotation);
	glm::mat4 t = glm::translate(glm::mat4(1.0f), translation);
	mc1.setTransforms(t * r);

	const fe::collision::AABB aabb1 = mc1.getAABB();
	glm::vec3 expectedMinimum(3.02625942f, -3.53264260f, -12.16605472f);
	glm::vec3 expectedMaximum(7.69599246f, 1.69831585f, -7.14549779f);
	for (unsigned int i = 0; i < 3; ++i) {
		EXPECT_LE(abs(aabb1.mMinimum[i] - expectedMinimum[i]), TOLERANCE);
		EXPECT_LE(abs(aabb1.mMaximum[i] - expectedMaximum[i]), TOLERANCE);
	}
}


TEST(MeshCollider, getOverlapingParts)
{
	const glm::vec3 translation(5.0f, -1.0f, -10.0f);
	const glm::quat rotation = glm::angleAxis(glm::pi<float>()/3, glm::vec3(2/3.0f, -2/3.0f, 1/3.0f));
	const std::vector<glm::vec3> vertices = {
		{  1.25f,  1.0f, -2.75f },
		{  1.25f, -1.0f, -2.75f },
		{ -0.25f, -1.0f, -2.75f },
		{ -0.25f,  1.0f,  0.0f  },
		{  1.25f,  1.0f,  2.75f },
		{  1.25f, -1.0f,  2.75f },
		{ -0.25f, -1.0f,  0.0f  },
		{ -0.25f,  1.0f,  2.75f },
		{  0.25f,  0.0f,  0.0f  }
	};
	const std::vector<unsigned short> indices = {
		0, 1, 2,
		7, 6, 5,
		1, 5, 6,
		6, 7, 3,
		4, 0, 3,
		0, 4, 8,
		1, 0, 8,
		4, 5, 8,
		5, 1, 8,
		3, 0, 2,
		4, 7, 5,
		2, 1, 6,
		2, 6, 3,
		7, 4, 3
	};
	const fe::collision::AABB aabb1(
		glm::vec3(3.47687816f, -3.09886074f, -10.11952781f),
		glm::vec3(5.47687816f, -1.09886074f, -8.11952781f)
	);

	fe::collision::MeshCollider mc1(vertices, indices);
	glm::mat4 r = glm::mat4_cast(rotation);
	glm::mat4 t = glm::translate(glm::mat4(1.0f), translation);
	glm::mat4 transforms = t * r;

	mc1.setTransforms(transforms);

	auto result = mc1.getOverlapingParts(aabb1);

	std::vector<fe::collision::ConvexPolyhedron> expectedRes = {
		fe::collision::ConvexPolyhedron({ vertices[7], vertices[6], vertices[5] }),
		fe::collision::ConvexPolyhedron({ vertices[1], vertices[5], vertices[6] }),
		fe::collision::ConvexPolyhedron({ vertices[6], vertices[7], vertices[3] }),
		fe::collision::ConvexPolyhedron({ vertices[4], vertices[0], vertices[3] }),
		fe::collision::ConvexPolyhedron({ vertices[0], vertices[4], vertices[8] }),
		fe::collision::ConvexPolyhedron({ vertices[4], vertices[5], vertices[8] }),
		fe::collision::ConvexPolyhedron({ vertices[5], vertices[1], vertices[8] }),
		fe::collision::ConvexPolyhedron({ vertices[4], vertices[7], vertices[5] }),
		fe::collision::ConvexPolyhedron({ vertices[2], vertices[6], vertices[3] }),
		fe::collision::ConvexPolyhedron({ vertices[7], vertices[4], vertices[3] })
	};
	for (fe::collision::ConvexPolyhedron& cp : expectedRes) {
		cp.setTransforms(transforms);
	}

	ASSERT_EQ(result.size(), expectedRes.size());
	for (size_t i = 0; i < result.size(); ++i) {
		fe::collision::AABB aabb2 = result[i]->getAABB();
		fe::collision::AABB aabb3 = expectedRes[i].getAABB();
		for (unsigned int j = 0; j < 3; ++j) {
			EXPECT_LE(abs(aabb2.mMinimum[j] - aabb3.mMinimum[j]), TOLERANCE);
			EXPECT_LE(abs(aabb2.mMaximum[j] - aabb3.mMaximum[j]), TOLERANCE);
		}
	}
}
