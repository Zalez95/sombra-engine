#ifndef MESH_LOADER_H
#define MESH_LOADER_H

#include <memory>

namespace fe { namespace graphics { class Mesh; }}

namespace fe { namespace loaders {

	struct RawMesh;


	/**
	 * Class MeshLoader, it's used to create meshes from raw data or from the
	 * given files
	 */
	class MeshLoader
	{
	private:	// Nested types
		typedef std::unique_ptr<graphics::Mesh> MeshUPtr;

		/** The attribute indices of the Meshes */
		enum Attributes : unsigned int
		{
			POSITION_ATTRIBUTE = 0,
			NORMAL_ATTRIBUTE,
			UV_ATTRIBUTE,
			JOINT_WEIGHT_ATTRIBUTE,
			JOINT_INDEX_ATTRIBUTE
		};

	public:		// Functions
		/** Creates a new MeshLoader */
		MeshLoader() {};

		/** Class destructor */
		~MeshLoader() {};

		/** creates a Mesh with the given mesh data
		 *
		 * @param	rawMesh the data with which we will create the graphics
		 *			mesh
		 * @return	a pointer to the new created Mesh */
		MeshUPtr createMesh(const RawMesh& rawMesh) const;
	};

}}

#endif		// MESH_LOADER_H
