#ifndef MESH_LOADER_H
#define MESH_LOADER_H

#include "se/graphics/3D/Mesh.h"
#include "se/collision/HalfEdgeMesh.h"

namespace se::loaders {

	struct RawMesh;


	/**
	 * Class MeshLoader, it's used to create meshes from raw meshes data
	 */
	class MeshLoader
	{
	public:		// Functions
		/** creates a Graphics Mesh with the given mesh data
		 *
		 * @param	rawMesh the data with which we will create the graphics
		 *			mesh
		 * @return	the new Graphics Mesh */
		static graphics::Mesh createGraphicsMesh(const RawMesh& rawMesh);

		/** creates a HalfEdgeMesh with the given mesh data
		 *
		 * @param	rawMesh the data with which we will create the HalfEdgeMesh
		 * @return	a pair with the loaded HafEdgeMesh and a bool that indicates
		 *			if the HalfEdgeMesh was loaded correctly or not */
		static std::pair<collision::HalfEdgeMesh, bool> createHalfEdgeMesh(
			const RawMesh& rawMesh
		);
	};

}

#endif		// MESH_LOADER_H
