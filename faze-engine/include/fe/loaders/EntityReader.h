#ifndef ENTITY_READER_H
#define ENTITY_READER_H

#include "MeshLoader.h"
#include "MeshReader.h"
#include "MaterialReader.h"
#include "../app/GraphicsManager.h"
#include "../app/PhysicsManager.h"

namespace fe { namespace app { struct Entity; }}
namespace fe { namespace utils { class FileReader; }}

namespace fe { namespace loaders {

	/**
	 * Class EntityReader, it's used to create meshes from raw data or from the
	 * given files
	 */
	class EntityReader
	{
	private:	// Nested types
		typedef std::unique_ptr<app::Entity> EntityUPtr;

		/** Struct FileFormat, it holds the name, version and other data of
		 * our Entity file format */
		struct FileFormat
		{
			static const std::string	sFileName;
			static const std::string	sFileExtension;
			static const unsigned int	sVersion = 1;
			static const unsigned int	sRevision = 3;
		};

		/** Struct SharedData, it holds the data loaded by other loaders that
		 * the Entities will share */
		struct SharedData;

	private:	// Attributes
		/** The MeshLoader used for loading the Meshes for the
		 * graphics data of the Entities */
		MeshLoader mMeshLoader;

		/** The MeshReader used for reading the Meshes for the
		 * graphics data of the Entities */
		MeshReader mMeshReader;

		/** The MaterialReader used for creating the Materials for the
		 * graphics data of the Entities */
		MaterialReader mMaterialReader;

		/** The GraphicsManager that will store the readed graphics data */
		app::GraphicsManager& mGraphicsManager;

		/** The PhysicsManager that will store the readed physics data */
		app::PhysicsManager& mPhysicsManager;

	public:		// Functions
		/** Creates a new EntityReader
		 *
		 * @param	graphicsManager the GraphicsManager where we will store
		 *			the readed graphics data
		 * @param	physicsManager the PhysicsManager where we will store
		 *			the readed physics data */
		EntityReader(
			app::GraphicsManager& graphicsManager,
			app::PhysicsManager& physicsManager
		) : mGraphicsManager(graphicsManager),
			mPhysicsManager(physicsManager) {};

		/** Class destructor */
		~EntityReader() {};

		/** Parses the Entities in the given file and returns them
		 *
		 * @note	the cursor of the file reader will be moved after parsing
		 *			the file
		 * @param	fileReader the file reader with the Entities that we want
		 *			to parse
		 * @return	a vector with the parsed Entities */
		std::vector<EntityUPtr> load(utils::FileReader& fileReader);
	private:
		/** Checks the header of the given file
		 *
		 * @param	fileReader the reader of the file with the header we want
		 *			to check
		 * @return	true if the file and version is ok, false otherwise */
		bool checkHeader(utils::FileReader& fileReader) const;

		/** Loads the shared data needed to create the Entities
		 *
		 * @param	fileReader the file reader with the meshes that we want to
		 *			parse
		 * @return	a struct holding the data */
		SharedData loadSharedData(utils::FileReader& fileReader) const;

		/** Parses the Entities in the given file and returns them
		 *
		 * @param	fileReader the file reader with the entities that we want
		 *			to parse
		 * @param	sharedData the data needed for creating the Entities
		 * @return	a vector with the parsed entities */
		std::vector<EntityUPtr> parseEntities(
			utils::FileReader& fileReader, SharedData& sharedData
		) const;

		/** Parses the Entity at the current position of the given file and
		 * returns it
		 *
		 * @param	fileReader the file reader with the file that we want
		 *			to read
		 * @param	sharedData the data needed for creating the Entities
		 * @return	a pointer to the parsed Entity */
		EntityUPtr parseEntity(
			utils::FileReader& fileReader, SharedData& sharedData
		) const;
	};

}}

#endif		// ENTITY_READER_H
