#include <gtest/gtest.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fe/collision/BoundingBox.h>
#include <fe/collision/BoundingSphere.h>
#include <fe/collision/MeshCollider.h>
#include <fe/collision/Manifold.h>
#include <fe/collision/FineCollisionDetector.h>

#define TOLERANCE 0.000001f
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
		{ 12.345150947f, -4.478355407f, 5.021325111f },
		{ 12.345151901f, -4.478355407f, 5.021327018f }
	};
	const glm::vec3 expectedLocalPos[] = {
		{ -1.154849052f, 0.771644830f, -2.078674554f },
		{ 4.094133853f, -3.184389114f, -0.388609051f }
	};
	const glm::vec3 expectedNormal(-0.461939632f, 0.308657944f, -0.831469833f);
	const float expectedPenetration = glm::length(expectedWorldPos[1] - expectedWorldPos[0]);
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


TEST(FineCollisionDetector, SphereSphere3)
{
	const glm::vec3 v1(15.0f, -7.0f, 6.0f), v2(13.5f, -5.25f, 7.1f);
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
	// TODO: complete
}


TEST(FineCollisionDetector, CVXPolyCVXPoly1)
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


TEST(FineCollisionDetector, CVXPolyCVXPoly2)
{
	const glm::vec3 expectedWorldPos[] = {
		{ -3.471179485f, 4.671000003f, -2.168259382f },
		{ -3.471183061f, 4.671000957f, -2.168255567f }
	};
	const glm::vec3 expectedLocalPos[] = {
		{ -0.219993710f, 1.000000238f, 0.720000684f },
		{ 0.5f, 0.125f, -0.25f }
	};
	const glm::vec3 expectedNormal(-0.679432392f, 0.211933776f, 0.702463984f);
	const float expectedPenetration = glm::length(expectedWorldPos[1] - expectedWorldPos[0]);
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


TEST(FineCollisionDetector, SphereMesh1)
{
	const glm::vec3 expectedWorldPos[] = {
		{ 15.029119491f, -6.232823848f, 8.426028251f },
		{ 14.939769744f, -6.063839912f, 8.329644203f }
	};
	const glm::vec3 expectedLocalPos[] = {
		{ 1.529119491f, -0.982823848f, 1.326028347f },
		{ 1.249999523f, -0.999999284f, 0.213817119f }
	};
	const glm::vec3 expectedNormal(0.416512221f, -0.789699912f, 0.450434893f);
	const float expectedPenetration = glm::length(expectedWorldPos[1] - expectedWorldPos[0]);
	const float radius = 2.25;
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
	const glm::vec3 v1(13.5f, -5.25f, 7.1f), v2(14.67f, -7.62f, 8.667f);
	const glm::quat o1(1.0f, glm::vec3(0.0f)), o2(0.473f, -0.313f, 0.057f, 0.821f);

	fe::collision::BoundingSphere bs1(radius);
	glm::mat4 r1 = glm::mat4_cast(o1);
	glm::mat4 t1 = glm::translate(glm::mat4(1.0f), v1);
	bs1.setTransforms(t1 * r1);

	fe::collision::MeshCollider mc1(vertices, indices);
	glm::mat4 r2 = glm::mat4_cast(o2);
	glm::mat4 t2 = glm::translate(glm::mat4(1.0f), v2);
	mc1.setTransforms(t2 * r2);

	fe::collision::Manifold manifold(&bs1, &mc1);
	fe::collision::FineCollisionDetector fineCollisionDetector(
		MIN_F_DIFFERENCE, CONTACT_PRECISION,
		CONTACT_SEPARATION
	);

	ASSERT_TRUE(fineCollisionDetector.collide(manifold));
 	std::vector<fe::collision::Contact> contacts = manifold.getContacts();
	EXPECT_EQ(static_cast<int>(contacts.size()), 4);

	auto it = std::max_element(
		contacts.begin(), contacts.end(),
		[](fe::collision::Contact& c1, fe::collision::Contact& c2) { return c1.getPenetration() > c2.getPenetration(); }
	);
	fe::collision::Contact& res = *it;

	EXPECT_NEAR(res.getPenetration(), expectedPenetration, TOLERANCE);
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(res.getNormal()[i], expectedNormal[i], TOLERANCE);
		for (int j = 0; j < 2; ++j) {
			EXPECT_NEAR(res.getWorldPosition(j)[i], expectedWorldPos[j][i], TOLERANCE);
			EXPECT_NEAR(res.getLocalPosition(j)[i], expectedLocalPos[j][i], TOLERANCE);
		}
	}

	// TODO: Compare the rest
}
