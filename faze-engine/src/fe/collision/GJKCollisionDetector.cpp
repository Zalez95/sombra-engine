#include "fe/collision/GJKCollisionDetector.h"
#include <cassert>
#include <glm/gtc/random.hpp>
#include "fe/collision/ConvexCollider.h"

namespace fe { namespace collision {

	bool GJKCollisionDetector::calculate(
		const ConvexCollider& collider1, const ConvexCollider& collider2
	) {
		// 1. Get an arbitrary point
		glm::vec3 direction = glm::sphericalRand(1.0f);
		mSimplex = { SupportPoint(collider1, collider2, direction) };

		bool flag = !doSimplex(mSimplex, direction);
		while (flag) {
			// 2. Get a support point along the current direction
			SupportPoint sp(collider1, collider2, direction);

			// 3. Check if the support point is further along the search direction
			if (glm::dot(sp.getCSOPosition(), direction) > 0) {
				// 4.1 Add the point and update the simplex
				mSimplex.push_back(sp);
				flag = !doSimplex(mSimplex, direction);
			}
			else {
				// 4.2 There is no collision, exit without finishing the simplex
				return false;
			}
		}

		return true;
	}

// Private functions
	bool GJKCollisionDetector::doSimplex(
		std::vector<SupportPoint>& simplex,
		glm::vec3& searchDir
	) const
	{
		assert(!simplex.empty() && "The simplex has to have at least one initial point");

		bool ret;
		switch (simplex.size() - 1) {
			case 0:
				ret = doSimplex0D(simplex, searchDir);
				break;
			case 1:
				ret = doSimplex1D(simplex, searchDir);
				break;
			case 2:
				ret = doSimplex2D(simplex, searchDir);
				break;
			case 3:
				ret = doSimplex3D(simplex, searchDir);
				break;
			default:
				ret = false;
		}

		return ret;
	}


	bool GJKCollisionDetector::doSimplex0D(
		std::vector<SupportPoint>& simplex,
		glm::vec3& searchDir
	) const
	{
		bool ret = false;

		SupportPoint a = simplex[0];
		simplex = { a };
		searchDir = -a.getCSOPosition();

		// Check if the support point is the origin
		ret = (a.getCSOPosition() == glm::vec3(0.0f));

		return ret;
	}


	bool GJKCollisionDetector::doSimplex1D(
		std::vector<SupportPoint>& simplex,
		glm::vec3& searchDir
	) const
	{
		bool ret = false;

		SupportPoint b = simplex[0], a = simplex[1];
		glm::vec3 ab = b.getCSOPosition() - a.getCSOPosition(), ao = -a.getCSOPosition();

		float dot = glm::dot(ab, ao);
		if (dot >= 0) {
			// The origin is between b and a
			simplex = { b, a };
			searchDir = glm::cross(glm::cross(ab, ao), ab);

			// Check if the origin is on the line
			ret = (dot == 0.0f);
		}
		else {
			// Discard b and do the same than with 0 dimensions
			simplex = { a };
			ret = doSimplex0D(simplex, searchDir);
		}

		return ret;
	}


	bool GJKCollisionDetector::doSimplex2D(
		std::vector<SupportPoint>& simplex,
		glm::vec3& searchDir
	) const
	{
		bool ret = false;

		SupportPoint c = simplex[0], b = simplex[1], a = simplex[2];
		glm::vec3	ab = b.getCSOPosition() - a.getCSOPosition(), ac = c.getCSOPosition() - a.getCSOPosition(),
					ao = -a.getCSOPosition(),
					abc = glm::cross(ab, ac);

		if (glm::dot(glm::cross(ab, abc), ao) > 0) {
			// Origin outside the triangle from the ab edge
			// Discard c point and test the edge in 1 dimension
			simplex = { b, a };
			ret = doSimplex1D(simplex, searchDir);
		}
		else {
			if (glm::dot(glm::cross(abc, ac), ao) > 0) {
				// Origin outside the triangle from the ac edge
				// Discard b point and test the edge in 1 dimension
				simplex = { c, a };
				ret = doSimplex1D(simplex, searchDir);
			}
			else {
				// Inside the triangle in 2D
				// Check if the origin is above or below the triangle
				float dot = glm::dot(abc, ao);
				if (dot >= 0) {
					simplex = { c, b, a };
					searchDir = abc;

					// Check if the origin is on the triangle
					ret = (dot == 0.0f);
				}
				else {
					simplex = { b, c, a };
					searchDir = -abc;
				}
			}
		}

		return ret;
	}


	bool GJKCollisionDetector::doSimplex3D(
		std::vector<SupportPoint>& simplex,
		glm::vec3& searchDir
	) const
	{
		bool ret = false;

		SupportPoint d = simplex[0], c = simplex[1], b = simplex[2], a = simplex[3];
		glm::vec3	ab = b.getCSOPosition() - a.getCSOPosition(), ac = c.getCSOPosition() - a.getCSOPosition(),
					ad = d.getCSOPosition() - a.getCSOPosition(), ao = -a.getCSOPosition(),
					abc = glm::cross(ab, ac), acd = glm::cross(ac, ad), adb = glm::cross(ad, ab);

		if (glm::dot(abc, ao) > 0) {
			// Origin outside the tetrahedron from the abc face
			// Discard d and check the triangle in 2 dimensions
			simplex = { c, b, a };
			ret = doSimplex2D(simplex, searchDir);
		}
		else {
			if (glm::dot(acd, ao) > 0) {
				// Origin outside the tetrahedron from the acd face
				// Discard b and check the triangle in 2 dimensions
				simplex = { d, c, a };
				ret = doSimplex2D(simplex, searchDir);
			}
			else {
				if (glm::dot(adb, ao) > 0) {
					// Origin outside the tetrahedron from the adb face
					// Discard c and check the triangle in 2 dimensions
					simplex = { b, d, a };
					ret = doSimplex2D(simplex, searchDir);
				}
				else {
					// Inside the tetrahedron
					ret = true;
				}
			}
		}

		return ret;
	}

}}
