#include <gtest/gtest.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <se/physics/collision/Manifold.h>
#include <se/physics/collision/BoundingBox.h>
#include <se/physics/collision/BoundingSphere.h>
#include <se/physics/collision/TriangleCollider.h>
#include <se/physics/collision/FineCollisionDetector.h>
#include "TestMeshes.h"

using namespace se::physics;
static constexpr float kTolerance			= 0.0001f;
static constexpr float kMinFDifference		= 0.0001f;
static constexpr std::size_t kMaxIterations	= 128;
static constexpr float kContactPrecision	= 0.0000001f;
static constexpr float kContactSeparation	= 0.0000001f;

TEST(FineCollisionDetector, SphereSphere1)
{
	const glm::vec3 v1(13.5f, -5.25f, 7.1f), v2(0.0f);
	const glm::quat o1(1.0f, 0.0f, 0.0f, 0.0f), o2(0.795f, -0.002f, -0.575f, 0.192f);
	BoundingSphere bs1(2.5f), bs2(5.2f);

	glm::mat4 r1 = glm::mat4_cast(o1);
	glm::mat4 t1 = glm::translate(glm::mat4(1.0f), v1);
	bs1.setTransforms(t1 * r1);

	glm::mat4 r2 = glm::mat4_cast(o2);
	glm::mat4 t2 = glm::translate(glm::mat4(1.0f), v2);
	bs2.setTransforms(t2 * r2);

	Manifold manifold(&bs1, &bs2);
	FineCollisionDetector fineCollisionDetector(
		kMinFDifference, kMaxIterations,
		kContactPrecision, kContactSeparation
	);

	ASSERT_FALSE(fineCollisionDetector.collide(manifold));
}


TEST(FineCollisionDetector, SphereSphere2)
{
	const glm::vec3 expectedWorldPos[] = {
		{ 12.345151365f, -4.478355257f, 5.021325737f },
		{ 12.345151438f, -4.478355306f, 5.021325869f }
	};
	const glm::vec3 expectedLocalPos[] = {
		{ -1.154848634f, 0.771644742f, -2.078674167f },
		{ 4.095410456f, -3.183182967f, -0.384987776f }
	};
	const glm::vec3 expectedNormal(-0.459900856f, 0.313255608f, -0.830880344f);
	const float expectedPenetration = 0.000000159f;
	const glm::vec3 v1(13.5f, -5.25f, 7.1f), v2(9.943065643f, -2.873334407f, 0.697683811f);
	const glm::quat o1(1.0f, glm::vec3(0.0f)), o2(0.795f, -0.002f, -0.575f, 0.192f);

	BoundingSphere bs1(2.5f), bs2(5.2f);

	glm::mat4 r1 = glm::mat4_cast(o1);
	glm::mat4 t1 = glm::translate(glm::mat4(1.0f), v1);
	bs1.setTransforms(t1 * r1);

	glm::mat4 r2 = glm::mat4_cast(o2);
	glm::mat4 t2 = glm::translate(glm::mat4(1.0f), v2);
	bs2.setTransforms(t2 * r2);

	Manifold manifold(&bs1, &bs2);
	FineCollisionDetector fineCollisionDetector(
		kMinFDifference, kMaxIterations,
		kContactPrecision, kContactSeparation
	);

	ASSERT_TRUE(fineCollisionDetector.collide(manifold));
	ASSERT_EQ(static_cast<int>(manifold.contacts.size()), 1);

	const Contact& res = manifold.contacts.front();
	EXPECT_NEAR(res.penetration, expectedPenetration, kTolerance);
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(res.normal[i], expectedNormal[i], kTolerance);
		for (int j = 0; j < 2; ++j) {
			EXPECT_NEAR(res.worldPosition[j][i], expectedWorldPos[j][i], kTolerance);
			EXPECT_NEAR(res.localPosition[j][i], expectedLocalPos[j][i], kTolerance);
		}
	}
}

