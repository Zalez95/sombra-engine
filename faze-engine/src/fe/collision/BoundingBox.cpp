#include "fe/collision/BoundingBox.h"

namespace fe { namespace collision {

	HalfEdgeMesh BoundingBox::meshFromLengths(const glm::vec3& lengths) const
	{
		HalfEdgeMesh ret;

		std::vector<int> iVertices;
		iVertices.reserve(8);

		iVertices.push_back( ret.addVertex(glm::vec3(-lengths.x, -lengths.y, -lengths.z) / 2.0f) );
		iVertices.push_back( ret.addVertex(glm::vec3(-lengths.x, -lengths.y,  lengths.z) / 2.0f) );
		iVertices.push_back( ret.addVertex(glm::vec3(-lengths.x,  lengths.y, -lengths.z) / 2.0f) );
		iVertices.push_back( ret.addVertex(glm::vec3(-lengths.x,  lengths.y,  lengths.z) / 2.0f) );
		iVertices.push_back( ret.addVertex(glm::vec3( lengths.x, -lengths.y, -lengths.z) / 2.0f) );
		iVertices.push_back( ret.addVertex(glm::vec3( lengths.x, -lengths.y,  lengths.z) / 2.0f) );
		iVertices.push_back( ret.addVertex(glm::vec3( lengths.x,  lengths.y, -lengths.z) / 2.0f) );
		iVertices.push_back( ret.addVertex(glm::vec3( lengths.x,  lengths.y,  lengths.z) / 2.0f) );

		ret.addFace({ iVertices[0], iVertices[1], iVertices[3], iVertices[2] });
		ret.addFace({ iVertices[0], iVertices[2], iVertices[6], iVertices[4] });
		ret.addFace({ iVertices[4], iVertices[6], iVertices[7], iVertices[5] });
		ret.addFace({ iVertices[5], iVertices[7], iVertices[3], iVertices[1] });
		ret.addFace({ iVertices[2], iVertices[3], iVertices[7], iVertices[6] });
		ret.addFace({ iVertices[0], iVertices[4], iVertices[5], iVertices[1] });

		return ret;
	}

}}
