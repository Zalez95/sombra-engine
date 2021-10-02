#include <gtest/gtest.h>
#include <glm/gtc/matrix_transform.hpp>
#include <se/physics/collision/ConvexPolyhedron.h>
#include <se/physics/collision/BoundingSphere.h>
#include <se/physics/collision/CoarseCollisionDetector.h>
#include "TestMeshes.h"

using namespace se::physics;

TEST(CoarseCollisionDetector, collide)
{
	CoarseCollisionDetector ccd;

	HalfEdgeMesh meshData1 = createTestPolyhedron1();
	ConvexPolyhedron cp1(meshData1);
	cp1.setTransforms(glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 4.0f, 4.0f)));
	ccd.submit(&cp1);

	HalfEdgeMesh meshData2 = createTestPolyhedron3();
	ConvexPolyhedron cp2(meshData2);
	cp2.setTransforms(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 4.0f, 4.0f)));
	ccd.submit(&cp2);

	BoundingSphere bs1(1.0f);
	bs1.setTransforms(glm::translate(glm::mat4(1.0f), glm::vec3(4.0f, 7.0f, 5.0f)));
	ccd.submit(&bs1);

	BoundingSphere bs2(1.0f);
	bs2.setTransforms(glm::translate(glm::mat4(1.0f), glm::vec3(4.0f, 3.0f, 5.0f)));
	ccd.submit(&bs2);

	std::set<std::pair<const Collider*, const Collider*>> expectedRes = {
		std::make_pair(&cp1, &cp2),
		std::make_pair(&cp1, &bs2)
	};
	ccd.processIntersectingColliders([&](const std::pair<const Collider*, const Collider*>& pair1) {
		bool flag = false;
		for (auto& pair2 : expectedRes) {
			if ((pair1.first == pair2.first && pair1.second == pair2.second)
				|| (pair1.first == pair2.second && pair1.second == pair2.first)
			) {
				flag = true;
				break;
			}
		}

		EXPECT_TRUE(flag);
	});
}
