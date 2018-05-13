#include <cassert>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include "fe/collision/SupportPoint.h"
#include "fe/collision/ConvexCollider.h"
#include "Polytope.h"

namespace fe { namespace collision {

	Polytope::Polytope(
		const ConvexCollider& collider1, const ConvexCollider& collider2,
		std::vector<SupportPoint>& simplex
	) {
		assert(simplex.size() > 0 && "The simplex must have at least one support point.");

		const float kEpsilon = 0.0001f;
		if (simplex.size() == 1) {
			// Search a support point in each axis direction
			for (int i = 0; i < 3; ++i) {
				for (float v : {-1.0f, 1.0f}) {
					glm::vec3 searchDir(0.0f);
					searchDir[i] = v;
					SupportPoint sp(collider1, collider2, searchDir);

					if (glm::length(sp.getCSOPosition() - simplex[0].getCSOPosition()) >= kEpsilon) {
						simplex.push_back(sp);
						break;
					}
				}
			}
		}
		if (simplex.size() == 2) {
			glm::vec3 v01 = simplex[1].getCSOPosition() - simplex[0].getCSOPosition();

			// Calculate a rotation matrix of 60 degrees around the line vector
			glm::mat3 rotate60 = glm::mat3(glm::rotate(glm::mat4(1.0f), glm::pi<float>() / 6.0f, v01));

			// Find the least significant axis
			glm::vec3 vAxis(0.0f);
			int iAxis = std::distance(&v01.x, std::max_element(&v01.x, &v01.x + 3));
			vAxis[iAxis] = 1.0f;

			// Calculate a normal vector to the line with the axis
			glm::vec3 vNormal = glm::cross(v01, vAxis);

			// Search a support point in the 6 directions around the line
			glm::vec3 searchDir = vNormal;
			for (int i = 0; i < 6; ++i) {
				SupportPoint sp(collider1, collider2, searchDir);

				if (glm::length(sp.getCSOPosition()) > kEpsilon) {
					simplex.push_back(sp);
					break;
				}

				searchDir = rotate60 * searchDir;
			}
		}
		if (simplex.size() == 3) {
			// Search a support point along the simplex's triangle normal
			glm::vec3 v01 = simplex[1].getCSOPosition() - simplex[0].getCSOPosition();
			glm::vec3 v02 = simplex[2].getCSOPosition() - simplex[0].getCSOPosition();
			glm::vec3 searchDir = glm::cross(v01, v02);

			SupportPoint sp(collider1, collider2, searchDir);

			if (glm::length(sp.getCSOPosition()) < kEpsilon) {
				// Try the opposite direction
				searchDir = -searchDir;
				sp = SupportPoint(collider1, collider2, searchDir);
			}

			simplex.push_back(sp);
		}

		// Create the polytope from the simplex's points
		vertices.push_back(simplex[0]);		SupportPoint *d = &vertices.back();
		vertices.push_back(simplex[1]);		SupportPoint *c = &vertices.back();
		vertices.push_back(simplex[2]);		SupportPoint *b = &vertices.back();
		vertices.push_back(simplex[3]);		SupportPoint *a = &vertices.back();

		glm::vec3 da = a->getCSOPosition() - d->getCSOPosition(),
			db = b->getCSOPosition() - d->getCSOPosition(),
			dc = c->getCSOPosition() - d->getCSOPosition(); 
		if (glm::dot(da, glm::cross(db, dc)) > 0.0f) {
			// Fix the tetrahedron winding (the faces must be in
			// counter-clockwise order)
			std::swap(b, c);
		}

		faces = { Triangle(a,b,c), Triangle(a,d,b), Triangle(a,c,d), Triangle(b,d,c) };
	}

}}