TEST(FineCollisionDetector, CPolyCPolyNonColliding)
{
	const glm::vec3 v1(-5.65946f, -2.8255f, -1.52118f), v2(-4.58841f, -2.39753f, -0.164247f);
	const glm::quat o1(0.890843f, 0.349613f, 0.061734f, 0.283475f), o2(0.962876f, -0.158823f, 0.216784f, -0.025477f);
	BoundingBox bb1({ 2.0f, 1.0f, 2.0f }), bb2({ 1.0f, 1.0f, 0.5f });

	glm::mat4 r1 = glm::mat4_cast(o1);
	glm::mat4 t1 = glm::translate(glm::mat4(1.0f), v1);
	bb1.setTransforms(t1 * r1);

	glm::mat4 r2 = glm::mat4_cast(o2);
	glm::mat4 t2 = glm::translate(glm::mat4(1.0f), v2);
	bb2.setTransforms(t2 * r2);

	Manifold manifold(&bb1, &bb2);
	FineCollisionDetector fineCollisionDetector(
		kMinFDifference, kMaxIterations,
		kContactPrecision, kContactSeparation
	);

	ASSERT_FALSE(fineCollisionDetector.collide(manifold));
}

TEST(FineCollisionDetector, CPolyCPolyVertexFace)
{
	const glm::vec3 expectedWorldPos[] = {
		{ -3.471183140f, 4.671001170f, -2.168255635f },
		{ -3.471179485f, 4.671000003f, -2.168259382f }
	};
	const glm::vec3 expectedLocalPos[] = {
		{ -0.219993725f, 1.0f, 0.720000408f },
		{ 0.5f, 0.125f, -0.25f }
	};
	const glm::vec3 expectedNormal(-0.679432451f, 0.211933821f, 0.702463984f);
	const float expectedPenetration = 0.000005355f;
	const glm::vec3 v1(-2.787537574f, 5.180943965f, -3.084435224f), v2(-3.950720071f, 4.450982570f, -1.945194125f);
	const glm::quat o1(0.770950198f, 0.507247209f, -0.107715316f, 0.369774848f), o2(0.550417125f, -0.692481637f, -0.259043514f, 0.387822926f);
	BoundingBox bb1({ 1.0f, 2.0f, 2.0f }), bb2({ 1.0f, 0.25f, 0.5f });

	glm::mat4 r1 = glm::mat4_cast(o1);
	glm::mat4 t1 = glm::translate(glm::mat4(1.0f), v1);
	bb1.setTransforms(t1 * r1);

	glm::mat4 r2 = glm::mat4_cast(o2);
	glm::mat4 t2 = glm::translate(glm::mat4(1.0f), v2);
	bb2.setTransforms(t2 * r2);

	Manifold manifold(&bb1, &bb2);
	FineCollisionDetector fineCollisionDetector(
		kMinFDifference, kMaxIterations,
		kContactPrecision, kContactSeparation
	);

	ASSERT_TRUE(fineCollisionDetector.collide(manifold));
	ASSERT_EQ(static_cast<int>(manifold.contacts.size()), 1);

	const Contact& res = manifold.contacts.front();
	EXPECT_NEAR(res.penetration, expectedPenetration, kTolerance);
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(res.normal[i], expectedNormal[i], kTolerance);
		for (int j = 0; j < 2; ++j) {
			EXPECT_NEAR(res.worldPosition[j][i], expectedWorldPos[j][i], kTolerance);
			EXPECT_NEAR(res.localPosition[j][i], expectedLocalPos[j][i], kTolerance);
		}
	}
}


TEST(FineCollisionDetector, CPolyCPolyVertexVertex)
{
	const glm::vec3 expectedWorldPos[] = {
		{ 2.647833347f, 1.175995111f, 0.072492107f },
		{ 2.647833347f, 1.175995111f, 0.072492107f }
	};
	const glm::vec3 expectedLocalPos[] = {
		{ 0.5f, -1.1f, -1.0f },
		{ -1.0f, -0.6f, -0.025f }
	};
	const glm::vec3 expectedNormal(0.866025448f, -0.369224906f, 0.337154180f);
	const float expectedPenetration = 0.0f;
	const glm::vec3 v1(2.764820814f, 2.738384008f, 0.0f), v2(3.065070390f, 0.126421570f, 0.363925665f);
	const glm::quat o1(0.900554239f, -0.349306106f, -0.093596287f, -0.241302788f), o2(0.637856543f, -0.079467326f, -0.094705462f, -0.760167777f);
	BoundingBox bb1({ 1.0f, 2.2f, 2.0f }), bb2({ 2.0f, 1.2f, 0.05f });

	glm::mat4 r1 = glm::mat4_cast(o1);
	glm::mat4 t1 = glm::translate(glm::mat4(1.0f), v1);
	bb1.setTransforms(t1 * r1);

	glm::mat4 r2 = glm::mat4_cast(o2);
	glm::mat4 t2 = glm::translate(glm::mat4(1.0f), v2);
	bb2.setTransforms(t2 * r2);

	Manifold manifold(&bb1, &bb2);
	FineCollisionDetector fineCollisionDetector(
		kMinFDifference, kMaxIterations,
		kContactPrecision, kContactSeparation
	);

	ASSERT_TRUE(fineCollisionDetector.collide(manifold));
	ASSERT_EQ(static_cast<int>(manifold.contacts.size()), 1);

	const Contact& res = manifold.contacts.front();
	EXPECT_NEAR(res.penetration, expectedPenetration, kTolerance);
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(res.normal[i], expectedNormal[i], kTolerance);
		for (int j = 0; j < 2; ++j) {
			EXPECT_NEAR(res.worldPosition[j][i], expectedWorldPos[j][i], kTolerance);
			EXPECT_NEAR(res.localPosition[j][i], expectedLocalPos[j][i], kTolerance);
		}
	}
}


