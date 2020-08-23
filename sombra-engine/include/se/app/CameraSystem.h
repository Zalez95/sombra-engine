#ifndef CAMERA_SYSTEM_H
#define CAMERA_SYSTEM_H

#include <memory>
#include <unordered_map>
#include "../utils/PackedVector.h"
#include "../graphics/Pass.h"
#include "../graphics/core/UniformVariable.h"
#include "../graphics/3D/Renderable3D.h"
#include "ISystem.h"
#include "events/ContainerEvent.h"

namespace se::app {

	class Application;


	/**
	 * Class CameraSystem, it's the System used for updating the Entities
	 * Cameras, and the view and projection matrices on their shaders
	 */
	class CameraSystem : public ISystem
	{
	private:	// Nested types
		using PassSPtr = std::shared_ptr<graphics::Pass>;
		using Mat4Uniform = std::shared_ptr<
			graphics::UniformVariableValue<glm::mat4>
		>;

		/** Struct PassData, holds the shared uniform variables between the
		 * Renderables */
		struct PassData
		{
			std::size_t userCount = 0;
			PassSPtr pass;
			Mat4Uniform viewMatrix;
			Mat4Uniform projectionMatrix;
		};

	private:	// Attributes
		/** The Application that holds the Scene with the Repository with the
		 * Passes and Programs */
		Application& mApplication;

		/** The Entity that holds the current active camera with which the
		 * scene will be rendered */
		Entity mCameraEntity;

		/** If the active camera has been updated or not */
		bool mCameraUpdated;

		/** The shared uniform variables of the Passes */
		utils::PackedVector<PassData> mPassesData;

		/** Maps each Entity with its respective PassData indices */
		std::unordered_map<Entity, std::vector<std::size_t>> mEntityPasses;

	public:		// Functions
		/** Creates a new CameraSystem
		 *
		 * @param	application a reference to the Application that holds the
		 *			current System */
		CameraSystem(Application& application);

		/** Class destructor */
		~CameraSystem();

		/** Notifies the CameraSystem of the given event
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

		/** Updates the Cameras sources with the Entities */
		virtual void update() override;
	private:
		/** Handles the given ContainerEvent by updating the Camera Entity with
		 * which the Scene will be rendered
		 *
		 * @param	event the ContainerEvent to handle */
		void onCameraEvent(const ContainerEvent<Topic::Camera, Entity>& event);

		/** Processes the passes of the given renderable adding them their
		 * respective shared uniforms
		 *
		 * @param	renderable the Renderable3D to process
		 * @param	output the vector where the indices of the passes in
		 *			@see mPassesData will be appended */
		void processPasses(
			graphics::Renderable3D& renderable, std::vector<std::size_t>& output
		);
	};

}

#endif		// CAMERA_SYSTEM_H
