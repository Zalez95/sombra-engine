#ifndef RTERRAIN_SYSTEM_H
#define RTERRAIN_SYSTEM_H

#include <memory>
#include <unordered_map>
#include "../graphics/Pass.h"
#include "../graphics/core/UniformVariable.h"
#include "ISystem.h"
#include "events/ContainerEvent.h"

namespace se::app {

	class Application;


	/**
	 * Class RTerrainSystem, it's a System used for updating the Entities'
	 * RenderableTerrain data
	 */
	class RTerrainSystem : public ISystem
	{
	private:	// Nested types
		/** Struct RenderableTerrainUniforms, holds the uniform variables to
		 * update of a RenderableTerrain */
		struct RenderableTerrainUniforms
		{
			std::shared_ptr<graphics::Pass> pass;
			std::shared_ptr<graphics::UniformVariableValue<glm::mat4>>
				modelMatrix;
		};

	private:	// Attributes
		/** The Application that holds the GraphicsEngine used for rendering
		 * the RenderableTerrains */
		Application& mApplication;

		/** The RenderableTerrain uniform variables mapped by the Entity */
		std::unordered_map<Entity, std::vector<RenderableTerrainUniforms>>
			mEntityUniforms;

		/** The camera Entity used for rendering */
		Entity mCameraEntity;

		/** If the camera was updated or not */
		bool mCameraUpdated;

	public:		// Functions
		/** Creates a new RTerrainSystem
		 *
		 * @param	application a reference to the Application that holds the
		 *			current System */
		RTerrainSystem(Application& application);

		/** Class destructor */
		~RTerrainSystem();

		/** Notifies the RTerrainSystem of the given event
		 *
		 * @param	event the IEvent to notify */
		virtual void notify(const IEvent& event) override;

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
	private:
		/** Handles the given ContainerEvent by updating the Camera Entity with
		 * which the Scene will be rendered
		 *
		 * @param	event the ContainerEvent to handle */
		void onCameraEvent(const ContainerEvent<Topic::Camera, Entity>& event);
	};

}

#endif		// RTERRAIN_SYSTEM_H
