#include <gtest/gtest.h>
#include <fe/collision/AABB.h>

#define TOLERANCE 0.000000001


TEST(AABB, overlaps)
{
	const fe::collision::AABB aabb1(glm::vec3(-3.5f, 1.0f, 2.5f), glm::vec3(-1.5f, 2.0f, 3.5f));
	const fe::collision::AABB aabb2(glm::vec3(-2.0f, -0.85f, 1.0f), glm::vec3(-1.0f, 1.15f, 3.0f));
	EXPECT_TRUE(aabb1.overlaps(aabb2));
}


TEST(AABB, overlapsVertex)
{
	const fe::collision::AABB aabb1(glm::vec3(-4.0f, -2.5f, -3.5f), glm::vec3(-2.0f, -1.5f, -1.5f));
	const fe::collision::AABB aabb2(glm::vec3(-2.0f, -3.5f, -1.5f), glm::vec3(-1.0f, -2.5f, -0.5f));
	EXPECT_TRUE(aabb1.overlaps(aabb2));
}



TEST(AABB, overlapsEdge)
{
	const fe::collision::AABB aabb1(glm::vec3(-4.0f, -2.5f, -3.5f), glm::vec3(-2.0f, -1.5f, -1.5f));
	const fe::collision::AABB aabb2(glm::vec3(-3.5f, -1.5f, -4.5f), glm::vec3(-2.5f, -0.5f, -3.5f));
	EXPECT_TRUE(aabb1.overlaps(aabb2));
}


TEST(AABB, overlapsFace)
{
	const fe::collision::AABB aabb1(glm::vec3(-4.0f, -2.5f, -3.5f), glm::vec3(-2.0f, -1.5f, -1.5f));
	const fe::collision::AABB aabb2(glm::vec3(-3.5f, -3.5f, -3.0f), glm::vec3(-2.5f, -2.5f, -2.0f));
	EXPECT_TRUE(aabb1.overlaps(aabb2));
}


TEST(AABB, overlapsFalse)
{
	const fe::collision::AABB aabb1(glm::vec3(-3.5f, 1.0f, 2.5f), glm::vec3(-1.5f, 2.0f, 3.5f));
	const fe::collision::AABB aabb2(glm::vec3(0.5f, -0.85f, 1.0f), glm::vec3(1.5f, 1.15f, 3.0f));
	EXPECT_FALSE(aabb1.overlaps(aabb2));
}
