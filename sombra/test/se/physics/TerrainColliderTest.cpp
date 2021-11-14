#include <gtest/gtest.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <se/utils/MathUtils.h>
#include <se/physics/collision/TerrainCollider.h>
#include <se/physics/collision/TriangleCollider.h>

using namespace se::physics;
static constexpr float kTolerance = 0.000001f;

TEST(TerrainCollider, getAABB)
{
	float heights[] = {
		-0.224407124f, -0.182230042f, -0.063670491f, -0.063680544f, -0.274178390f, -0.002076677f,
		0.240925990f, -0.427923002f, 0.499461910f, 0.320841177f, 0.431347578f, 0.199959035f,
		-0.225947124f, -0.101790362f, -0.419971141f, -0.278538079f, 0.044960733f, -0.266057232f,
		0.251054237f, 0.476726697f, -0.422780143f, 0.063881184f, -0.266370011f, -0.139245431f,
		-0.279247346f, -0.234977409f, -0.294798492f, -0.247099806f, 0.002694404f, 0.378445211f,
		0.112437157f, 0.392135236f, 0.466178188f, -0.306503992f, -0.381612994f, -0.219027959f,
		0.112001758f, -0.283234569f, 0.367756026f, -0.288402094f, -0.006938715f, -0.109673572f,
		-0.283075078f, 0.129306909f, 0.134741993f, -0.250951479f, 0.104189257f, -0.422417659f
	};
	const int xSize = 6, zSize = 8;
	const glm::vec3 expectedMinimum(-0.5f, -0.427923002f, -0.5f);
	const glm::vec3 expectedMaximum(0.5f, 0.49946191f, 0.5f);

	TerrainCollider tc1;
	tc1.setHeights(heights, xSize, zSize);
	AABB aabb1 = tc1.getAABB();
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(aabb1.minimum[i], expectedMinimum[i], kTolerance);
		EXPECT_NEAR(aabb1.maximum[i], expectedMaximum[i], kTolerance);
	}
}


TEST(TerrainCollider, getAABBTransforms)
{
	const glm::vec3 scale(8.0f, 3.5f, 16.0f);
	const glm::vec3 translation(-3.24586f, -1.559f, 4.78164f);
	const glm::quat rotation = glm::angleAxis(glm::pi<float>()/3, glm::vec3(2/3.0f, -2/3.0f, 1/3.0f));
	float heights[] = {
		-0.224407124f, -0.182230042f, -0.063670491f, -0.063680544f, -0.274178390f, -0.002076677f,
		0.240925990f, -0.427923002f, 0.499461910f, 0.320841177f, 0.431347578f, 0.199959035f,
		-0.225947124f, -0.101790362f, -0.419971141f, -0.278538079f, 0.044960733f, -0.266057232f,
		0.251054237f, 0.476726697f, -0.422780143f, 0.063881184f, -0.266370011f, -0.139245431f,
		-0.279247346f, -0.234977409f, -0.294798492f, -0.247099806f, 0.002694404f, 0.378445211f,
		0.112437157f, 0.392135236f, 0.466178188f, -0.306503992f, -0.381612994f, -0.219027959f,
		0.112001758f, -0.283234569f, 0.367756026f, -0.288402094f, -0.006938715f, -0.109673572f,
		-0.283075078f, 0.129306909f, 0.134741993f, -0.250951479f, 0.104189257f, -0.422417659f
	};
	const int xSize = 6, zSize = 8;
	const glm::vec3 expectedMinimum(-9.358484268f, -8.048053741f, -2.782845735f);
	const glm::vec3 expectedMaximum(3.376655340f, 4.209253787f, 11.290613174f);

	TerrainCollider tc1;
	tc1.setHeights(heights, xSize, zSize);
	glm::mat4 s = glm::scale(glm::mat4(1.0f), scale);
	glm::mat4 r = glm::mat4_cast(rotation);
	glm::mat4 t = glm::translate(glm::mat4(1.0f), translation);
	tc1.setTransforms(t * r * s);

	AABB aabb1 = tc1.getAABB();
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(aabb1.minimum[i], expectedMinimum[i], kTolerance);
		EXPECT_NEAR(aabb1.maximum[i], expectedMaximum[i], kTolerance);
	}
}


