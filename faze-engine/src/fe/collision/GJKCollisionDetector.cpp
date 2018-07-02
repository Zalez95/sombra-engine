#include <cassert>
#include <glm/gtc/random.hpp>
#include <glm/gtc/epsilon.hpp>
#include "fe/collision/ConvexCollider.h"
#include "fe/collision/GJKCollisionDetector.h"

namespace fe { namespace collision {

	std::pair<bool, GJKCollisionDetector::SupportPointVector> GJKCollisionDetector::calculate(
		const ConvexCollider& collider1, const ConvexCollider& collider2
	) const
	{
		// 1. Get an initial point in the direction from one collider to another
		glm::vec3 c1Location = collider1.getTransforms() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		glm::vec3 c2Location = collider2.getTransforms() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		glm::vec3 direction = c2Location - c1Location;
		SupportPointVector simplex = { SupportPoint(collider1, collider2, direction) };

		bool containsOrigin = doSimplex(simplex, direction);
		while (!containsOrigin) {
			// 2. Get a support point along the current direction
			SupportPoint sp(collider1, collider2, direction);

			// 3. Check if the support point is further along the search direction
			if (glm::dot(sp.getCSOPosition(), direction) < -mEpsilon) {
				// 4.1 There is no collision, exit without finishing the simplex
				return std::make_pair(false, simplex);
			}
			else {
				// 4.2 Add the point and update the simplex
				simplex.push_back(sp);
				containsOrigin = doSimplex(simplex, direction);
			}
		}

		return std::make_pair(true, simplex);
	}

// Private functions
	bool GJKCollisionDetector::doSimplex(
		SupportPointVector& simplex, glm::vec3& searchDir
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
		SupportPointVector& simplex, glm::vec3& searchDir
	) const
	{
		bool ret = false;

		SupportPoint a = simplex[0];
		glm::vec3 ao = -a.getCSOPosition();

		if (!glm::all( glm::epsilonEqual(ao, glm::vec3(0.0f), mEpsilon) )) {
			// Mantain a and search in the direction to the origin
			simplex = { a };
			searchDir = ao;
		}
		else {
			// The support point is the origin
			ret = true;
		}

		return ret;
	}


	bool GJKCollisionDetector::doSimplex1D(
		SupportPointVector& simplex, glm::vec3& searchDir
	) const
	{
		bool ret = false;

		SupportPoint b = simplex[0], a = simplex[1];
		glm::vec3 ab = b.getCSOPosition() - a.getCSOPosition(), ao = -a.getCSOPosition(),
			abo = glm::cross(ab, ao);

		float dot = glm::dot(ab, ao);
		if (dot < -mEpsilon) {
			// The origin is outside the segment between b and a
			// Discard b and do the same than with 0 dimensions
			simplex = { a };
			ret = doSimplex0D(simplex, searchDir);
		}
		else {
			// The origin is between b and a
			if (!glm::all( glm::epsilonEqual(abo, glm::vec3(0.0f), mEpsilon) )) {
				// Mantain a and b and search towards a normal vector to the
				// ab segment
				simplex = { b, a };
				searchDir = glm::cross(abo, ab);
			}
			else {
				// The origin is on the segment
				ret = true;
			}
		}

		return ret;
	}


	bool GJKCollisionDetector::doSimplex2D(
		SupportPointVector& simplex, glm::vec3& searchDir
	) const
	{
		bool ret = false;

		SupportPoint c = simplex[0], b = simplex[1], a = simplex[2];
		glm::vec3	ab = b.getCSOPosition() - a.getCSOPosition(), ac = c.getCSOPosition() - a.getCSOPosition(),
					ao = -a.getCSOPosition(),
					abc = glm::cross(ab, ac);

		if (glm::dot(glm::cross(ab, abc), ao) > mEpsilon) {
			// The origin is outside the triangle from the ab edge
			// Discard c point and test the edge in 1 dimension
			simplex = { b, a };
			ret = doSimplex1D(simplex, searchDir);
		}
		else if (glm::dot(glm::cross(abc, ac), ao) > mEpsilon) {
			// The origin is outside the triangle from the ac edge
			// Discard b point and test the edge in 1 dimension
			simplex = { c, a };
			ret = doSimplex1D(simplex, searchDir);
		}
		else {
			// The origin is inside the triangle in 2D
			float dot = glm::dot(abc, ao);
			if (dot > mEpsilon) {
				// The origin is above the triangle
				simplex = { c, b, a };
				searchDir = abc;
			}
			else if (dot < -mEpsilon) {
				// The origin is below the triangle
				simplex = { b, c, a };
				searchDir = -abc;
			}
			else {
				// The origin is on the triangle
				ret = true;
			}
		}

		return ret;
	}


	bool GJKCollisionDetector::doSimplex3D(
		SupportPointVector& simplex, glm::vec3& searchDir
	) const
	{
		bool ret = false;

		SupportPoint d = simplex[0], c = simplex[1], b = simplex[2], a = simplex[3];
		glm::vec3	ab = b.getCSOPosition() - a.getCSOPosition(), ac = c.getCSOPosition() - a.getCSOPosition(),
					ad = d.getCSOPosition() - a.getCSOPosition(), ao = -a.getCSOPosition(),
					abc = glm::cross(ab, ac), acd = glm::cross(ac, ad), adb = glm::cross(ad, ab);

		if (glm::dot(abc, ao) > mEpsilon) {
			// The origin is outside the tetrahedron from the abc face
			// Discard d and check the triangle in 2 dimensions
			simplex = { c, b, a };
			ret = doSimplex2D(simplex, searchDir);
		}
		else if (glm::dot(acd, ao) > mEpsilon) {
			// The origin is outside the tetrahedron from the acd face
			// Discard b and check the triangle in 2 dimensions
			simplex = { d, c, a };
			ret = doSimplex2D(simplex, searchDir);
		}	
		else if (glm::dot(adb, ao) > mEpsilon) {
			// The origin is outside the tetrahedron from the adb face
			// Discard c and check the triangle in 2 dimensions
			simplex = { b, d, a };
			ret = doSimplex2D(simplex, searchDir);
		}
		else {
			// The origin is inside the tetrahedron
			ret = true;
		}

		return ret;
	}

}}
