#ifndef GRAPHICS_COMPONENT_H
#define GRAPHICS_COMPONENT_H

#include <memory>
#include "../graphics/GraphicsSystem.h"
#include "../graphics/3D/Camera.h"
#include "../graphics/3D/Lights.h"
#include "../graphics/3D/Renderable3D.h"

namespace game {

	class Entity;


	/**
	 * Class std::unique_ptr<graphicsComponent, it's an asbtract Component used for storing
	 * and update the std::unique_ptr<graphics data of an Entity
	 */
	class GraphicsComponent
	{
	private:	// Attributes
		/** The System used for rendering the data of the GraphicsComponent */
		graphics::GraphicsSystem& mGraphicsSystem;

		std::unique_ptr<graphics::Camera> mCamera;
		std::unique_ptr<graphics::PointLight> mPointLight;
		std::unique_ptr<graphics::Renderable3D> mRenderable3D;

	public:		// Functions
		/** Creates a new GraphicsComponent */
		GraphicsComponent(
			graphics::GraphicsSystem& graphicsSystem,
			std::unique_ptr<graphics::Camera> camera,
			std::unique_ptr<graphics::PointLight> pointLight,
			std::unique_ptr<graphics::Renderable3D> renderable3D
		);

		/** Class destructor */
		~GraphicsComponent();

		/** Updates the given common data of the given entity
		 * 
		 * @param	entity the Entity to update
		 * @param	delta the elapsed time since the last update */
		void update(Entity& entity, float delta);
	};

}

#endif		// GRAPHICS_COMPONENT_H