TEST(TerrainCollider, getOverlapingParts1)
{
	const glm::vec3 scale(8.0f, 3.5f, 16.0f);
	const glm::vec3 translation(-3.24586f, -1.559f, 4.78164f);
	const glm::quat rotation = glm::angleAxis(glm::pi<float>()/3, glm::vec3(2/3.0f, -2/3.0f, 1/3.0f));
	float heights[] = {
		-0.224407124f,-0.182230042f,-0.063670491f,-0.063680544f,-0.274178390f,-0.002076677f,
		 0.240925990f,-0.427923002f, 0.499461910f, 0.320841177f, 0.431347578f, 0.199959035f,
		-0.225947124f,-0.101790362f,-0.419971141f,-0.278538079f, 0.044960733f,-0.266057232f,
		 0.251054237f, 0.476726697f,-0.422780143f, 0.063881184f,-0.266370011f,-0.139245431f,
		-0.279247346f,-0.234977409f,-0.294798492f,-0.247099806f, 0.002694404f, 0.378445211f,
		 0.112437157f, 0.392135236f, 0.466178188f,-0.306503992f,-0.381612994f,-0.219027959f,
		 0.112001758f,-0.283234569f, 0.367756026f,-0.288402094f,-0.006938715f,-0.109673572f,
		-0.283075078f, 0.129306909f, 0.134741993f,-0.250951479f, 0.104189257f,-0.422417659f
	};
	const int xSize = 6, zSize = 8;
	const AABB aabb1{
		glm::vec3(-5.620086193f,-6.647461891f, 6.396442413f),
		glm::vec3(-6.620086193f,-7.647461891f, 5.396442413f)
	};

	TerrainCollider tc1;
	tc1.setHeights(heights, xSize, zSize);
	glm::mat4 s = glm::scale(glm::mat4(1.0f), scale);
	glm::mat4 r = glm::mat4_cast(rotation);
	glm::mat4 t = glm::translate(glm::mat4(1.0f), translation);
	glm::mat4 transforms = t * r * s;

	tc1.setTransforms(transforms);

	std::size_t result = 0;
	tc1.processOverlapingParts(aabb1, kTolerance, [&result](const ConvexCollider&) { result++; });

	std::vector<TriangleCollider> expectedRes = {};
	ASSERT_EQ(result, expectedRes.size());
}


TEST(TerrainCollider, updated)
{
	float heights[] = {
		-0.224407124f,-0.182230042f,-0.063670491f,-0.063680544f,-0.274178390f,-0.002076677f,
		 0.240925990f,-0.427923002f, 0.499461910f, 0.320841177f, 0.431347578f, 0.199959035f,
		-0.225947124f,-0.101790362f,-0.419971141f,-0.278538079f, 0.044960733f,-0.266057232f,
		 0.251054237f, 0.476726697f,-0.422780143f, 0.063881184f,-0.266370011f,-0.139245431f,
		-0.279247346f,-0.234977409f,-0.294798492f,-0.247099806f, 0.002694404f, 0.378445211f,
		 0.112437157f, 0.392135236f, 0.466178188f,-0.306503992f,-0.381612994f,-0.219027959f,
		 0.112001758f,-0.283234569f, 0.367756026f,-0.288402094f,-0.006938715f,-0.109673572f,
		-0.283075078f, 0.129306909f, 0.134741993f,-0.250951479f, 0.104189257f,-0.422417659f
	};
	const int xSize = 6, zSize = 8;
	TerrainCollider tc1;
	tc1.setHeights(heights, xSize, zSize);
	EXPECT_TRUE(tc1.updated());
	tc1.resetUpdatedState();
	EXPECT_FALSE(tc1.updated());
	tc1.setTransforms(glm::mat4(1.0f));
	EXPECT_TRUE(tc1.updated());
	tc1.resetUpdatedState();
	EXPECT_FALSE(tc1.updated());
}


