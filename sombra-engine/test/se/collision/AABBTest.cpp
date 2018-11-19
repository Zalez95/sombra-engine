#include <gtest/gtest.h>
#include <se/collision/AABB.h>

using namespace se::collision;

TEST(AABB, overlaps)
{
	const AABB aabb1{ glm::vec3(-3.5f, 1.0f, 2.5f), glm::vec3(-1.5f, 2.0f, 3.5f) };
	const AABB aabb2{ glm::vec3(-2.0f, -0.85f, 1.0f), glm::vec3(-1.0f, 1.15f, 3.0f) };
	EXPECT_TRUE(overlaps(aabb1, aabb2));
}


TEST(AABB, overlapsVertex)
{
	const AABB aabb1{ glm::vec3(-4.0f, -2.5f, -3.5f), glm::vec3(-2.0f, -1.5f, -1.5f) };
	const AABB aabb2{ glm::vec3(-2.0f, -3.5f, -1.5f), glm::vec3(-1.0f, -2.5f, -0.5f) };
	EXPECT_TRUE(overlaps(aabb1, aabb2));
}


TEST(AABB, overlapsEdge)
{
	const AABB aabb1{ glm::vec3(-4.0f, -2.5f, -3.5f), glm::vec3(-2.0f, -1.5f, -1.5f) };
	const AABB aabb2{ glm::vec3(-3.5f, -1.5f, -4.5f), glm::vec3(-2.5f, -0.5f, -3.5f) };
	EXPECT_TRUE(overlaps(aabb1, aabb2));
}


TEST(AABB, overlapsFace)
{
	const AABB aabb1{ glm::vec3(-4.0f, -2.5f, -3.5f), glm::vec3(-2.0f, -1.5f, -1.5f) };
	const AABB aabb2{ glm::vec3(-3.5f, -3.5f, -3.0f), glm::vec3(-2.5f, -2.5f, -2.0f) };
	EXPECT_TRUE(overlaps(aabb1, aabb2));
}


TEST(AABB, overlapsFalse)
{
	const AABB aabb1{ glm::vec3(-3.5f, 1.0f, 2.5f), glm::vec3(-1.5f, 2.0f, 3.5f) };
	const AABB aabb2{ glm::vec3(0.5f, -0.85f, 1.0f), glm::vec3(1.5f, 1.15f, 3.0f) };
	EXPECT_FALSE(overlaps(aabb1, aabb2));
}
