#ifndef APP_RENDER_GRAPH_H
#define APP_RENDER_GRAPH_H

#include "se/graphics/RenderGraph.h"

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
	 * For rendering shadows there are available a "shadowTerrainProxyRenderer"
	 * and a "shadowMeshProxyRenderer" renderers, particle can't cast shadows.
	 * For the Renderable2Ds, there is a "renderer2D" for submitting them
	 */
	class AppRenderGraph : public graphics::RenderGraph
	{
	private:	// Nested types
		class CombineNode;

	public:		// Functions
		/** Creates a new AppRenderGraph
		 *
		 * @param	context the graphics Context used for creating the Bindables
		 * @param	width the initial width of the FrameBuffers where the
		 *			Renderables will be drawn
		 * @param	height the initial height of the FrameBuffers where the
		 *			Renderables will be drawn */
		AppRenderGraph(
			graphics::Context& context, std::size_t width, std::size_t height
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
		void setResolution(std::size_t width, std::size_t height);
	private:
		/** Adds shared resources to the RenderGraph resource node
		 *
		 * @param	context the graphics Context used for creating the Bindables
		 * @param	width the initial width of the FrameBuffer where the
		 *			Entities are going to be rendered
		 * @param	height the initial height of the FrameBuffer where the
		 *			Entities are going to be rendered
		 * @return	true if the resources were added succesfully,
		 *			false otherwise */
		bool addResources(
			graphics::Context& context, std::size_t width, std::size_t height
		);

		/** Adds nodes to the RenderGraph and links them. It will add:
		 *  - "renderer2D" - renders Renderable2Ds
		 *  - the renderers of @see addShadowRenderers,
		 *    @see addDeferredRenderers and @see addForwardRenderers
		 *
		 * @param	context the graphics Context used for creating the Bindables
		 * @param	width the initial width of the FrameBuffer where the
		 *			Entities are going to be rendered
		 * @param	height the initial height of the FrameBuffer where the
		 *			Entities are going to be rendered
		 * @return	true if the nodes were added and linked succesfully,
		 *			false otherwise */
		bool addNodes(
			graphics::Context& context, std::size_t width, std::size_t height
		);

		/** Creates a deferred renderer with the following GBuffer Renderer3Ds:
		 *	"gBufferRendererTerrain" - renders RenderableTerrains
		 *	"gBufferRendererMesh" - renders RenderableMeshes
		 *	"gBufferRendererParticles" - renders ParticleSystems
		 *	"shadowTerrainProxyRenderer" - renders RenderableTerrains shadows
		 *	"shadowMeshProxyRenderer" - renders RenderableMeshes shadows
		 * The nodes and connections that will be added to the graph looks like
		 * the following (the resource node already exists):
		 *
		 *  [                            "resources"                        ]
		 *    |gBuffer  |ssaoBuffer  |positionTexture |normalTexture |planeMesh
		 *    |         |            |____            |              |
		 *    |         |_______________  |           |              |
		 *    |target                   | |           |               \
		 *  ["gFBClear"]                | |           |                |
		 *    |target                   | |           |______________  |
		 *    |                         | |input                     | |
		 *    |target                   |["texUnitNodeSSAOPosition"] | |
		 *  ["gBufferRendererTerrain"]  | |output        ____________| |
		 *    |target                   | |             |input         |
		 *    |                         | | ["texUnitNodeSSAONormal"]  |
		 *    |target                   | |             |output        |
		 *  ["gBufferRendererMesh"]     | |  ___________|              |
		 *    |target                   | | | _________________________|
		 *    |                         | | ||                         |
		 *    |target                   | | ||                         |
		 *  ["gBufferRendererParticles"]| | ||                         |
		 *    |target |attach   ________| | ||                         |
		 *    |       |        |          | ||                         |
		 *    |       |attach  |target    | ||                         |
		 *    |    ["ssaoFBClear"]        | ||                         |
		 *    |      |target _____________| ||                         |
		 *    |     /       /         ______||                         |
		 *    |    |target |position |normal |plane                    |
		 *    |  [         "ssaoNode"         ]                        |
		 *    |    |position |target |attach                           |
		 *    |    |  _______|       |                                 |
		 *    |    | |      _________|                                 |_______
		 *    |    | |     | [            "resources"            ]             |
		 *    |    | |     |   |ssaoHBlurFBClear |ssaoTexture |ssaoHBlurTexture|
		 *    |    | |     |   |                 |__          |________        |
		 *    |    | |     |   |                    |input             |       |
		 *    |    | |     |   |         ["ssaoHBlurTexUnitNode"]      |       |
		 *    |    | |     |   |___                 |output            |       |
		 *    |    | |     |       |                |                  |       |
		 *    |    | |     |attach |target          |           _______|_______|
		 *    |    | |["ssaoHBlurFBClear"]          |          |       |       |
		 *    |    | |          |target             |___       |       |       |
		 *    |    | |          |____________           |      |       |       |
		 *    |    | |                       |target    |input |plane  |       |
		 *    |    | |                      [   "ssaoHBlurNode"   ]    |       |
		 *    |    | |______________          _________|attach         |       |
		 *    |    |                |target  |attach                   |input  |
		 *    |    |input        ["ssaoVBlurFBClear"]  ["ssaoVBlurTexUnitNode"]|
		 *    |["texUnitNodeAmbientPosition"] |target       |output            |
		 *    |             |output           |__         __|    ______________|
		 *    |       ______|                    |target |input |plane         |
		 *    |      |                        [  "ssaoVBlurNode"  ]            |
		 *    |      |  [         "resources"         ]  |attach               |
		 *    |      |   |deferredBuffer |ssaoTexture    |                     |
		 *    |      |   |               |________       |                     |
		 *    |      |   |target                  |input |attach               |
		 *    |      |  ["deferredFBClear"] ["texUnitNodeAmbientSSAO"]         |
		 *    |      |           |target               |output                 |
		 *    |      |           |        _____________|                       |
		 *    |      |           |       |      _______________________________|
		 *    |      |position   |target |ssao |plane
		 *    |    [    "deferredAmbientRenderer"    ]  ["resources"]
		 *    |                    |target                   |positionTexture
		 *    |        ____________|                         |
		 *    |input1 |input2                                |input
		 *  ["zBufferCopy"]                      ["texUnitNodeLightPosition"]
		 *    |output ["shadowTerrainProxyRenderer"]         |output
		 *    |         |attach ["shadowMeshProxyRenderer"]  |
		 *    |         |              |attach    ___________|
		 *    |target   |attach1       |attach2  |position
		 *  [        "deferredLightSubGraph"        ]
		 *                      |target
		 *
		 * @note	where position appears, there will be also similar nodes
		 *			and connections for the normal, albedo, material
		 *			and emissive textures (emissive only for the
		 *			deferredAmbientRenderer)
		 *
		 * @param	context the graphics Context used for creating the Bindables
		 * @param	width the initial width of the FrameBuffer where the
		 *			Entities are going to be rendered
		 * @param	height the initial height of the FrameBuffer where the
		 *			Entities are going to be rendered
		 * @return	true if the nodes where added successfully, false
		 *			otherwise */
		bool addDeferredRenderers(
			graphics::Context& context, std::size_t width, std::size_t height
		);

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
