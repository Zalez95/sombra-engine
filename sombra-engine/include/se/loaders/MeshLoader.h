#ifndef MESH_LOADER_H
#define MESH_LOADER_H

#include <memory>

namespace se::graphics { class Mesh; }
namespace se::collision { class HalfEdgeMesh; }

namespace se::loaders {

	struct RawMesh;


	/**
	 * Class MeshLoader, it's used to create meshes from raw meshes data
	 */
	class MeshLoader
	{
	private:	// Nested types
		using GraphicsMeshUPtr = std::unique_ptr<graphics::Mesh>;
		using HalfEdgeMeshUPtr = std::unique_ptr<collision::HalfEdgeMesh>;

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
		/** creates a Graphics Mesh with the given mesh data
		 *
		 * @param	rawMesh the data with which we will create the graphics
		 *			mesh
		 * @return	a pointer to the new Graphics Mesh */
		static GraphicsMeshUPtr createGraphicsMesh(const RawMesh& rawMesh);

		/** creates a HalfEdgeMesh with the given mesh data
		 *
		 * @param	rawMesh the data with which we will create the HalfEdgeMesh
		 * @return	a pointer to the new HalfEdgeMesh */
		static HalfEdgeMeshUPtr createHalfEdgeMesh(const RawMesh& rawMesh);
	};

}

#endif		// MESH_LOADER_H
