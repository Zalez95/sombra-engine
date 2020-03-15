#include <cassert>
#include <glm/gtc/random.hpp>
#include <glm/gtc/epsilon.hpp>
#include "se/collision/ConvexCollider.h"
#include "se/collision/GJKCollisionDetector.h"

namespace se::collision {

	std::pair<bool, Simplex> GJKCollisionDetector::calculateIntersection(
		const ConvexCollider& collider1, const ConvexCollider& collider2
	) const
	{
		// 1. Get an initial point in the direction from one collider to another
		glm::vec3 c1Location = collider1.getTransforms() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		glm::vec3 c2Location = collider2.getTransforms() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		glm::vec3 direction = (c1Location == c2Location)? glm::sphericalRand(1.0f) : glm::normalize(c2Location - c1Location);
		Simplex simplex = { SupportPoint(collider1, collider2, direction) };

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
	bool GJKCollisionDetector::doSimplex(Simplex& simplex, glm::vec3& searchDir) const
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


	bool GJKCollisionDetector::doSimplex0D(Simplex& simplex, glm::vec3& searchDir) const
	{
		bool ret = false;

		SupportPoint &a = simplex[0];
		glm::vec3 a0 = -a.getCSOPosition();

		if (!glm::all(glm::epsilonEqual(a0, glm::vec3(0.0f), mEpsilon))) {
			// Preserve the current simplex and search in the direction to the
			// origin
			searchDir = glm::normalize(a0);
		}
		else {
			// The support point is the origin
			ret = true;
		}

		return ret;
	}


	bool GJKCollisionDetector::doSimplex1D(Simplex& simplex, glm::vec3& searchDir) const
	{
		bool ret = false;

		SupportPoint &a = simplex[0], &b = simplex[1];
		glm::vec3	ba = a.getCSOPosition() - b.getCSOPosition(), b0 = -b.getCSOPosition();

		if (glm::dot(ba, b0) < -mEpsilon) {
			// The origin is outside the segment between b and a
			// Discard the a point and test the simplex in 0 dimensions
			simplex = { b };
			ret = doSimplex0D(simplex, searchDir);
		}
		else {
			// The origin is between b and a
			glm::vec3 n = glm::normalize(glm::cross(glm::cross(ba, b0), ba));
			if (glm::dot(b0, n) > mEpsilon) {
				// Preserve the current simplex and search towards the normal
				// vector to the ab segment
				searchDir = n;
			}
			else {
				// The origin is on the segment
				ret = true;
			}
		}

		return ret;
	}


	bool GJKCollisionDetector::doSimplex2D(Simplex& simplex, glm::vec3& searchDir) const
	{
		bool ret = false;

		SupportPoint &a = simplex[0], &b = simplex[1], &c = simplex[2];
		glm::vec3	ca = a.getCSOPosition() - c.getCSOPosition(), cb = b.getCSOPosition() - c.getCSOPosition(),
					c0 = -c.getCSOPosition(), n = glm::normalize(glm::cross(cb, ca)),
					nxca = glm::normalize(glm::cross(n, ca)), cbxn = glm::normalize(glm::cross(cb, n));

		if (glm::dot(nxca, c0) > mEpsilon) {
			// The origin is outside the triangle from the ca edge
			// Discard the b point and test the simplex in 1 dimension
			simplex = { a, c };
			ret = doSimplex1D(simplex, searchDir);
		}
		else if (glm::dot(cbxn, c0) > mEpsilon) {
			// The origin is outside the triangle from the cb edge
			// Discard the a point and test the simplex in 1 dimension
			simplex = { b, c };
			ret = doSimplex1D(simplex, searchDir);
		}
		else {
			// The origin is inside the triangle in 2D
			float dot = glm::dot(n, c0);
			if (dot > mEpsilon) {
				// The origin is above the triangle, preserve the current
				// simplex and search towards the normal vector
				searchDir = n;
			}
			else if (dot < -mEpsilon) {
				// The origin is below the triangle, reverse the current
				// simplex and search towards the -normal vector
				simplex = { b, a, c };
				searchDir = -n;
			}
			else {
				// The origin is on the triangle
				ret = true;
			}
		}

		return ret;
	}


	bool GJKCollisionDetector::doSimplex3D(Simplex& simplex, glm::vec3& searchDir) const
	{
		bool ret = false;

		SupportPoint &a = simplex[0], &b = simplex[1], &c = simplex[2], &d = simplex[3];
		glm::vec3	da = a.getCSOPosition() - d.getCSOPosition(), db = b.getCSOPosition() - d.getCSOPosition(),
					dc = c.getCSOPosition() - d.getCSOPosition(), d0 = -d.getCSOPosition(),
					dbxda = glm::normalize(glm::cross(db, da)), daxdc = glm::normalize(glm::cross(da, dc)),
					dcxdb = glm::normalize(glm::cross(dc, db));

		if (glm::dot(dbxda, d0) > mEpsilon) {
			// The origin is outside the tetrahedron from the bda face
			// Discard c and check the triangle in 2 dimensions
			simplex = { a, b, d };
			ret = doSimplex2D(simplex, searchDir);
		}
		else if (glm::dot(daxdc, d0) > mEpsilon) {
			// The origin is outside the tetrahedron from the adc face
			// Discard b and check the triangle in 2 dimensions
			simplex = { c, a, d };
			ret = doSimplex2D(simplex, searchDir);
		}
		else if (glm::dot(dcxdb, d0) > mEpsilon) {
			// The origin is outside the tetrahedron from the cdb face
			// Discard a and check the triangle in 2 dimensions
			simplex = { b, c, d };
			ret = doSimplex2D(simplex, searchDir);
		}
		else {
			// The origin is inside the tetrahedron, preserve the current
			// simplex
			ret = true;
		}

		return ret;
	}

}
