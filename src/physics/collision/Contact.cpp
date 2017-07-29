#include "Contact.h"

namespace physics {

	glm::mat3 Contact::getContactToWorldMatrix() const
	{
		// Two of the tangent vectors of the Contact that
		// with the normal vector create an orthonormal basis
		glm::vec3 tangents[2];

		// We create the orthonormal with the normal and one generated vector
		// (we split the code so we don't end with problems with the normal being
		// parallel to the generated vector)
		if (abs(mNormal.x) > abs(mNormal.y)) {
			// The normal is nearer to the X axis so we use the Y axis as the generated vector
			tangents[0] = glm::cross(mNormal, glm::vec3(0, 1, 0));
			tangents[1] = glm::cross(tangents[0], mNormal);
		}
		else {
			// The normal is nearer to the Y axis so we use the X axis as the generated vector
			tangents[0] = glm::cross(mNormal, glm::vec3(1, 0, 0));
			tangents[1] = glm::cross(tangents[0], mNormal);
		}

		tangents[0] = glm::normalize(tangents[0]);
		tangents[1] = glm::normalize(tangents[1]);

		// Create the matrix from the basis vectors and return it's inverse
		// Note that the inverse of a rotation matrix is the same than it's transpose
		return glm::transpose( glm::mat3(mNormal, tangents[0], tangents[1]) );
	}

}