TEST(TerrainCollider, getOverlapingParts2)
{
	const glm::vec3 scale(8.0f, 3.5f, 16.0f);
	const glm::vec3 translation(-3.24586f, -1.559f, 4.78164f);
	const glm::quat rotation = glm::angleAxis(glm::pi<float>()/3, glm::vec3(2/3.0f, -2/3.0f, 1/3.0f));
	float heights[] = {
		-0.224407124f,-0.182230042f,-0.063670491f,-0.063680544f,-0.274178390f,-0.002076677f,
		 0.240925990f,-0.427923002f, 0.499461910f, 0.320841177f, 0.431347578f, 0.199959035f,
		-0.225947124f,-0.101790362f,-0.419971141f,-0.278538079f, 0.044960733f,-0.266057232f,
		 0.251054237f, 0.476726697f,-0.422780143f, 0.063881184f,-0.266370011f,-0.139245431f,
		-0.279247346f,-0.234977409f,-0.294798492f,-0.247099806f, 0.002694404f, 0.378445211f,
		 0.112437157f, 0.392135236f, 0.466178188f,-0.306503992f,-0.381612994f,-0.219027959f,
		 0.112001758f,-0.283234569f, 0.367756026f,-0.288402094f,-0.006938715f,-0.109673572f,
		-0.283075078f, 0.129306909f, 0.134741993f,-0.250951479f, 0.104189257f,-0.422417659f
	};
	const int xSize = 6, zSize = 8;
	const AABB aabb1{
		glm::vec3(-3.536325216f,-0.434814631f, 0.558086156f),
		glm::vec3(-2.536325216f, 0.565185368f, 1.558086156f)
	};

	TerrainCollider tc1;
	tc1.setHeights(heights, xSize, zSize);
	glm::mat4 s = glm::scale(glm::mat4(1.0f), scale);
	glm::mat4 r = glm::mat4_cast(rotation);
	glm::mat4 t = glm::translate(glm::mat4(1.0f), translation);
	glm::mat4 transforms = t * r * s;
	tc1.setTransforms(transforms);

	std::vector<TriangleCollider> expectedRes = {
		TriangleCollider({
			glm::vec3(-0.5f, 0.240925982f, -0.357142865f),
			glm::vec3(-0.300000011f, -0.427922993f, -0.357142865f),
			glm::vec3(-0.5f, -0.225947126f, -0.214285716f)
		}),
		TriangleCollider({
			glm::vec3(-0.300000011f, -0.427922993f, -0.357142865f),
			glm::vec3(-0.300000011f, -0.101790361f, -0.214285716f),
			glm::vec3(-0.5f, -0.225947126f, -0.214285716f)
		}),
		TriangleCollider({
			glm::vec3(-0.300000011f, -0.427922993f, -0.357142865f),
			glm::vec3(-0.100000001f, 0.499461919f, -0.357142865f),
			glm::vec3(-0.300000011f, -0.101790361f, -0.214285716f)
		}),
		TriangleCollider({
			glm::vec3(-0.100000001f, 0.499461919f, -0.357142865f),
			glm::vec3(-0.100000001f, -0.419971138f, -0.214285716f),
			glm::vec3(-0.300000011f, -0.101790361f, -0.214285716f)
		}),
		TriangleCollider({
			glm::vec3(-0.5f, -0.225947126f, -0.214285716f),
			glm::vec3(-0.300000011f, -0.101790361f, -0.214285716f),
			glm::vec3(-0.5f, 0.251054227f, -0.071428574f)
		}),
		TriangleCollider({
			glm::vec3(-0.300000011f, -0.101790361f, -0.214285716f),
			glm::vec3(-0.300000011f, 0.476726710f, -0.071428574f),
			glm::vec3(-0.5f, 0.251054227f, -0.071428574f)
		}),
		TriangleCollider({
			glm::vec3(-0.300000011f, -0.101790361f, -0.214285716f),
			glm::vec3(-0.100000001f, -0.419971138f, -0.214285716f),
			glm::vec3(-0.300000011f, 0.476726710f, -0.071428574f)
		}),
		TriangleCollider({
			glm::vec3(-0.100000001f, -0.419971138f, -0.214285716f),
			glm::vec3(-0.100000001f, -0.422780156f, -0.071428574f),
			glm::vec3(-0.300000011f, 0.476726710f, -0.071428574f)
		})
	};
	for (TriangleCollider& cp : expectedRes) {
		cp.setTransforms(transforms);
	}

	std::size_t iPart = 0;
	tc1.processOverlapingParts(aabb1, kTolerance, [&](const ConvexCollider& part) {
		AABB aabb2 = part.getAABB();
		AABB aabb3 = expectedRes[iPart].getAABB();
		for (int j = 0; j < 3; ++j) {
			EXPECT_NEAR(aabb2.minimum[j], aabb3.minimum[j], kTolerance);
			EXPECT_NEAR(aabb2.maximum[j], aabb3.maximum[j], kTolerance);
		}
		iPart++;
	});
	EXPECT_EQ(iPart, expectedRes.size());
}


