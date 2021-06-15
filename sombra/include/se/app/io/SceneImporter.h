#ifndef SCENE_IMPORTER_H
#define SCENE_IMPORTER_H

#include <memory>
#include "../graphics/Material.h"
#include "../graphics/RenderableShader.h"
#include "../Scene.h"
#include "Result.h"

namespace se::app {

	/**
	 * Class SceneImporter, it's used for loading the full scenes stored in the
	 * given files
	 */
	class SceneImporter
	{
	public:		// Nested types
		using SceneImporterUPtr = std::unique_ptr<SceneImporter>;

		/** The scenes files than are supported by the SceneImporter */
		enum class FileType
		{
			GLTF = 0
		};

		/** Class ShaderBuilder, it's an interface used for creating
		 * Shaders from Materials */
		class ShaderBuilder
		{
		public:		// Nested types
			using ShaderRef = Repository::ResourceRef<RenderableShader>;

		public:		// Functions
			/** Class destructor */
			virtual ~ShaderBuilder() = default;

			/** Creates a new Shader from the given Material
			 *
			 * @param	name the name used for storing the RenderableShader and
			 *			its Steps in the scene repository
			 * @param	material the material used for creating the Shader
			 * @param	hasSkin if the Shader must support skeletal animation
			 *			or not
			 * @return	the new Shader */
			virtual ShaderRef createShader(
				const char* name, const Material& material, bool hasSkin
			) = 0;
		};

	protected:	// Attributes
		/** A reference to the ShaderBuilder used for creating the Shaders */
		ShaderBuilder& mShaderBuilder;

		/** The default shader of the ShaderBuilder */
		Repository::ResourceRef<RenderableShader> mDefaultShader;

		/** The default shader with skinning of the ShaderBuilder */
		Repository::ResourceRef<RenderableShader> mDefaultShaderSkin;

	public:		// Functions
		/** Class destructor */
		virtual ~SceneImporter() = default;

		/** Creates a SceneImporter capable of reading the given file format
		 *
		 * @param	fileType the FileType that we want to read
		 * @param	shaderBuilder the ShaderBuilder used for creating the
		 *			Shaders
		 * @return	a pointer to the new SceneImporter */
		static SceneImporterUPtr createSceneImporter(
			FileType fileType, ShaderBuilder& shaderBuilder
		);

		/** Parses the given file and stores the result in the given Scenes
		 * object
		 *
		 * @param	path the path to the file to parse
		 * @param	output the Scene where the file data will be stored
		 * @return	a Result object with the result of the load operation
		 * @note	if there is any mesh without a material in the given file,
		 *			we will use the default Shaders of the Scene repository
		 *			(it will be created if it doesn't exist) */
		virtual Result load(const std::string& path, Scene& output) = 0;
	protected:
		/** Creates a new SceneImporter
		 *
		 * @param	shaderBuilder the ShaderBuilder used for creating the
		 *			Shaders */
		SceneImporter(ShaderBuilder& shaderBuilder) :
			mShaderBuilder(shaderBuilder) {};

		/** Creates a "defaultShader" and a "defaultShaderSkin" in the
		 * given Scene repository if it doesn't exist yet
		 *
		 * @param	scene the Scene where the Shaders are going to be added */
		void createDefaultShaders(Scene& scene);
	};

}

#endif		// SCENE_IMPORTER_H
