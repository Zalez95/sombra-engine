#ifndef APP_RENDER_GRAPH_H
#define APP_RENDER_GRAPH_H

#include "../../graphics/RenderGraph.h"
#include "se/app/Repository.h"

namespace se::app {

	/**
	 * Class AppRenderGraph. It's the RenderGraph used by the Application for
	 * rendering all the Entities and Renderables.
	 * It creates a "forwardRendererMesh", "gBufferRendererTerrain",
	 * "gBufferRendererMesh" and "gBufferRendererParticles" that can be used
	 * for rendering the Entities Renderable3Ds. The gBufferRenderers are used
	 * for rendering the meshes in a deferred pipeline, the shaders that uses
	 * them must output position, normal, albedo, material and emissive
	 * textures in that order, later those textures will be used for rendering
	 * in a PBR pipeline. The "forwardRendererMesh" is reserved for special
	 * cases that can't be rendered this way (mostrly transparency), and they
	 * can't cast nor receive shadows.
	 * For rendering shadows there are available a "shadowRendererMesh" and a
	 * "shadowRendererTerrain" renderers, particle can't cast shadows.
	 * For the Renderable2Ds, there is a "renderer2D" for submitting them
	 */
	class AppRenderGraph : public graphics::RenderGraph
	{
	private:	// Nested types
		class CombineNode;

	public:		// Functions
		/** Creates a new AppRenderGraph
		 *
		 * @param	repository the Repository that holds the Resources
		 * @param	width the initial width of the FrameBuffers where the
		 *			Renderables will be drawn
		 * @param	height the initial height of the FrameBuffers where the
		 *			Renderables will be drawn */
		AppRenderGraph(
			Repository& repository, std::size_t width, std::size_t height
		);

		/** Class destructor */
		virtual ~AppRenderGraph() = default;

		/** Sets the resolution of the FrameBuffers where the Renderables will
		 * be drawn
		 *
		 * @param	width the new width of the FrameBuffers where the
		 *			Renderables will be drawn
		 * @param	height the new height of the FrameBuffers where the
		 *			Renderables will be drawn */
		virtual void setResolution(std::size_t width, std::size_t height);
	private:
		/** Adds shared resources to the RenderGraph resource node
		 *
		 * @param	repository the Repository that holds the Resources
		 * @param	width the initial width of the FrameBuffer where the
		 *			Entities are going to be rendered
		 * @param	height the initial height of the FrameBuffer where the
		 *			Entities are going to be rendered
		 * @return	true if the resources were added succesfully,
		 *			false otherwise */
		bool addResources(
			Repository& repository, std::size_t width, std::size_t height
		);

		/** Adds nodes to the RenderGraph and links them. It will add:
		 *  - "renderer2D" - renders Renderable2Ds
		 *  - the renderers of @see addShadowRenderers,
		 *    @see addDeferredRenderers and @see addForwardRenderers
		 *
		 * @param	repository the Repository that holds the Resources
		 * @param	width the initial width of the FrameBuffer where the
		 *			Entities are going to be rendered
		 * @param	height the initial height of the FrameBuffer where the
		 *			Entities are going to be rendered
		 * @return	true if the nodes were added and linked succesfully,
		 *			false otherwise */
		bool addNodes(
			Repository& repository, std::size_t width, std::size_t height
		);

		/** Creates the Renderer3Ds used for rendering shadows with the names
		 *	"shadowRendererMesh" - renders RenderableMeshes
		 *	"shadowRendererTerrain" - renders RenderableTerrains
		 * The nodes and connections that will be added to the graph looks like
		 * the following (the resource node already exists):
		 *
		 *  [        "resources"        ]     |                   |attach
		 *     |shadowTexture |shadowBuffer   |attach  ["shadowRendererTerrain"]
		 *     |              |        ["shadowRendererMesh"]     |attach
		 *     |     ["shadowFBClear"]       |attach              |
		 *     |              |_________     |__         _________|
		 *     |                       |       |        |
		 *     |    |position | normal |target |attach1 |attach2
		 *     |  [        "shadowRenderSubGraph"        ]
		 *     |                       |target
		 *
		 * @param	repository the Repository that holds the Resources
		 * @param	width the initial width of the FrameBuffer where the
		 *			Entities are going to be rendered
		 * @param	height the initial height of the FrameBuffer where the
		 *			Entities are going to be rendered
		 * @return	true if the nodes where added successfully, false
		 *			otherwise */
		bool addShadowRenderers(
			Repository& repository, std::size_t width, std::size_t height
		);

		/** Creates a deferred renderer with the following GBuffer Renderer3Ds:
		 *	"gBufferRendererTerrain" - renders RenderableTerrains
		 *	"gBufferRendererMesh" - renders RenderableMeshes
		 *	"gBufferRendererParticles" - renders ParticleSystems
		 * The nodes and connections that will be added to the graph looks like
		 * the following (the resource node already exists):
		 *
		 *  [                            "resources"                        ]
		 *    |gBuffer  |deferredBuffer              |positionTexture
		 *    \         |________________            |
		 *    |input                    |            |
		 *  ["gFBClear"]                |            |
		 *    |output                   |            |
		 *    |                         |  ["texUnitNodePosition"]
		 *    |target                   |            |
		 *  ["gBufferRendererTerrain"]  |            |
		 *    |target                   |            |
		 *    |                         |            |
		 *    |target                   |            |
		 *  ["gBufferRendererMesh"]     |            |
		 *    |target                   |            |
		 *    |                         |            |
		 *    |target                   |            |
		 *  ["gBufferRendererParticles"]|            |
		 *    |target |attach         __|            |__________
		 *    |   ____|             /                          |
		 *    |  |attach1 |attach2 |target |irradiance |shadow |position
		 *    | [            "deferredLightRenderer"            ]
		 *    |                         |target
		 *
		 * @note	where irradiance appears, there will be also simila
		 *			connections for prefilter and brdf textures
		 * @note	where position appears, there will be also similar nodes
		 *			and connections for the normal, albedo, material
		 *			and emissive textures
		 *
		 * @param	repository the Repository that holds the Resources
		 * @return	true if the nodes where added successfully, false
		 *			otherwise */
		bool addDeferredRenderers(Repository& repository);

		/** Creates the following forward renderers and adds them to the given
		 * RenderGraph:
		 *	"forwardRendererMesh" - renders RenderableMeshes
		 * The nodes and connections that will be added to the graph looks like
		 * the following (the resource node already exists):
		 *
		 *      |target  |irradiance  |prefilter  |brdf  |color  |bright
		 *  [                    "forwardRendererMesh"                    ]
		 *                 |target      |color      |bright
		 *
		 * @return	true if the nodes where added successfully, false
		 *			otherwise */
		bool addForwardRenderers();
	};

}

#endif		// APP_RENDER_GRAPH_H
