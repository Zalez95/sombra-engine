#ifndef RAW_MESH_H
#define RAW_MESH_H

#include <vector>
#include <string>
#include <glm/glm.hpp>

namespace fe { namespace loaders {

	/**
	 * Struct RawMesh. It holds all the mesh readed data by the loader classes.
	 */
	struct RawMesh
	{
	public:
		std::string mName;

		std::vector<glm::vec3> mPositions;
		std::vector<glm::vec3> mNormals;
		std::vector<glm::vec2> mUVs;
		std::vector<float> mJointWeights;

		std::vector<unsigned short> mFaceIndices;
		std::vector<unsigned short> mJointIndices;

		RawMesh(const std::string& name) : mName(name) {};

		~RawMesh() {};
	};

}}

#endif		// RAW_MESH_H
