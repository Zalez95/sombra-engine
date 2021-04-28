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
		se::utils::Repository& mRepository;

	public:		// Functions
		/** Creates a new DefaultShaderBuilder
		 *
		 * @param	application the Application needed for creating
		 *			RenderableShaders
			* @param	repository the Repository needed for creating
			*			RenderableShaders */
		DefaultShaderBuilder(
			se::app::Application& application,
			se::utils::Repository& repository
		) : mApplication(application), mRepository(repository) {};

		/** @copydoc ShaderBuilder::createShader(const Material&, bool) */
		virtual ShaderSPtr createShader(const se::app::Material& material, bool hasSkin) override
		{
			std::string shadowPassKey = hasSkin? "passShadowSkinning" : "passShadow";
			auto shadowPass = mRepository.find<std::string, se::graphics::Pass>(shadowPassKey);

			auto& renderGraph = mApplication.getExternalTools().graphicsEngine->getRenderGraph();
			auto gBufferRendererMesh = static_cast<se::graphics::Renderer*>(renderGraph.getNode("gBufferRendererMesh"));
			std::string programKey = hasSkin? "programGBufMaterialSkinning" : "programGBufMaterial";
			auto program = mRepository.find<std::string, se::graphics::Program>(programKey);
			auto pass = std::make_shared<se::graphics::Pass>(*gBufferRendererMesh);
			pass->addBindable(program);
			se::app::ShaderLoader::addMaterialBindables(pass, material, program);

			auto shader = std::make_shared<se::app::RenderableShader>(mApplication.getEventManager());
			shader->addPass(shadowPass)
				.addPass(pass);

			return shader;
		};
	};

}

#endif		// DEFAULT_SHADER_BUILDER_H
