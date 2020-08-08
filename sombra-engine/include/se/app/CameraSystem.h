#ifndef CAMERA_SYSTEM_H
#define CAMERA_SYSTEM_H

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "../graphics/Pass.h"
#include "../graphics/core/Program.h"
#include "../graphics/core/UniformVariable.h"
#include "ISystem.h"

namespace se::app {

	class Camera;
	class ResizeEvent;


	/**
	 * Class CameraSystem, it's the System used for updating the Entities
	 * Cameras
	 */
	class CameraSystem : public ISystem
	{
	private:	// Nested types
		friend class RMeshSystem;
		friend class RTerrainSystem;
		friend class LightSourceSystem;

		using PassSPtr = std::shared_ptr<graphics::Pass>;
		using ProgramSPtr = std::shared_ptr<graphics::Program>;
		using Mat4Uniform = std::shared_ptr<
			graphics::UniformVariableValue<glm::mat4>
		>;

		struct PassData
		{
			bool is2D;
			PassSPtr pass;
			ProgramSPtr program;
			Mat4Uniform viewMatrix;
			Mat4Uniform projectionMatrix;

			PassData(bool is2D) : is2D(is2D) {};
		};

	private:	// Attributes
		/** The passes added to the CameraSystem */
		std::vector<PassData> mPassesData;

		/** The width of the window */
		std::size_t mWidth;

		/** The height of the window */
		std::size_t mHeight;

		/** A pointer to the current active camera with which the scene will
		 * be rendered */
		Camera* mActiveCamera;

		/** If the active camera has been updated or not */
		bool mActiveCameraUpdated;

	public:		// Functions
		/** Creates a new CameraSystem
		 *
		 * @param	entityDatabase the EntityDatabase that holds all the
		 *			Entities
		 * @param	width the initial width of the window
		 * @param	height the initial height of the window */
		CameraSystem(
			EntityDatabase& entityDatabase,
			std::size_t width, std::size_t height
		);

		/** Class destructor */
		~CameraSystem();

		/** @return	a pointer to the active Camera */
		Camera* getActiveCamera() { return mActiveCamera; };

		/** @return	true if the active Camera position was updated in the
		 *			last frame */
		bool wasCameraUpdated() { return mActiveCameraUpdated; };

		/** Creates a new Pass and adds the uniform variables for the cameras
		 *
		 * @param	renderer a pointer to the Renderer of the new Pass
		 * @param	program a pointer to the program of the new Pass
		 * @return	the new Pass */
		PassSPtr createPass2D(
			graphics::Renderer* renderer, ProgramSPtr program
		);

		/** Creates a new Pass and adds the uniform variables for the cameras
		 * and lightning
		 *
		 * @param	renderer a pointer to the Renderer of the new Pass
		 * @param	program a pointer to the Program of the new Pass
		 * @return	the new Pass */
		PassSPtr createPass3D(
			graphics::Renderer* renderer, ProgramSPtr program
		);

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
		/** Handles the given ResizeEvent by notifying the GraphicsEngine of
		 * the window resize
		 *
		 * @param	event the ResizeEvent to handle */
		void onResizeEvent(const ResizeEvent& event);
	};

}

#endif		// CAMERA_SYSTEM_H