TEST(TerrainCollider, processIntersectingParts)
{
	const Ray ray1({ -11.041489601f, -2.530857086f, 6.313727378f }, { 0.955237627f, -0.086757071f, -0.282832711f });

	const glm::vec3 scale(8.0f, 3.5f, 16.0f);
	const glm::vec3 translation(-3.24586f, -1.559f, 4.78164f);
	const glm::quat rotation = glm::angleAxis(glm::pi<float>()/3, glm::vec3(2/3.0f, -2/3.0f, 1/3.0f));
	float heights[] = {
		-0.224407124f,-0.182230042f,-0.063670491f,-0.063680544f,-0.274178390f,-0.002076677f,
		 0.240925990f,-0.427923002f, 0.499461910f, 0.320841177f, 0.431347578f, 0.199959035f,
		-0.225947124f,-0.101790362f,-0.419971141f,-0.278538079f, 0.044960733f,-0.266057232f,
		 0.251054237f, 0.476726697f,-0.422780143f, 0.063881184f,-0.266370011f,-0.139245431f,
		-0.279247346f,-0.234977409f,-0.294798492f,-0.247099806f, 0.002694404f, 0.378445211f,
		 0.112437157f, 0.392135236f, 0.466178188f,-0.306503992f,-0.381612994f,-0.219027959f,
		 0.112001758f,-0.283234569f, 0.367756026f,-0.288402094f,-0.006938715f,-0.109673572f,
		-0.283075078f, 0.129306909f, 0.134741993f,-0.250951479f, 0.104189257f,-0.422417659f
	};
	const int xSize = 6, zSize = 8;

	TerrainCollider tc1;
	tc1.setHeights(heights, xSize, zSize);
	glm::mat4 s = glm::scale(glm::mat4(1.0f), scale);
	glm::mat4 r = glm::mat4_cast(rotation);
	glm::mat4 t = glm::translate(glm::mat4(1.0f), translation);
	glm::mat4 transforms = t * r * s;
	tc1.setTransforms(transforms);

	std::vector<TriangleCollider> expectedRes = {
		TriangleCollider({
			glm::vec3(-0.100000001f, -0.422780156f, -0.071428574f),
			glm::vec3(-0.100000001f, -0.294798493f, 0.071428574f),
			glm::vec3(-0.300000011f, -0.234977409f, 0.071428574f)
		}),
		TriangleCollider({
			glm::vec3(-0.300000011f, -0.234977409f, 0.071428574f),
			glm::vec3(-0.100000001f, -0.294798493f, 0.071428574f),
			glm::vec3(-0.300000011f, 0.392135232f, 0.214285716f)
		}),
		TriangleCollider({
			glm::vec3(-0.100000001f, -0.294798493f, 0.071428574f),
			glm::vec3(-0.100000001f, 0.466178178f, 0.214285716f),
			glm::vec3(-0.300000011f, 0.392135232f, 0.214285716f)
		})
	};
	for (TriangleCollider& cp : expectedRes) {
		cp.setTransforms(transforms);
	}

	std::size_t numTris = 0;
	tc1.processIntersectingParts(ray1, kTolerance, [&](const ConvexCollider& part) {
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
