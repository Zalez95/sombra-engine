#ifndef DEFAULT_SHADER_BUILDER_H
#define DEFAULT_SHADER_BUILDER_H

#include <se/app/Application.h>
#include <se/graphics/Renderer.h>
#include <se/graphics/GraphicsEngine.h>
#include <se/app/io/SceneImporter.h>
#include <se/app/io/ShaderLoader.h>

using namespace se::app;

namespace editor {

	/** Class DefaultShaderBuilder, it's the default ShaderBuilder, it
	 * creates RenderableShaders with the PBR and Shadow passes */
	class DefaultShaderBuilder : public se::app::SceneImporter::ShaderBuilder
	{
	private:	// Attributes
		/** A reference to the Application that holds the EventManager used
		 * for creating the RenderableShaders */
		se::app::Application& mApplication;

		/** The repsoitory that holds the passes of the RenderableShaders */
		se::app::Repository& mRepository;

	public:		// Functions
		/** Creates a new DefaultShaderBuilder
		 *
		 * @param	application the Application needed for creating
		 *			RenderableShaders
			* @param	repository the Repository needed for creating
			*			RenderableShaders */
		DefaultShaderBuilder(
			se::app::Application& application,
			se::app::Repository& repository
		) : mApplication(application), mRepository(repository) {};

		/** @copydoc ShaderBuilder::createShader(const Material&, bool) */
		virtual ShaderRef createShader(const char* name, const se::app::Material& material, bool hasSkin) override
		{
			const char* shadowStepName = hasSkin? "stepShadowSkinning" : "stepShadow";
			auto shadowStep = mRepository.findByName<se::app::RenderableShaderStep>(shadowStepName);
			if (!shadowStep) {
				return ShaderRef();
			}

			auto& renderGraph = mApplication.getExternalTools().graphicsEngine->getRenderGraph();
			auto gBufferRendererMesh = dynamic_cast<se::graphics::Renderer*>(renderGraph.getNode("gBufferRendererMesh"));
			if (!gBufferRendererMesh) {
				return ShaderRef();
			}

			const char* programName = hasSkin? "programGBufMaterialSkinning" : "programGBufMaterial";
			auto program = mRepository.findByName<se::graphics::Program>(programName);
			if (!program) {
				return ShaderRef();
			}

			auto step = mRepository.insert(std::make_shared<se::app::RenderableShaderStep>(*gBufferRendererMesh), name);
			se::app::ShaderLoader::addMaterialBindables(step, material, program);

			auto shader = mRepository.emplace<se::app::RenderableShader>(mApplication.getEventManager());
			shader.getResource().setName(name);
			shader->addStep(shadowStep)
				.addStep(step);

			return shader;
		};
	};

}

#endif		// DEFAULT_SHADER_BUILDER_H
