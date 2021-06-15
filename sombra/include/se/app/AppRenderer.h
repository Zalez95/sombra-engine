#ifndef APP_RENDERER_H
#define APP_RENDERER_H

#include <glm/glm.hpp>
#include "../graphics/GraphicsEngine.h"
#include "../graphics/core/Texture.h"
#include "../graphics/3D/RenderableMesh.h"
#include "events/ContainerEvent.h"
#include "events/ResizeEvent.h"
#include "graphics/DeferredLightRenderer.h"
#include "ECS.h"
#include "LightComponent.h"

namespace se::app {

	class Application;


	/**
	 * Class AppRenderer, It's a System used for creating the RenderGraph and
	 * rendering the Entities
	 * @note	the Passes can use either the "forwardRendererMesh", or the
	 *			"gBufferRendererTerrain", "gBufferRendererMesh" or
	 *			"gBufferRendererTerrain" of the RenderGraph for submitting the
	 *			Renderable3Ds. The gBufferRenderers are used for rendering
	 *			geometry in a deferred pipeline, the passes that uses it
	 *			must output position, normal, albedo, material and emissive
	 *			textures in that order, later this textures will be used for
	 *			rendering in a PBR pipeline. The "forwardRendererMesh" is
	 *			reserved for special cases that can't be rendered this way.
	 *			For the Renderable2Ds, there is a "renderer2D" for submitting
	 *			them */
	class AppRenderer : public ISystem
	{
	private:	// Nested types
		class StartShadowNode;
		class EndShadowNode;
		class CombineNode;

	private:	// Attributes
		/** The Application that holds the GraphicsEngine used for rendering
		 * the Entities */
		Application& mApplication;

		/** A pointer to the renderer used for deferred lighting */
		DeferredLightRenderer* mDeferredLightRenderer;

		/** A pointer to the resources node of the render graph */
		graphics::BindableRenderNode* mResources;

		/** The configuration used for rendering the shadows */
		ShadowData mShadowData;

		/** The bindable index of the irradiance Texture to render with */
		std::size_t mIrradianceTextureResource;

		/** The bindable index of the prefilter Texture to render with */
		std::size_t mPrefilterTextureResource;

		/** The light source Entity used for shadow mapping */
		Entity mShadowEntity;

		/** The light Probe Entity used for rendering */
		Entity mLightProbeEntity;

	public:		// Functions
		/** Creates a new AppRenderer
		 *
		 * @param	application a reference to the Application that holds the
		 *			current System
		 * @param	shadowData the configuration used for rendering the shadows
		 * @param	width the initial width of the FrameBuffer where the
		 *			Entities are going to be rendered
		 * @param	height the initial height of the FrameBuffer where the
		 *			Entities are going to be rendered */
		AppRenderer(
			Application& application, const ShadowData& shadowData,
			std::size_t width, std::size_t height
		);

		/** Class destructor */
		~AppRenderer();

		/** @copydoc ISystem::notify(const IEvent&) */
		virtual void notify(const IEvent& event) override;

		/** @copydoc ISystem::onNewComponent(Entity, const EntityDatabase::ComponentMask&) */
		virtual void onNewComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask
		) override;

