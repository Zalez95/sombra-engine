#include <gtest/gtest.h>
#include <glm/gtc/matrix_transform.hpp>
#include <fe/collision/ConvexPolyhedron.h>
#include <fe/collision/BoundingSphere.h>
#include <fe/collision/CoarseCollisionDetector.h>

#define TOLERANCE 0.000000001f


TEST(CoarseCollisionDetector, collide)
{
	fe::collision::CoarseCollisionDetector ccd;

	fe::collision::ConvexPolyhedron cp1({
		glm::vec3( 1.0f,  1.0f, -1.0f),
		glm::vec3( 1.0f, -1.0f, -1.0f),
		glm::vec3(-1.0f, -1.0f, -1.0f),
		glm::vec3(-1.0f,  1.0f, -1.0f),
		glm::vec3( 1.0f,  1.0f,  1.0f),
		glm::vec3( 0.0f, -1.0f,  1.0f),
		glm::vec3(-1.0f, -1.0f,  1.0f),
		glm::vec3(-1.0f,  1.0f,  1.0f)
	});
	cp1.setTransforms(glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 4.0f, 4.0f)));
	ccd.submit(&cp1);

	fe::collision::ConvexPolyhedron cp2({
		glm::vec3(-1.0f, -0.5f, -0.5f),
		glm::vec3(-1.0f, -0.5f,  0.5f),
		glm::vec3(-1.0f,  0.5f, -0.5f),
		glm::vec3(-1.0f,  0.5f,  0.5f),
		glm::vec3( 1.0f,  0.0f,  0.0f)
	});
	cp2.setTransforms(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 4.0f, 4.0f)));
	ccd.submit(&cp2);

	fe::collision::BoundingSphere bs1(1.0f);
	bs1.setTransforms(glm::translate(glm::mat4(1.0f), glm::vec3(4.0f, 7.0f, 5.0f)));
	ccd.submit(&bs1);

	fe::collision::BoundingSphere bs2(1.0f);
	bs2.setTransforms(glm::translate(glm::mat4(1.0f), glm::vec3(4.0f, 3.0f, 5.0f)));
	ccd.submit(&bs2);

	auto result = ccd.getIntersectingColliders();
	std::set<std::pair<fe::collision::Collider*, fe::collision::Collider*>> expectedRes = {
		std::make_pair(&cp1, &cp2),
		std::make_pair(&cp1, &bs2)
	};

	ASSERT_EQ(result.size(), expectedRes.size());
	for (auto pair1 : result) {
		bool flag = false;
		for (auto pair2 : result) {
			if ((pair1.first == pair2.first && pair1.second == pair2.second)
				|| (pair1.first == pair2.second && pair1.second == pair2.first)
			) {
				flag = true;
				break;
			}
		}

		EXPECT_TRUE(flag);
	}
}
