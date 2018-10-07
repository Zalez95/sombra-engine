#include <gtest/gtest.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fe/collision/Manifold.h>
#include <fe/collision/BoundingBox.h>
#include <fe/collision/BoundingSphere.h>
#include <fe/collision/FineCollisionDetector.h>
#include "TestMeshes.h"

#define TOLERANCE 0.0001f
#define MIN_F_DIFFERENCE	0.0001f
#define CONTACT_PRECISION	0.0000001f
#define CONTACT_SEPARATION	0.0000001f

TEST(FineCollisionDetector, SphereSphere1)
{
	const glm::vec3 v1(13.5f, -5.25f, 7.1f), v2(0.0f);
	const glm::quat o1(1.0f, 0.0f, 0.0f, 0.0f), o2(0.795f, -0.002f, -0.575f, 0.192f);
	fe::collision::BoundingSphere bs1(2.5f), bs2(5.2f);

	glm::mat4 r1 = glm::mat4_cast(o1);
	glm::mat4 t1 = glm::translate(glm::mat4(1.0f), v1);
	bs1.setTransforms(t1 * r1);

	glm::mat4 r2 = glm::mat4_cast(o2);
	glm::mat4 t2 = glm::translate(glm::mat4(1.0f), v2);
	bs2.setTransforms(t2 * r2);

	fe::collision::Manifold manifold(&bs1, &bs2);
	fe::collision::FineCollisionDetector fineCollisionDetector(
		MIN_F_DIFFERENCE, CONTACT_PRECISION,
		CONTACT_SEPARATION
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
	const glm::vec3 expectedNormal(-0.461796492f, 0.311872839f, -0.830348908f);
	const float expectedPenetration = 0.000000159f;
	const glm::vec3 v1(13.5f, -5.25f, 7.1f), v2(9.943065643f, -2.873334407f, 0.697683811f);
	const glm::quat o1(1.0f, glm::vec3(0.0f)), o2(0.795f, -0.002f, -0.575f, 0.192f);

	fe::collision::BoundingSphere bs1(2.5f), bs2(5.2f);

	glm::mat4 r1 = glm::mat4_cast(o1);
	glm::mat4 t1 = glm::translate(glm::mat4(1.0f), v1);
	bs1.setTransforms(t1 * r1);

	glm::mat4 r2 = glm::mat4_cast(o2);
	glm::mat4 t2 = glm::translate(glm::mat4(1.0f), v2);
	bs2.setTransforms(t2 * r2);

	fe::collision::Manifold manifold(&bs1, &bs2);
	fe::collision::FineCollisionDetector fineCollisionDetector(
		MIN_F_DIFFERENCE, CONTACT_PRECISION,
		CONTACT_SEPARATION
	);

	ASSERT_TRUE(fineCollisionDetector.collide(manifold));
	std::vector<fe::collision::Contact> contacts = manifold.getContacts();
	EXPECT_EQ(static_cast<int>(contacts.size()), 1);

	fe::collision::Contact& res = contacts.front();
	EXPECT_NEAR(res.getPenetration(), expectedPenetration, TOLERANCE);
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(res.getNormal()[i], expectedNormal[i], TOLERANCE);
		for (int j = 0; j < 2; ++j) {
			EXPECT_NEAR(res.getWorldPosition(j)[i], expectedWorldPos[j][i], TOLERANCE);
			EXPECT_NEAR(res.getLocalPosition(j)[i], expectedLocalPos[j][i], TOLERANCE);
		}
	}
}

/** Non colliding */
TEST(FineCollisionDetector, CPolyCPoly1)
{
	const glm::vec3 v1(-5.65946f, -2.8255f, -1.52118f), v2(-4.58841f, -2.39753f, -0.164247f);
	const glm::quat o1(0.890843f, 0.349613f, 0.061734f, 0.283475f), o2(0.962876f, -0.158823f, 0.216784f, -0.025477f);
	fe::collision::BoundingBox bb1(glm::vec3(2.0f, 1.0f, 2.0f)), bb2(glm::vec3(1.0f, 1.0f, 0.5f));

	glm::mat4 r1 = glm::mat4_cast(o1);
	glm::mat4 t1 = glm::translate(glm::mat4(1.0f), v1);
	bb1.setTransforms(t1 * r1);

	glm::mat4 r2 = glm::mat4_cast(o2);
	glm::mat4 t2 = glm::translate(glm::mat4(1.0f), v2);
	bb2.setTransforms(t2 * r2);

	fe::collision::Manifold manifold(&bb1, &bb2);
	fe::collision::FineCollisionDetector fineCollisionDetector(
		MIN_F_DIFFERENCE, CONTACT_PRECISION,
		CONTACT_SEPARATION
	);

	ASSERT_FALSE(fineCollisionDetector.collide(manifold));
}

/** Colliding - vertex face */
TEST(FineCollisionDetector, CPolyCPoly2)
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
	fe::collision::BoundingBox bb1(glm::vec3(1.0f, 2.0f, 2.0f)), bb2(glm::vec3(1.0f, 0.25f, 0.5f));

	glm::mat4 r1 = glm::mat4_cast(o1);
	glm::mat4 t1 = glm::translate(glm::mat4(1.0f), v1);
	bb1.setTransforms(t1 * r1);

	glm::mat4 r2 = glm::mat4_cast(o2);
	glm::mat4 t2 = glm::translate(glm::mat4(1.0f), v2);
	bb2.setTransforms(t2 * r2);

	fe::collision::Manifold manifold(&bb1, &bb2);
	fe::collision::FineCollisionDetector fineCollisionDetector(
		MIN_F_DIFFERENCE, CONTACT_PRECISION,
		CONTACT_SEPARATION
	);

	ASSERT_TRUE(fineCollisionDetector.collide(manifold));
	std::vector<fe::collision::Contact> contacts = manifold.getContacts();
	EXPECT_EQ(static_cast<int>(contacts.size()), 1);

	fe::collision::Contact& res = contacts.front();
	EXPECT_NEAR(res.getPenetration(), expectedPenetration, TOLERANCE);
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(res.getNormal()[i], expectedNormal[i], TOLERANCE);
		for (int j = 0; j < 2; ++j) {
			EXPECT_NEAR(res.getWorldPosition(j)[i], expectedWorldPos[j][i], TOLERANCE);
			EXPECT_NEAR(res.getLocalPosition(j)[i], expectedLocalPos[j][i], TOLERANCE);
		}
	}
}

