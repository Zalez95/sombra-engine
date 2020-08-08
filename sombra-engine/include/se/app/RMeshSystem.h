#ifndef RMESH_SYSTEM_H
#define RMESH_SYSTEM_H

#include <memory>
#include <unordered_map>
#include "../utils/MathUtils.h"
#include "../graphics/GraphicsEngine.h"
#include "../graphics/core/UniformVariable.h"
#include "ISystem.h"
#include "CameraSystem.h"

namespace se::app {

	/**
	 * Class RMeshSystem, it's a System used for updating the Entities'
	 * RenderableMesh data
	 */
	class RMeshSystem : public ISystem
	{
	private:	// Nested types
		/** The maximum number of joints in the program */
		static constexpr unsigned int kMaxJoints = 64;

		/** Struct RenderableMeshUniforms, holds the uniform variables to
		 * update of a RenderableMesh */
		struct RenderableMeshUniforms
		{
			std::shared_ptr<
				graphics::UniformVariableValue<glm::mat4>
			> modelMatrix;
			std::shared_ptr<
				graphics::UniformVariableValueVector<glm::mat4, kMaxJoints>
			> jointMatrices;
		};

	private:	// Attributes
		/** The GraphicsEngine used for rendering the RenderableMeshes */
		graphics::GraphicsEngine& mGraphicsEngine;

		/** The CameraSystem that holds the Passes data */
		CameraSystem& mCameraSystem;

		/** The RenderableMesh uniform variables mapped by the Entity and
		 * the RenderableMesh index in its MeshComponent */
		std::unordered_map<
			std::pair<Entity, std::size_t>,
			RenderableMeshUniforms,
			utils::PairHash
		> mRMeshUniforms;

	public:		// Functions
		/** Creates a new RMeshSystem
		 *
		 * @param	entityDatabase the EntityDatabase that holds all the
		 *			Entities
		 * @param	graphicsEngine the GraphicsEngine used for rendering the
		 *			RenderableMeshes
		 * @param	cameraSystem the CameraSystem that holds the Passes data */
		RMeshSystem(
			EntityDatabase& entityDatabase,
			graphics::GraphicsEngine& graphicsEngine,
			CameraSystem& cameraSystem
		);

		/** Class destructor */
		~RMeshSystem();

		/** Function that the EntityDatabase will call when an Entity is
		 * added
		 *
		 * @param	entity the new Entity */
		virtual void onNewEntity(Entity entity);

		/** Function that the EntityDatabase will call when an Entity is
		 * removed
		 *
		 * @param	entity the Entity to remove */
		virtual void onRemoveEntity(Entity entity);

		/** Updates the RenderableMeshes with the Entities */
		virtual void update() override;
	};

}

#endif		// RMESH_SYSTEM_H
