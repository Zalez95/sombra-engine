#ifndef MESH_COMPONENT_H
#define MESH_COMPONENT_H

#include "../utils/FixedVector.h"
#include "../graphics/3D/RenderableMesh.h"

namespace se::app {

	/**
	 * Struct MeshComponent, it's a Component that holds all the graphics
	 * RenderableMeshes of an Entity.
	 */
	struct MeshComponent
	{
		/** The maximum number of RenderableMeshes that the MeshComponent can
		 * hold */
		static constexpr std::size_t kMaxMeshes = 128;

		/** All the RenderableMeshes added to the MeshComponent */
		utils::FixedVector<graphics::RenderableMesh, kMaxMeshes> rMeshes;
	};

}

#endif		// MESH_COMPONENT_H
