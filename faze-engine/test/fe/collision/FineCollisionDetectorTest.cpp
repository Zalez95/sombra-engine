#include <gtest/gtest.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fe/collision/BoundingBox.h>
#include <fe/collision/BoundingSphere.h>
#include <fe/collision/Manifold.h>
#include <fe/collision/FineCollisionDetector.h>

#define TOLERANCE 0.000001f


TEST(GJKCollisionDetector, SphereSphere1)
{
	glm::vec3 v1(13.5f, -5.25f, 7.1f), v2(0.0f);
	glm::quat o1(1.0f, 0.0f, 0.0f, 0.0f), o2(0.795f, -0.002f, -0.575f, 0.192f);
	fe::collision::BoundingSphere bs1(2.5f), bs2(5.2f);

	glm::mat4 r1 = glm::mat4_cast(o1);
	glm::mat4 t1 = glm::translate(glm::mat4(1.0f), v1);
	bs1.setTransforms(t1 * r1);

	glm::mat4 r2 = glm::mat4_cast(o2);
	glm::mat4 t2 = glm::translate(glm::mat4(1.0f), v2);
	bs2.setTransforms(t2 * r2);

	fe::collision::Manifold manifold(&bs1, &bs2);
	fe::collision::FineCollisionDetector fineCollisionDetector;

	EXPECT_FALSE(fineCollisionDetector.collide(&bs1, &bs2, manifold));
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
	fe::collision::FineCollisionDetector fineCollisionDetector;

	EXPECT_TRUE(fineCollisionDetector.collide(&bs1, &bs2, manifold));
 	std::vector<fe::collision::Contact> contacts = manifold.getContacts();
	EXPECT_EQ(static_cast<int>(contacts.size()), 1);

	fe::collision::Contact& res = contacts.front();
	EXPECT_LE(abs(res.getPenetration() - expectedPenetration), TOLERANCE);
	for (int i = 0; i < 3; ++i) {
		EXPECT_LE(abs(res.getNormal()[i] - expectedNormal[i]), TOLERANCE);
		for (int j = 0; j < 2; ++j) {
			EXPECT_LE(abs(res.getWorldPosition(j)[i] - expectedWorldPos[j][i]), TOLERANCE);
			EXPECT_LE(abs(res.getLocalPosition(j)[i] - expectedLocalPos[j][i]), TOLERANCE);
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
	fe::collision::FineCollisionDetector fineCollisionDetector;

	EXPECT_TRUE(fineCollisionDetector.collide(&bs1, &bs2, manifold));
 	std::vector<fe::collision::Contact> contacts = manifold.getContacts();
	EXPECT_EQ(static_cast<int>(contacts.size()), 1);
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
	fe::collision::FineCollisionDetector fineCollisionDetector;

	EXPECT_FALSE(fineCollisionDetector.collide(&bb1, &bb2, manifold));
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
	fe::collision::FineCollisionDetector fineCollisionDetector;

	EXPECT_TRUE(fineCollisionDetector.collide(&bb1, &bb2, manifold));
 	std::vector<fe::collision::Contact> contacts = manifold.getContacts();
	EXPECT_EQ(static_cast<int>(contacts.size()), 1);

	fe::collision::Contact& res = contacts.front();
	EXPECT_LE(abs(res.getPenetration() - expectedPenetration), TOLERANCE);
	for (int i = 0; i < 3; ++i) {
		EXPECT_LE(abs(res.getNormal()[i] - expectedNormal[i]), TOLERANCE);
		for (int j = 0; j < 2; ++j) {
			EXPECT_LE(abs(res.getWorldPosition(j)[i] - expectedWorldPos[j][i]), TOLERANCE);
			EXPECT_LE(abs(res.getLocalPosition(j)[i] - expectedLocalPos[j][i]), TOLERANCE);
		}
	}
	EXPECT_LE(abs(res.getPenetration() - expectedPenetration), TOLERANCE);
}
