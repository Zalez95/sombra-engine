#include <cassert>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include "fe/collision/Polytope.h"
#include "fe/collision/ConvexCollider.h"

namespace fe { namespace collision {

// Static variables definition
	const float Polytope::sKEpsilon = 0.0001f;

// Public functions
	Polytope::Polytope(
		const ConvexCollider& collider1, const ConvexCollider& collider2,
		std::vector<SupportPoint>& simplex
	) {
		assert(simplex.size() > 0 && "The simplex must have at least one support point.");

		if (simplex.size() == 1) {
			// Search a support point in each axis direction
			for (int i = 0; i < 3; ++i) {
				for (float v : {-1.0f, 1.0f}) {
					glm::vec3 searchDir(0.0f);
					searchDir[i] = v;
					SupportPoint sp(collider1, collider2, searchDir);

					if (glm::length(sp.getCSOPosition() - simplex[0].getCSOPosition()) >= sKEpsilon) {
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

				if (glm::length(sp.getCSOPosition()) > sKEpsilon) {
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

			if (glm::length(sp.getCSOPosition()) < sKEpsilon) {
				// Try the opposite direction
				searchDir = -searchDir;
				sp = SupportPoint(collider1, collider2, searchDir);
			}

			simplex.push_back(sp);
		}

		// Fix the tetrahedron winding (It's faces must be counter-clockwise)
		SupportPoint *d = &simplex[0], *c = &simplex[1], *b = &simplex[2], *a = &simplex[3];
		glm::vec3 da = a->getCSOPosition() - d->getCSOPosition(),
			db = b->getCSOPosition() - d->getCSOPosition(),
			dc = c->getCSOPosition() - d->getCSOPosition(); 
		if (glm::dot(da, glm::cross(db, dc)) > 0.0f) {
			std::swap(b, c);
		}

		// Create the polytope from the simplex's points
		vertices = { *d, *c, *b, *a };
		faces = { Triangle(a,b,c), Triangle(a,d,b), Triangle(a,c,d), Triangle(b,d,c) };
	}

}}
