#include <gtest/gtest.h>
#include <se/collision/AABB.h>

using namespace se::collision;
static constexpr float kTolerance = 0.000001f;

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


TEST(AABB, transforms1)
{
	const AABB expectedAABB{
		glm::vec3(0.895738661f,-0.550082027f, 2.521824836f),
		glm::vec3(2.960489511f, 0.775754570f, 4.899669170f)
	};
	const AABB aabb1{
		glm::vec3(-3.5f, 1.0f, 2.5f), glm::vec3(-1.5f, 2.0f, 3.5f)
	};
	const glm::mat4 transforms(
		0.435682356f, -0.000000004f, -0.900100529f, 7.474256515f,
		0.375225603f, 0.908965766f, 0.181623250f, 2.883776426f,
		0.818160533f, -0.416870802f, 0.396020323f, 1.379018545f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	auto res = transform(aabb1, transforms);
	for (std::size_t i = 0; i < 3; ++i) {
		EXPECT_NEAR(res.minimum[i], expectedAABB.minimum[i], kTolerance);
		EXPECT_NEAR(res.maximum[i], expectedAABB.maximum[i], kTolerance);
	}
}