		/** @copydoc ISystem::onRemoveComponent(Entity, const EntityDatabase::ComponentMask&) */
		virtual void onRemoveComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask
		) override;

		/** Updates the light sources with the Entities
		 *
		 * @note	this function must be called from the thread with the
		 *			Graphics API context (probably thread 0) */
		virtual void update() override;

		/** Renders the graphics data of the Entities
		 *
		 * @note	this function must be called from the thread with the
		 *			Graphics API context (probably thread 0) */
		void render();
	private:
		/** Function called when a LightComponent is added to an Entity
		 *
		 * @param	entity the Entity that holds the LightComponent
		 * @param	light a pointer to the new LightComponent */
		void onNewLight(Entity entity, LightComponent* light);

		/** Function called when a LightComponent is going to be removed from
		 * an Entity
		 *
		 * @param	entity the Entity that holds the LightComponent
		 * @param	light a pointer to the LightComponent that is going to be
		 *			removed */
		void onRemoveLight(Entity entity, LightComponent* light);

		/** Function called when a LightProbe is added to an Entity
		 *
		 * @param	entity the Entity that holds the LightProbe
		 * @param	lightProbe a pointer to the new LightProbe */
		void onNewLightProbe(Entity entity, LightProbe* lightProbe);

		/** Function called when a LightProbe is going to be removed from an
		 * Entity
		 *
		 * @param	entity the Entity that holds the LightProbe
		 * @param	lightProbe a pointer to the LightProbe that is going to be
		 *			removed */
		void onRemoveLightProbe(Entity entity, LightProbe* lightProbe);

		/** Adds shared resources to the RenderGraph resource node
		 *
		 * @param	width the initial width of the FrameBuffer where the
		 *			Entities are going to be rendered
		 * @param	height the initial height of the FrameBuffer where the
		 *			Entities are going to be rendered
		 * @return	true if the resources were added succesfully,
		 *			false otherwise */
		virtual bool addResources(std::size_t width, std::size_t height);

		/** Adds nodes to the RenderGraph and links them. It will add:
		 *  - "renderer2D" - renders Renderable2Ds
		 *  - the renderers of @see addShadowRenderers,
		 *    @see addDeferredRenderers and @see addForwardRenderers
		 *
		 * @param	width the initial width of the FrameBuffer where the
		 *			Entities are going to be rendered
		 * @param	height the initial height of the FrameBuffer where the
		 *			Entities are going to be rendered
		 * @return	true if the nodes were added and linked succesfully,
		 *			false otherwise */
		virtual bool addNodes(std::size_t width, std::size_t height);

		/** Creates the Renderer3Ds used for rendering shadows with the names
		 *	"shadowRendererMesh" - renders RenderableMeshes
		 *	"shadowRendererTerrain" - renders RenderableTerrains
		 * The nodes and connections that will be added to the graph looks like
		 * the following (the resource node already exists):
		 *
		 *  [    "resources"    ]                   |attach
		 *     |shadowBuffer  |shadowTexture   ["startShadow"]
		 *     |              |                     |attach
		 *     |input         |                     |
		 *  ["shadowFBClear"] |           __________|
		 *     |output        |          |
		 *     |              |          |
		 *     |target        |shadow    |attach
		 *  [    "shadowRendererTerrain"    ]
		 *     |target        |shadow
		 *     |              |
		 *     |target        |shadow
		 *  [    "shadowRendererMesh"    ]
		 *     |target        |shadow  |attach
		 *     |              |        |
		 *     |              |        |attach
		 *     |              |   ["endShadow"]
		 *     |              |        |attach
		 *
		 * @note	Any node that should be executed prior to the shadow
		 *			Renderer3Ds must be attached to the "startShadow" "input"
		 *			and any node that should be executed after them should be
		 *			attached to the "endShadow" "output"
		 *
		 * @param	renderGraph the RenderGraph where the nodes will be added
		 * @param	width the initial width of the FrameBuffer where the
		 *			Entities are going to be rendered
		 * @param	height the initial height of the FrameBuffer where the
		 *			Entities are going to be rendered
		 * @return	true if the nodes where added successfully, false
		 *			otherwise */
		bool addShadowRenderers(
			graphics::RenderGraph& renderGraph,
			std::size_t width, std::size_t height
		);

		/** Creates a deferred renderer with the following GBuffer Renderer3Ds:
		 *	"gBufferRendererTerrain" - renders RenderableTerrains
		 *	"gBufferRendererMesh" - renders RenderableMeshes
		 *	"gBufferRendererParticles" - renders ParticleSystems
		 * The nodes and connections that will be added to the graph looks like
		 * the following (the resource node already exists):
		 *
		 *  [                            "resources"                        ]
		 *   |gBuffer  |deferredBuffer             |positionTexture
		 *    \        |________________           |
		 *    |input                    |           |
		 *  ["gFBClear"]                |           |
		 *    |output                   |           |
		 *    |                         | ["texUnitNodePosition"]
		 *    |target                   |           |
		 *  ["gBufferRendererTerrain"]  |           |
		 *    |target                   |           |
		 *    |                         |           |
		 *    |target                   |           |
		 *  ["gBufferRendererMesh"]     |           |
		 *    |target                   |           |
		 *    |                         |           |
		 *    |target                   |           |
		 *  ["gBufferRendererParticles"]|           |
		 *    |target |attach___________|           |___________________
		 *    |    ___|    /                                           |
		 *    |   |attach |target |irradiance |prefilter |brdf |shadow |position
		 *    |  [                "deferredLightRenderer"                ]
		 *    |                             |target
		 *
		 * @note	where position appears, there will be also similar nodes
		 *			and connections for the normal, albedo, material
		 *			and emissive textures
		 *
		 * @param	renderGraph the RenderGraph where the nodes will be added
		 * @return	true if the nodes where added successfully, false
		 *			otherwise */
		bool addDeferredRenderers(graphics::RenderGraph& renderGraph);

		/** Creates the following forward renderers and adds them to the given
		 * RenderGraph:
		 *	"forwardRendererMesh" - renders RenderableMeshes
		 * The nodes and connections that will be added to the graph looks like
		 * the following (the resource node already exists):
		 *
		 *    |target  |irradiance  |prefilter  |brdf  |shadow  |color  |bright
		 *  [                    "forwardRendererMesh"                    ]
		 *                 |target      |color      |bright
		 *
		 * @param	renderGraph the RenderGraph where the nodes will be added
		 * @return	true if the nodes where added successfully, false
		 *			otherwise */
		bool addForwardRenderers(graphics::RenderGraph& renderGraph);

		/** Handles the given ContainerEvent by updating the Shadow Entity with
		 * which the shadow will be rendered
		 *
		 * @param	event the ContainerEvent to handle */
		void onShadowEvent(const ContainerEvent<Topic::Shadow, Entity>& event);

		/** Handles the given ResizeEvent by notifying the GraphicsEngine of
		 * the window resize
		 *
		 * @param	event the ResizeEvent to handle */
		void onResizeEvent(const ResizeEvent& event);
	};

}

#endif		// APP_RENDERER_H
