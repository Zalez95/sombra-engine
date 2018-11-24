#ifndef RAW_MESH_H
#define RAW_MESH_H

#include <vector>
#include <string>
#include <glm/glm.hpp>

namespace se::loaders {

	/**
	 * Struct RawMesh. It holds all the mesh readed data by the loader classes.
	 */
	struct RawMesh
	{
	public:
		std::string name;

		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> uvs;
		std::vector<float> jointWeights;

		std::vector<unsigned short> faceIndices;
		std::vector<unsigned short> jointIndices;

		RawMesh(const std::string& name) : name(name) {};
	};

}

#endif		// RAW_MESH_H
