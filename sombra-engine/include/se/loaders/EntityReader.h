#ifndef ENTITY_READER_H
#define ENTITY_READER_H

#include "MeshReader.h"
#include "MaterialReader.h"
#include "../app/GraphicsManager.h"
#include "../app/PhysicsManager.h"
#include "../app/CollisionManager.h"

namespace se::app { struct Entity; }
namespace se::utils { class FileReader; }

namespace se::loaders {

	/**
	 * Class EntityReader, it's used to create meshes from raw data or from the
	 * given files
	 */
	class EntityReader
	{
	private:	// Nested types
		using EntityUPtr = std::unique_ptr<app::Entity>;

		/** Struct FileFormat, it holds the name, version and other data of
		 * our Entity file format */
		struct FileFormat
		{
			static const std::string sFileName;
			static const std::string sFileExtension;
			static const int sVersion = 1;
			static const int sRevision = 3;
		};

		/** Struct SharedData, it holds the data loaded by other loaders that
		 * the Entities will share */
		struct SharedData;

	private:	// Attributes
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

		/** The CollisionManager that will store the readed collider data */
		app::CollisionManager& mCollisionManager;

	public:		// Functions
		/** Creates a new EntityReader
		 *
		 * @param	graphicsManager the GraphicsManager where we will store
		 *			the readed graphics data
		 * @param	physicsManager the PhysicsManager where we will store
		 *			the readed physics data
		 * @param	collisionManager the CollisionManager where we will store
		 *			the readed collider data */
		EntityReader(
			app::GraphicsManager& graphicsManager,
			app::PhysicsManager& physicsManager,
			app::CollisionManager& collisionManager
		) : mGraphicsManager(graphicsManager),
			mPhysicsManager(physicsManager),
			mCollisionManager(collisionManager) {};

		/** Class destructor */
		~EntityReader() {};

		/** Parses the Entities in the given file and returns them
		 *
		 * @note	the cursor of the file reader will be moved after parsing
		 *			the file
		 * @param	fileReader the file reader with the Entities that we want
		 *			to parse
		 * @return	a vector with the parsed Entities
		 * @throw	runtime_error in case of any error while parsing */
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
		 * @return	a struct holding the data
		 * @throw	runtime_error in case of any error while parsing */
		SharedData loadSharedData(utils::FileReader& fileReader) const;

		/** Parses the Entities in the given file and returns them
		 *
		 * @param	fileReader the file reader with the entities that we want
		 *			to parse
		 * @param	sharedData the data needed for creating the Entities
		 * @return	a vector with the parsed entities
		 * @throw	runtime_error in case of any error while parsing */
		std::vector<EntityUPtr> parseEntities(
			utils::FileReader& fileReader, SharedData& sharedData
		) const;

		/** Parses the Entity at the current position of the given file and
		 * returns it
		 *
		 * @param	fileReader the file reader with the file that we want
		 *			to read
		 * @param	sharedData the data needed for creating the Entities
		 * @return	a pointer to the parsed Entity
		 * @throw	runtime_error in case of any error while parsing */
		EntityUPtr parseEntity(
			utils::FileReader& fileReader, SharedData& sharedData
		) const;
	};

}

#endif		// ENTITY_READER_H
