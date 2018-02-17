#include <gtest/gtest.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fe/collision/BoundingBox.h>
#include <fe/collision/BoundingSphere.h>
#include <fe/collision/GJKCollisionDetector.h>

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

	fe::collision::GJKCollisionDetector gjk;
	EXPECT_FALSE(gjk.calculate(bs1, bs2));
}


TEST(GJKCollisionDetector, SphereSphere2)
{
	glm::vec3 v1(13.5f, -5.25f, 7.1f), v2(9.943065643f, -2.873334407f, 0.697683811f);
	glm::quat o1(1.0f, glm::vec3(0.0f)), o2(0.795f, -0.002f, -0.575f, 0.192f);
	fe::collision::BoundingSphere bs1(2.5f), bs2(5.2f);

	glm::mat4 r1 = glm::mat4_cast(o1);
	glm::mat4 t1 = glm::translate(glm::mat4(1.0f), v1);
	bs1.setTransforms(t1 * r1);

	glm::mat4 r2 = glm::mat4_cast(o2);
	glm::mat4 t2 = glm::translate(glm::mat4(1.0f), v2);
	bs2.setTransforms(t2 * r2);

	fe::collision::GJKCollisionDetector gjk;
	EXPECT_TRUE(gjk.calculate(bs1, bs2));
}


TEST(GJKCollisionDetector, SphereSphere3)
{
	glm::vec3 v1(13.5f, -5.25f, 7.1f), v2(13.5f, -5.25f, 7.1f);
	glm::quat o1(1.0f, glm::vec3(0.0f)), o2(0.795f, -0.002f, -0.575f, 0.192f);
	fe::collision::BoundingSphere bs1(2.5f), bs2(5.2f);

	glm::mat4 r1 = glm::mat4_cast(o1);
	glm::mat4 t1 = glm::translate(glm::mat4(1.0f), v1);
	bs1.setTransforms(t1 * r1);

	glm::mat4 r2 = glm::mat4_cast(o2);
	glm::mat4 t2 = glm::translate(glm::mat4(1.0f), v2);
	bs2.setTransforms(t2 * r2);

	fe::collision::GJKCollisionDetector gjk;
	EXPECT_TRUE(gjk.calculate(bs1, bs2));
}


TEST(GJKCollisionDetector, CVXPolyCVXPoly1)
{
	glm::vec3 v1(-5.65946f, -2.8255f, -1.52118f), v2(-4.58841f, -2.39753f, -0.164247f);
	glm::quat o1(0.890843f, 0.349613f, 0.061734f, 0.283475f), o2(0.962876f, -0.158823f, 0.216784f, -0.025477f);
	fe::collision::BoundingBox bb1(glm::vec3(2.0f, 1.0f, 2.0f)), bb2(glm::vec3(1.0f, 1.0f, 0.5f));

	glm::mat4 r1 = glm::mat4_cast(o1);
	glm::mat4 t1 = glm::translate(glm::mat4(1.0f), v1);
	bb1.setTransforms(t1 * r1);

	glm::mat4 r2 = glm::mat4_cast(o2);
	glm::mat4 t2 = glm::translate(glm::mat4(1.0f), v2);
	bb2.setTransforms(t2 * r2);

	fe::collision::GJKCollisionDetector gjk;
	EXPECT_FALSE(gjk.calculate(bb1, bb2));
}


TEST(GJKCollisionDetector, CVXPolyCVXPoly2)
{
	glm::vec3 v1(-2.787537574f, 5.180943965f, -3.084435224f), v2(-3.950720071f, 4.450982570f, -1.945194125f);
	glm::quat o1(0.770950198f, 0.507247209f, -0.107715316f, 0.369774848f), o2(0.550417125f, -0.692481637f, -0.259043514f, 0.387822926f);
	fe::collision::BoundingBox bb1(glm::vec3(1.0f, 2.0f, 2.0f)), bb2(glm::vec3(1.0f, 0.25f, 0.5f));

	glm::mat4 r1 = glm::mat4_cast(o1);
	glm::mat4 t1 = glm::translate(glm::mat4(1.0f), v1);
	bb1.setTransforms(t1 * r1);

	glm::mat4 r2 = glm::mat4_cast(o2);
	glm::mat4 t2 = glm::translate(glm::mat4(1.0f), v2);
	bb2.setTransforms(t2 * r2);

	fe::collision::GJKCollisionDetector gjk;
	EXPECT_TRUE(gjk.calculate(bb1, bb2));
}
