#include <gtest/gtest.h>
#include <se/physics/collision/AABB.h>

using namespace se::physics;
static constexpr float kTolerance = 0.000001f;

TEST(AABB, expandInside)
{
	const AABB aabb1{ glm::vec3(-0.942509233f, 1.413457870f, 1.803313970f), glm::vec3(0.942509472f, 2.484320640f, 3.636957406f) };
	const AABB aabb2{ glm::vec3(0.033556699f, 1.708075523f, 2.498030185f), glm::vec3(0.140643119f, 1.896577358f, 2.681394577f) };
	const AABB expectedResult = aabb1;
	const AABB result = expand(aabb1, aabb2);
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(result.minimum[i], expectedResult.minimum[i], kTolerance);
		EXPECT_NEAR(result.maximum[i], expectedResult.maximum[i], kTolerance);
	}
}


TEST(AABB, expandOutside)
{
	const AABB aabb1{ glm::vec3(-1.005381226f, 0.734071910f, 2.225432872f), glm::vec3(1.005381584f, 2.378627538f, 3.214838504f) };
	const AABB aabb2{ glm::vec3(1.355390191f, -0.654550194f, 3.648259878f), glm::vec3(1.418809890f, 1.459201574f, 3.831624269f) };
	const AABB expectedResult{ glm::vec3(-1.005381226f, -0.654550194f, 2.225432872f), glm::vec3(1.418809890f, 2.378627538f, 3.831624269f) };
	const AABB result = expand(aabb1, aabb2);
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(result.minimum[i], expectedResult.minimum[i], kTolerance);
		EXPECT_NEAR(result.maximum[i], expectedResult.maximum[i], kTolerance);
	}
}


TEST(AABB, expandIntersecting)
{
	const AABB aabb1{ glm::vec3(-2.533915996f, -1.860466957f, 2.169377803f), glm::vec3(-0.447687864f, -0.965565443f, 3.213990211f) };
	const AABB aabb2{ glm::vec3(-0.633507549f, -2.244378089f, 1.569726586f), glm::vec3(-0.151783823f, -1.616235971f, 2.773105144f) };
	const AABB expectedResult{ glm::vec3(-2.533915996f, -2.244378089f, 1.569726586f), glm::vec3(-0.151783823f, -0.965565443f, 3.213990211f) };
	const AABB result = expand(aabb1, aabb2);
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(result.minimum[i], expectedResult.minimum[i], kTolerance);
		EXPECT_NEAR(result.maximum[i], expectedResult.maximum[i], kTolerance);
	}
}


TEST(AABB, overlaps)
{
	const AABB aabb1{ glm::vec3(-3.5f, 1.0f, 2.5f), glm::vec3(-1.5f, 2.0f, 3.5f) };
	const AABB aabb2{ glm::vec3(-2.0f, -0.85f, 1.0f), glm::vec3(-1.0f, 1.15f, 3.0f) };
	EXPECT_TRUE(overlaps(aabb1, aabb2, kTolerance));
}


TEST(AABB, overlapsVertex)
{
	const AABB aabb1{ glm::vec3(-4.0f, -2.5f, -3.5f), glm::vec3(-2.0f, -1.5f, -1.5f) };
	const AABB aabb2{ glm::vec3(-2.0f, -3.5f, -1.5f), glm::vec3(-1.0f, -2.5f, -0.5f) };
	EXPECT_TRUE(overlaps(aabb1, aabb2, kTolerance));
}


TEST(AABB, overlapsEdge)
{
	const AABB aabb1{ glm::vec3(-4.0f, -2.5f, -3.5f), glm::vec3(-2.0f, -1.5f, -1.5f) };
	const AABB aabb2{ glm::vec3(-3.5f, -1.5f, -4.5f), glm::vec3(-2.5f, -0.5f, -3.5f) };
	EXPECT_TRUE(overlaps(aabb1, aabb2, kTolerance));
}


TEST(AABB, overlapsFace)
{
	const AABB aabb1{ glm::vec3(-4.0f, -2.5f, -3.5f), glm::vec3(-2.0f, -1.5f, -1.5f) };
	const AABB aabb2{ glm::vec3(-3.5f, -3.5f, -3.0f), glm::vec3(-2.5f, -2.5f, -2.0f) };
	EXPECT_TRUE(overlaps(aabb1, aabb2, kTolerance));
}


TEST(AABB, overlapsFalse)
{
	const AABB aabb1{ glm::vec3(-3.5f, 1.0f, 2.5f), glm::vec3(-1.5f, 2.0f, 3.5f) };
	const AABB aabb2{ glm::vec3(0.5f, -0.85f, 1.0f), glm::vec3(1.5f, 1.15f, 3.0f) };
	EXPECT_FALSE(overlaps(aabb1, aabb2, kTolerance));
}


TEST(AABB, intersectsVertex)
{
	const AABB aabb1{ glm::vec3(-3.5f, 1.0f, 2.5f), glm::vec3(-1.5f, 2.0f, 3.5f) };
	EXPECT_TRUE(intersects(aabb1, glm::vec3(0.78f, 3.5f, 0.75f), glm::vec3(-0.588479697f, -0.387157678f, 0.709789097f), kTolerance));
}


TEST(AABB, intersectsEdge)
{
	const AABB aabb1{ glm::vec3(-3.5f, 1.0f, 2.5f), glm::vec3(-1.5f, 2.0f, 3.5f) };
	EXPECT_TRUE(intersects(aabb1, glm::vec3(0.78f, -1.0f, 0.75f), glm::vec3(-0.798201382f, 0.506318867f, 0.326367408f), kTolerance));
}


TEST(AABB, intersectsFace)
{
	const AABB aabb1{ glm::vec3(-3.5f, 1.0f, 2.5f), glm::vec3(-1.5f, 2.0f, 3.5f) };
	EXPECT_TRUE(intersects(aabb1, glm::vec3(-10.0f, -2.0f, 0.75f), glm::vec3(0.870518684f, 0.406005114f, 0.278131544f), kTolerance));
}


TEST(AABB, intersectsInside)
{
	const AABB aabb1{ glm::vec3(-3.5f, 1.0f, 2.5f), glm::vec3(-1.5f, 2.0f, 3.5f) };
	EXPECT_TRUE(intersects(aabb1, glm::vec3(-3.011199712f, 1.487346529f, 3.000000238f), glm::vec3(0.0f, 0.0f, 1.0f), kTolerance));
}


TEST(AABB, intersectsFalse)
{
	const AABB aabb1{ glm::vec3(-3.5f, 1.0f, 2.5f), glm::vec3(-1.5f, 2.0f, 3.5f) };
	EXPECT_FALSE(intersects(aabb1, glm::vec3(-10.0f, -2.0f, 0.75f), glm::vec3(0.725818753f, 0.647548079f, 0.232095971f), kTolerance));
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


TEST(AABB, area1)
{
	float expectedArea = 10.0f;
	const AABB aabb1{
		glm::vec3(-3.5f, 1.0f, 2.5f), glm::vec3(-1.5f, 2.0f, 3.5f)
	};

	auto res = calculateArea(aabb1);
	EXPECT_NEAR(res, expectedArea, kTolerance);
}
