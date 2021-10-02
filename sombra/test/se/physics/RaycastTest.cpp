#if 0
//TODO:
#include <gtest/gtest.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <se/physics/collision/BoundingBox.h>
#include <se/physics/collision/BoundingSphere.h>
#include <se/physics/collision/TriangleCollider.h>
#include <se/physics/collision/GJKRayCaster.h>
#include "TestMeshes.h"

using namespace se::physics;
static constexpr float kContactPrecision = 0.0000001f;
static constexpr int kMaxIterations = 32;

TEST(Raycast, Cube1)
{
	glm::quat rayOrientation(0.032990694f, 0.853827714f, 0.276134550f, -0.440044879f);
	glm::vec3 rayOrigin(-46.464401245f, 1.976649999f, -36.986301422f);

	glm::vec3 p1(-49.965099334f, 1.075446844f, -39.965072631f);
	glm::quat o1(0.996992051f, 0.054803427f, -0.000430180f, -0.054803423f);
	BoundingBox bb1(glm::vec3(1.0f));
	glm::mat4 r1 = glm::mat4_cast(o1);
	glm::mat4 t1 = glm::translate(glm::mat4(1.0f), p1);
	bb1.setTransforms(t1 * r1);

	GJKRayCaster gjk(kContactPrecision, kMaxIterations);
	bool res = gjk.calculateRayCast(rayOrigin, rayOrientation * glm::vec3(0.0f, 0.0f, 1.0f), bb1).first;

	ASSERT_TRUE(res);
}
#endif