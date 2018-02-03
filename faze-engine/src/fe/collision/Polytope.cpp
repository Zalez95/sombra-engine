#include "fe/collision/Polytope.h"
#include <cassert>
#include <glm/gtc/matrix_transform.hpp>
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
			for (unsigned int i = 0; i < 3; ++i) {
				for (float v : {-1.0f, 1.0f}) {
					glm::vec3 searchDir;
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
			unsigned int closestAxis = 0;
			for (unsigned int i = 1; i < 3; ++i) {
				if (v01[i] < v01[closestAxis]) {
					closestAxis = i;
				}
			}

			glm::vec3 axis(0.0f);
			axis[closestAxis] = 1.0f;

			// Calculate a normal vector to the line with the axis
			glm::vec3 vNormal = glm::cross(v01, axis);

			// Search a support point in the 6 directions around the line
			glm::vec3 searchDir = vNormal;
			for (size_t i = 0; i < 6; ++i) {
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

		// Create the polytope from the simplex's points
		SupportPoint *d = &simplex[0], *c = &simplex[1], *b = &simplex[2], *a = &simplex[3];
		mVertices = { *d, *c, *b, *a };
		mFaces = { Triangle(a,b,c), Triangle(a,d,b), Triangle(a,c,d), Triangle(b,d,c) };
	}

}}
