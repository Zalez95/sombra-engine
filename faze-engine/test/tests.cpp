#include <fe/collision/BoundingSphere.h>
#include <gtest/gtest.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

TEST(BoundingSphere, getAABB) {
	const fe::collision::BoundingSphere bs1(2.0f);
	const fe::collision::AABB aabb1 = bs1.getAABB();
	EXPECT_EQ(aabb1.mMinimum, glm::vec3(-2.0f));
	EXPECT_EQ(aabb1.mMaximum, glm::vec3(2.0f));

	const fe::collision::BoundingSphere bs2(0.0f);
	const fe::collision::AABB aabb2 = bs2.getAABB();
	EXPECT_EQ(aabb2.mMinimum, glm::vec3(0.0f));
	EXPECT_EQ(aabb2.mMaximum, glm::vec3(0.0f));
}


int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
