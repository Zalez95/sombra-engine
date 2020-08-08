#ifndef RTERRAIN_SYSTEM_H
#define RTERRAIN_SYSTEM_H

#include <memory>
#include <unordered_map>
#include "../graphics/GraphicsEngine.h"
#include "../graphics/3D/RenderableTerrain.h"
#include "../graphics/core/UniformVariable.h"
#include "ISystem.h"
#include "CameraSystem.h"

namespace se::app {

	/**
	 * Class RTerrainSystem, it's a System used for updating the Entities'
	 * RenderableTerrain data
	 */
	class RTerrainSystem : public ISystem
	{
	private:	// Nested types
		struct RenderableTerrainData
		{
			std::shared_ptr<
				graphics::UniformVariableValue<glm::mat4>
			> modelMatrix;
		};

	private:	// Attributes
		/** The GraphicsEngine used for rendering the RenderableTerrains */
		graphics::GraphicsEngine& mGraphicsEngine;

		/** The CameraSystem that holds the Passes data */
		CameraSystem& mCameraSystem;

		/** Maps each Entity with its respective RenderableTerrain uniform
		 * data */
		std::unordered_map<
			Entity, RenderableTerrainData
		> mRenderableTerrainEntities;

	public:		// Functions
		/** Creates a new RTerrainSystem
		 *
		 * @param	entityDatabase the EntityDatabase that holds all the
		 *			Entities
		 * @param	graphicsEngine the GraphicsEngine used for rendering the
		 *			RenderableTerrains
		 * @param	cameraSystem the CameraSystem that holds the Passes data */
		RTerrainSystem(
			EntityDatabase& entityDatabase,
			graphics::GraphicsEngine& graphicsEngine,
			CameraSystem& cameraSystem
		);

		/** Class destructor */
		~RTerrainSystem();

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

		/** Updates the RenderableTerrains with the Entities */
		virtual void update() override;
	};

}

#endif		// RTERRAIN_SYSTEM_H
