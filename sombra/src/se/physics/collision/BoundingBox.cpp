#include <array>
#include "se/physics/collision/BoundingBox.h"

namespace se::physics {

	BoundingBox::BoundingBox(const glm::vec3& lengths) : mLengths(0.0f)
	{
		setLengths(lengths);
	}


	void BoundingBox::setLengths(const glm::vec3& lengths)
	{
		assert(glm::all(glm::greaterThanEqual(lengths, glm::vec3(0.0f))) && "The lengths can't be smaller than zero.");

		mLengths = lengths;
		setLocalMesh(meshFromLengths(mLengths));
	}


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
