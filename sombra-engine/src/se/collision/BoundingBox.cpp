#include <array>
#include "se/collision/BoundingBox.h"

namespace se::collision {

	HalfEdgeMesh BoundingBox::meshFromLengths(const glm::vec3& lengths) const
	{
		HalfEdgeMesh ret;

		std::array<int, 8> iVertices = {
			addVertex(ret, glm::vec3(-lengths.x, -lengths.y, -lengths.z) / 2.0f),
			addVertex(ret, glm::vec3(-lengths.x, -lengths.y,  lengths.z) / 2.0f),
			addVertex(ret, glm::vec3(-lengths.x,  lengths.y, -lengths.z) / 2.0f),
			addVertex(ret, glm::vec3(-lengths.x,  lengths.y,  lengths.z) / 2.0f),
			addVertex(ret, glm::vec3( lengths.x, -lengths.y, -lengths.z) / 2.0f),
			addVertex(ret, glm::vec3( lengths.x, -lengths.y,  lengths.z) / 2.0f),
			addVertex(ret, glm::vec3( lengths.x,  lengths.y, -lengths.z) / 2.0f),
			addVertex(ret, glm::vec3( lengths.x,  lengths.y,  lengths.z) / 2.0f)
		};

		addFace(ret, { iVertices[0], iVertices[1], iVertices[3], iVertices[2] });
		addFace(ret, { iVertices[0], iVertices[2], iVertices[6], iVertices[4] });
		addFace(ret, { iVertices[4], iVertices[6], iVertices[7], iVertices[5] });
		addFace(ret, { iVertices[5], iVertices[7], iVertices[3], iVertices[1] });
		addFace(ret, { iVertices[2], iVertices[3], iVertices[7], iVertices[6] });
		addFace(ret, { iVertices[0], iVertices[4], iVertices[5], iVertices[1] });

		return ret;
	}

}
