#ifndef SCENE_READER_H
#define SCENE_READER_H

#include <memory>
#include "../../graphics/Technique.h"
#include "../graphics/Material.h"
#include "../Scene.h"
#include "../Application.h"
#include "Result.h"

namespace se::app {

	/**
	 * Class SceneReader, it's used for loading the full scenes stored in the
	 * given files
	 */
	class SceneReader
	{
	public:		// Nested types
		using SceneReaderUPtr = std::unique_ptr<SceneReader>;
		using TechniqueSPtr = std::shared_ptr<graphics::Technique>;

		/** The scenes files than are supported by the SceneReader */
		enum class FileType
		{
			GLTF
		};

		/** Class TechniqueBuilder, it's an interface used for creating
		 * Techniques from Materials */
		class TechniqueBuilder
		{
		public:		// Functions
			/** Class destructor */
			virtual ~TechniqueBuilder() = default;

			/** Creates a new Technique from the given Material
			 *
			 * @param	material the material used for creating the Technique
			 * @param	hasSkin if the technique must support skeletal animation
			 *			or not
			 * @return	a pointer to the new Technique */
			virtual TechniqueSPtr createTechnique(
				const Material& material, bool hasSkin
			) = 0;
		};

	protected:	// Attributes
		/** A reference to the Application that holds the EntityDatabase used
		 * for creating the Entities */
		Application& mApplication;

		/** A reference to the TechniqueBuilder used for creating the
		 * Techniques */
		TechniqueBuilder& mTechniqueBuilder;

	public:		// Functions
		/** Creates a new SceneReader
		 *
		 * @param	application the Application that holds the EntityDatabase
		 *			used for creating the Entities
		 * @param	techniqueBuilder the TechniqueBuilder used for creating the
		 *			Techniques */
		SceneReader(
			Application& application, TechniqueBuilder& techniqueBuilder
		) : mApplication(application), mTechniqueBuilder(techniqueBuilder) {};

		/** Class destructor */
		virtual ~SceneReader() = default;

		/** Creates a SceneReader capable of reading the given file format
		 *
		 * @param	fileType the FileType that we want to read
		 * @param	application the Application that holds the EntityDatabase
		 *			used for creating the Entities
		 * @param	techniqueBuilder the TechniqueBuilder used for creating the
		 *			Techniques
		 * @return	a pointer to the new SceneReader */
		static SceneReaderUPtr createSceneReader(
			FileType fileType,
			Application& application, TechniqueBuilder& techniqueBuilder
		);

		/** Parses the given file and stores the result in the given Scenes
		 * object
		 *
		 * @param	path the path to the file to parse
		 * @param	output the Scene where the file data will be stored
		 * @return	a Result object with the result of the load operation
		 * @note	if there is any mesh without a material in the given file,
		 *			we will use the default Techniques of the Scene repository
		 *			(it will be created if it doesn't exist) */
		virtual Result load(const std::string& path, Scene& output) = 0;
	protected:
		/** Creates a "defaultTechnique" and a "defaultTechniqueSkin" in the
		 * given Scene repository if it doesn't exist yet
		 *
		 * @param	scene the Scene where the Techniques are going to be
		 *			added */
		void createDefaultTechniques(Scene& scene);
	};

}

#endif		// SCENE_READER_H