/** Colliding vertex - vertex */
TEST(FineCollisionDetector, CPolyCPoly3)
{
	const glm::vec3 expectedWorldPos[] = {
		{ 2.647833347f, 1.175995111f, 0.072492107f },
		{ 2.647833347f, 1.175995111f, 0.072492107f }
	};
	const glm::vec3 expectedLocalPos[] = {
		{ 0.5f, -1.1f, -1.0f },
		{ -1.0f, -0.6f, -0.025f }
	};
	const glm::vec3 expectedNormal(0.866025507f, -0.369224995f, 0.337154150f);
	const float expectedPenetration = 0.0f;
	const glm::vec3 v1(2.764820814f, 2.738384008f, 0.0f), v2(3.065070390f, 0.126420855f, 0.363925933f);
	const glm::quat o1(0.900554239f, -0.349306106f, -0.093596287f, -0.241302788f), o2(0.637856543f, -0.079467326f, -0.094705462f, -0.760167777f);
	fe::collision::BoundingBox bb1(glm::vec3(1.0f, 2.2f, 2.0f)), bb2(glm::vec3(2.0f, 1.2f, 0.05f));

	glm::mat4 r1 = glm::mat4_cast(o1);
	glm::mat4 t1 = glm::translate(glm::mat4(1.0f), v1);
	bb1.setTransforms(t1 * r1);

	glm::mat4 r2 = glm::mat4_cast(o2);
	glm::mat4 t2 = glm::translate(glm::mat4(1.0f), v2);
	bb2.setTransforms(t2 * r2);

	fe::collision::Manifold manifold(&bb1, &bb2);
	fe::collision::FineCollisionDetector fineCollisionDetector(
		MIN_F_DIFFERENCE, CONTACT_PRECISION,
		CONTACT_SEPARATION
	);

	ASSERT_TRUE(fineCollisionDetector.collide(manifold));
	std::vector<fe::collision::Contact> contacts = manifold.getContacts();
	EXPECT_EQ(static_cast<int>(contacts.size()), 1);

	fe::collision::Contact& res = contacts.front();
	EXPECT_NEAR(res.getPenetration(), expectedPenetration, TOLERANCE);
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(res.getNormal()[i], expectedNormal[i], TOLERANCE);
		for (int j = 0; j < 2; ++j) {
			EXPECT_NEAR(res.getWorldPosition(j)[i], expectedWorldPos[j][i], TOLERANCE);
			EXPECT_NEAR(res.getLocalPosition(j)[i], expectedLocalPos[j][i], TOLERANCE);
		}
	}
}


TEST(FineCollisionDetector, SphereCPoly1)
{
	const glm::vec3 expectedWorldPos[] = {
		{ 14.383110774f, -7.013699324f, 8.182463250f },
		{ 14.315420150f, -6.879015922f, 8.099131584f }
	};
	const glm::vec3 expectedLocalPos[] = {
		{ 0.883110774f, -1.763699324f, 1.082463345f },
		{ 1.0f, 0.0f, 0.0f }
	};
	const glm::vec3 expectedNormal(0.393006712f, -0.781962514f, 0.483818501f);
	const float expectedPenetration = 0.172238088f;
	const float radius = 2.25f;
	const glm::vec3 v1(13.5f, -5.25f, 7.1f), v2(14.67f, -7.62f, 8.667f);
	const glm::quat o1(1.0f, glm::vec3(0.0f)), o2(0.473f, -0.313f, 0.057f, 0.821f);

	fe::collision::BoundingSphere bs1(radius);
	glm::mat4 r1 = glm::mat4_cast(o1);
	glm::mat4 t1 = glm::translate(glm::mat4(1.0f), v1);
	bs1.setTransforms(t1 * r1);

	fe::collision::ConvexPolyhedron cp1( createTestPolyhedron3() );
	glm::mat4 r2 = glm::mat4_cast(o2);
	glm::mat4 t2 = glm::translate(glm::mat4(1.0f), v2);
	cp1.setTransforms(t2 * r2);

	fe::collision::Manifold manifold(&bs1, &cp1);
	fe::collision::FineCollisionDetector fineCollisionDetector(
		MIN_F_DIFFERENCE, CONTACT_PRECISION,
		CONTACT_SEPARATION
	);

	ASSERT_TRUE(fineCollisionDetector.collide(manifold));
	std::vector<fe::collision::Contact> contacts = manifold.getContacts();
	EXPECT_EQ(static_cast<int>(contacts.size()), 1);

	fe::collision::Contact& res = contacts.front();
	EXPECT_NEAR(res.getPenetration(), expectedPenetration, TOLERANCE);
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(res.getNormal()[i], expectedNormal[i], TOLERANCE);
		for (int j = 0; j < 2; ++j) {
			EXPECT_NEAR(res.getWorldPosition(j)[i], expectedWorldPos[j][i], TOLERANCE);
			EXPECT_NEAR(res.getLocalPosition(j)[i], expectedLocalPos[j][i], TOLERANCE);
		}
	}
}
