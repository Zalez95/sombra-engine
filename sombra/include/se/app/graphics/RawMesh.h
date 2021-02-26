#ifndef RAW_MESH_H
#define RAW_MESH_H

#include <vector>
#include <string>
#include <glm/glm.hpp>

namespace se::app {

	/** The attribute indices of the VAO of a Mesh */
	struct MeshAttributes
	{
		static constexpr unsigned int PositionAttribute		= 0;
		static constexpr unsigned int NormalAttribute		= 1;
		static constexpr unsigned int TangentAttribute		= 2;
		static constexpr unsigned int TexCoordAttribute0	= 3;
		static constexpr unsigned int TexCoordAttribute1	= 4;
		static constexpr unsigned int ColorAttribute		= 5;
		static constexpr unsigned int JointIndexAttribute	= 6;
		static constexpr unsigned int JointWeightAttribute	= 7;
		static constexpr unsigned int NumAttributes			= 8;
	};


	/**
	 * Struct RawMesh. It holds all the mesh readed data by the loader classes.
	 */
	struct RawMesh
	{
	public:
		std::string name;

		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec3> tangents;
		std::vector<glm::vec2> texCoords;
		std::vector<glm::u16vec4> jointIndices;
		std::vector<glm::vec4> jointWeights;

		std::vector<unsigned short> indices;

		RawMesh(const std::string& name = "") : name(name) {};
	};

}

#endif		// RAW_MESH_H
