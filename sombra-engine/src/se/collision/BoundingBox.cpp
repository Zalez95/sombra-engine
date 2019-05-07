#include <array>
#include "se/collision/BoundingBox.h"

namespace se::collision {

	HalfEdgeMesh BoundingBox::meshFromLengths(const glm::vec3& lengths) const
	{
		HalfEdgeMesh ret;

		std::array<int, 8> vertexIndices = {
			addVertex(ret, glm::vec3(-lengths.x, -lengths.y, -lengths.z) / 2.0f),
			addVertex(ret, glm::vec3(-lengths.x, -lengths.y,  lengths.z) / 2.0f),
			addVertex(ret, glm::vec3(-lengths.x,  lengths.y, -lengths.z) / 2.0f),
			addVertex(ret, glm::vec3(-lengths.x,  lengths.y,  lengths.z) / 2.0f),
			addVertex(ret, glm::vec3( lengths.x, -lengths.y, -lengths.z) / 2.0f),
			addVertex(ret, glm::vec3( lengths.x, -lengths.y,  lengths.z) / 2.0f),
			addVertex(ret, glm::vec3( lengths.x,  lengths.y, -lengths.z) / 2.0f),
			addVertex(ret, glm::vec3( lengths.x,  lengths.y,  lengths.z) / 2.0f)
		};

		std::array<std::array<int, 4>, 6> faceIndices = {{
			{{ vertexIndices[0], vertexIndices[1], vertexIndices[3], vertexIndices[2] }},
			{{ vertexIndices[0], vertexIndices[2], vertexIndices[6], vertexIndices[4] }},
			{{ vertexIndices[4], vertexIndices[6], vertexIndices[7], vertexIndices[5] }},
			{{ vertexIndices[5], vertexIndices[7], vertexIndices[3], vertexIndices[1] }},
			{{ vertexIndices[2], vertexIndices[3], vertexIndices[7], vertexIndices[6] }},
			{{ vertexIndices[0], vertexIndices[4], vertexIndices[5], vertexIndices[1] }}
		}};

		for (const auto& face : faceIndices) {
			addFace(ret, face.begin(), face.end());
		}

		return ret;
	}

}
