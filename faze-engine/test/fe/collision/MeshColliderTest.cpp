#include <gtest/gtest.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fe/collision/MeshCollider.h>
#include <fe/collision/ConvexPolyhedron.h>

#define TOLERANCE 0.000001f

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
		0, 1, 2, 7, 6, 5,
		1, 5, 6, 6, 7, 3,
		4, 0, 3, 0, 4, 8,
		1, 0, 8, 4, 5, 8,
		5, 1, 8, 3, 0, 2,
		4, 7, 5, 2, 1, 6,
		2, 6, 3, 7, 4, 3
	};
	const glm::vec3 expectedMinimum(-0.25f, -1.0f, -2.75f);
	const glm::vec3 expectedMaximum(1.25f, 1.0, 2.75f);

	fe::collision::MeshCollider mc1(vertices, indices);
	fe::collision::AABB aabb1 = mc1.getAABB();
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(aabb1.minimum[i], expectedMinimum[i], TOLERANCE);
		EXPECT_NEAR(aabb1.maximum[i], expectedMaximum[i], TOLERANCE);
	}
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
		0, 1, 2, 7, 6, 5,
		1, 5, 6, 6, 7, 3,
		4, 0, 3, 0, 4, 8,
		1, 0, 8, 4, 5, 8,
		5, 1, 8, 3, 0, 2,
		4, 7, 5, 2, 1, 6,
		2, 6, 3, 7, 4, 3
	};
	const glm::vec3 expectedMinimum(3.026389360f, -3.532424926f, -12.166131973f);
	const glm::vec3 expectedMaximum(7.695832729f, 1.698557257f, -7.145406246f);

	fe::collision::MeshCollider mc1(vertices, indices);
	glm::mat4 r = glm::mat4_cast(rotation);
	glm::mat4 t = glm::translate(glm::mat4(1.0f), translation);
	mc1.setTransforms(t * r);

	fe::collision::AABB aabb1 = mc1.getAABB();
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(aabb1.minimum[i], expectedMinimum[i], TOLERANCE);
		EXPECT_NEAR(aabb1.maximum[i], expectedMaximum[i], TOLERANCE);
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
		0, 1, 2, 7, 6, 5,
		1, 5, 6, 6, 7, 3,
		4, 0, 3, 0, 4, 8,
		1, 0, 8, 4, 5, 8,
		5, 1, 8, 3, 0, 2,
		4, 7, 5, 2, 1, 6,
		2, 6, 3, 7, 4, 3
	};
	const fe::collision::AABB aabb1{
		glm::vec3(3.47687816f, -3.09886074f, -10.11952781f),
		glm::vec3(5.47687816f, -1.09886074f, -8.11952781f)
	};

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
		for (int j = 0; j < 3; ++j) {
			EXPECT_NEAR(aabb2.minimum[j], aabb3.minimum[j], TOLERANCE);
			EXPECT_NEAR(aabb2.maximum[j], aabb3.maximum[j], TOLERANCE);
		}
	}
}