TEST(FineCollisionDetector, SphereCPoly1)
{
	const glm::vec3 expectedWorldPos[] = {
		{ 14.383758805f, -7.014018252f, 8.181325494f },
		{ 14.315420150f, -6.879015922f, 8.099131584f }
	};
	const glm::vec3 expectedLocalPos[] = {
		{ 0.883758805f, -1.764018252f, 1.081325589f },
		{ 1.0f, 0.0f, 0.0f }
	};
	const glm::vec3 expectedNormal(0.396864354f, -0.784001648f, 0.477326035f);
	const float expectedPenetration = 0.172196496f;
	const float radius = 2.25f;
	const glm::vec3 v1(13.5f, -5.25f, 7.1f), v2(14.67f, -7.62f, 8.667f);
	const glm::quat o1(1.0f, glm::vec3(0.0f)), o2(0.473f, -0.313f, 0.057f, 0.821f);

	BoundingSphere bs1(radius);
	glm::mat4 r1 = glm::mat4_cast(o1);
	glm::mat4 t1 = glm::translate(glm::mat4(1.0f), v1);
	bs1.setTransforms(t1 * r1);

	ConvexPolyhedron cp1( createTestPolyhedron3() );
	glm::mat4 r2 = glm::mat4_cast(o2);
	glm::mat4 t2 = glm::translate(glm::mat4(1.0f), v2);
	cp1.setTransforms(t2 * r2);

	Manifold manifold(&bs1, &cp1);
	FineCollisionDetector fineCollisionDetector(
		kMinFDifference, kMaxIterations,
		kContactPrecision, kContactSeparation
	);

	ASSERT_TRUE(fineCollisionDetector.collide(manifold));
	ASSERT_EQ(static_cast<int>(manifold.contacts.size()), 1);

	const Contact& res = manifold.contacts.front();
	EXPECT_NEAR(res.penetration, expectedPenetration, kTolerance);
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(res.normal[i], expectedNormal[i], kTolerance);
		for (int j = 0; j < 2; ++j) {
			EXPECT_NEAR(res.worldPosition[j][i], expectedWorldPos[j][i], kTolerance);
			EXPECT_NEAR(res.localPosition[j][i], expectedLocalPos[j][i], kTolerance);
		}
	}
}


TEST(FineCollisionDetector, TriangleCPoly1)
{
	TriangleCollider tr1({
		glm::vec3( 0.072549045f, 0.107843161f, -0.158823520f ),
		glm::vec3( 0.072549045f, 0.111764729f, -0.154901952f ),
		glm::vec3( 0.068627476f, 0.088235318f, -0.154901952f )
	});
	glm::mat4 transforms2 = glm::scale(glm::mat4(1.0f), { 100.0f, 10.0f, 100.0f });
	tr1.setTransforms(transforms2);

	BoundingBox bb1({ 1.0f, 1.0f, 1.0f });
	glm::mat4 transforms1{
		{0.165384650f, -0.909461260f, 0.381481737f, 0.0f},
		{0.909961343f, -0.008435368f, -0.414607644f, 0.0f},
		{0.380287439f, 0.415703356f, 0.826179266f, 0.0f},
		{6.803552150f, 1.749064920f, -15.065380100f, 1.0f}
	};
	bb1.setTransforms(transforms1);

	Manifold manifold(&tr1, &bb1);
	FineCollisionDetector fineCollisionDetector(
		kMinFDifference, kMaxIterations,
		kContactPrecision, kContactSeparation
	);

	ASSERT_FALSE(fineCollisionDetector.collide(manifold));
}
