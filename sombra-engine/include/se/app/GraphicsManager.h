#ifndef GRAPHICS_MANAGER_H
#define GRAPHICS_MANAGER_H

#include <map>
#include "Skin.h"
#include "events/EventManager.h"
#include "../graphics/GraphicsEngine.h"
#include "../graphics/3D/Layer3D.h"
#include "../graphics/3D/Camera.h"
#include "../graphics/3D/Lights.h"
#include "../graphics/3D/Renderable3D.h"

namespace se::app {

	struct Entity;
	class ResizeEvent;


	/**
	 * Class GraphicsManager, it's a Manager used for storing, updating and
	 * rendering the Entities' graphics data
	 */
	class GraphicsManager : public IEventListener
	{
	private:	// Nested types
		using CameraUPtr = std::unique_ptr<graphics::Camera>;
		using LightUPtr = std::unique_ptr<graphics::ILight>;
		using Renderable3DUPtr = std::unique_ptr<graphics::Renderable3D>;
		using RenderableTerrainUPtr =
			std::unique_ptr<graphics::RenderableTerrain>;
		using SkinSPtr = std::shared_ptr<Skin>;

	private:	// Attributes
		/** The GraphicsEngine used for rendering the data of the Entities */
		graphics::GraphicsEngine& mGraphicsEngine;

		/** The EventManager that will notify the events */
		EventManager& mEventManager;

		/** The Layer3D used by the GraphicsEngine */
		graphics::Layer3D mLayer3D;

		/** The layers to update when the window is resized */
		std::vector<graphics::ILayer*> mLayers;

		std::map<Entity*, CameraUPtr> mCameraEntities;
		std::map<Entity*, LightUPtr> mLightEntities;
		std::multimap<Entity*, Renderable3DUPtr> mRenderable3DEntities;
		std::map<graphics::Renderable3D*, SkinSPtr> mRenderable3DSkins;
		std::map<Entity*, Renderable3DUPtr> mSkyEntities;
		std::map<Entity*, RenderableTerrainUPtr> mRenderableTerrainEntities;

	public:		// Functions
		/** Creates a new GraphicsManager
		 *
		 * @param	graphicsEngine a reference to the GraphicsEngine used by
		 * 			the GraphicsManager to render the entities
		 * @param	eventManager a reference to the EventManager that the
		 *			GraphicsManager will be subscribed to */
		GraphicsManager(
			graphics::GraphicsEngine& graphicsEngine,
			EventManager& eventManager
		);

		/** Class destructor */
		~GraphicsManager();

		/** Notifies the GraphicsManager of the given event
		 *
		 * @param	event the IEvent to notify */
		virtual void notify(const IEvent& event) override;

		/** Adds the given ILayer so it will be resized when the Window does so
		 *
		 * @param	layer a pointer to the layer to add */
		void addLayer(graphics::ILayer* layer);

		/** Removes the given ILayer so it won't longer be resized when the
		 * Window does so
		 *
		 * @param	layer a pointer to the layer to remove */
		void removeLayer(graphics::ILayer* layer);

		/** Adds the given Entity and its Camera data to the GraphicsManager
		 *
		 * @param	entity a pointer to the Entity to add to the GraphicsManager
		 * @param	camera a pointer to the camera to add to the GraphicsManager
		 * @note	The Camera initial data is overridden by the Entity one */
		void addCameraEntity(Entity* entity, CameraUPtr camera);

		/** Adds the given Entity and its Renderable3D and skin data to the
		 * GraphicsManager
		 *
		 * @param	entity a pointer to the Entity to add to the GraphicsManager
		 * @param	renderable3D a pointer to the Renderable3D to add to the
		 *			GraphicsManager
		 * @param	skin a pointer to the Skin needed for the skeletal animation
		 *			of the renderable3D mesh (optional)
		 * @note	The Renderable3D initial data is overridden by the Entity
		 *			one */
		void addRenderableEntity(
			Entity* entity,
			Renderable3DUPtr renderable3D, SkinSPtr skin = nullptr
		);

		/** Adds the given Entity and its Sky Renderable3D data to the
		 * GraphicsManager
		 *
		 * @param	entity a pointer to the Entity to add to the GraphicsManager
		 * @param	renderable3D a pointer to the Renderable3D to add to the
		 *			GraphicsManager as a Sky
		 * @note	The Renderable3D initial data is overridden by the Entity
		 *			one */
		void addSkyEntity(Entity* entity, Renderable3DUPtr renderable3D);

		/** Adds the given Entity and its RenderableTerrain data to the
		 * GraphicsManager
		 *
		 * @param	entity a pointer to the Entity to add to the GraphicsManager
		 * @param	renderable a pointer to the RenderableTerrain to add to the
		 *			GraphicsManager */
		void addTerrainEntity(Entity* entity, RenderableTerrainUPtr renderable);

		/** Adds the given Entity and its ILight data to the GraphicsManager
		 *
		 * @param	entity a pointer to the Entity to add to the GraphicsManager
		 * @param	light a pointer to the ILight to add to the GraphicsManager
		 * @note	The PointLight initial data is overridden by the Entity
		 *			one */
		void addLightEntity(Entity* entity, LightUPtr light);

		/** Removes the given Entity from the GraphicsManager so it won't
		 * longer be updated
		 *
		 * @param	entity a pointer to the Entity to remove from the
		 *			GraphicsManager */
		void removeEntity(Entity* entity);

		/** Updates the graphics data with the Entities */
		void update();

		/** Renders the graphics data of the Entities */
		void render();
	private:
		/** Handles the given ResizeEvent by notifying the GraphicsEngine of
		 * the window resize
		 *
		 * @param	event the ResizeEvent to handle */
		void onResizeEvent(const ResizeEvent& event);
	};

}

#endif		// GRAPHICS_MANAGER_H
