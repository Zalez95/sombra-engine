#include <algorithm>
#include "se/loaders/EntityReader.h"
#include "se/loaders/MeshLoader.h"
#include "se/utils/FileReader.h"
#include "se/app/Entity.h"
#include "se/graphics/3D/Camera.h"
#include "se/graphics/3D/Lights.h"
#include "se/graphics/3D/Renderable3D.h"
#include "se/graphics/3D/Mesh.h"
#include "se/graphics/3D/Material.h"
#include "se/physics/RigidBody.h"
#include "se/collision/ConvexPolyhedron.h"
#include "se/collision/QuickHull.h"
#include "se/loaders/RawMesh.h"

namespace se::loaders {

// Nested types definition
	struct EntityReader::SharedData
	{
		std::vector<std::shared_ptr<RawMesh>> rawMeshes;
		std::vector<std::shared_ptr<graphics::Mesh>> meshes;
		std::vector<std::shared_ptr<graphics::Material>> materials;
		std::vector<std::shared_ptr<graphics::Texture>> textures;
	};

// Static variables definition
	const std::string EntityReader::FileFormat::sFileName		= "SOMBRA_ENT_FILE";
	const std::string EntityReader::FileFormat::sFileExtension	= ".seent";

// Public Functions
	std::vector<EntityReader::EntityUPtr> EntityReader::load(utils::FileReader& fileReader)
	{
		try {
			// 1. Get the input file
			if (fileReader.getState() != utils::FileState::OK) {
				throw std::runtime_error("Error reading the file");
			}

			// 2. Check the file header
			if (!checkHeader(fileReader)) {
				throw std::runtime_error("Error with the header of the file");
			}

			// 3. Load the SharedData
			SharedData sharedData = loadSharedData(fileReader);

			// 4. Parse the Entities
			return parseEntities(fileReader, sharedData);
		}
		catch (const std::exception& e) {
			throw std::runtime_error("Error parsing the Entity in the file \"" + fileReader.getFilePath() + "\": " + e.what());
		}
	}

// Private functions
	bool EntityReader::checkHeader(utils::FileReader& fileReader) const
	{
		const std::string curFileVersion = std::to_string(FileFormat::sVersion) + '.' + std::to_string(FileFormat::sRevision);
		bool ret = false;

		std::string fileName, fileVersion;
		fileReader >> fileName >> fileVersion;
		if ((fileReader.getState() == utils::FileState::OK) &&
			(fileName == FileFormat::sFileName) &&
			(fileVersion == curFileVersion)
		) {
			ret = true;
		}

		return ret;
	}


	EntityReader::SharedData EntityReader::loadSharedData(utils::FileReader& fileReader) const
	{
		SharedData ret;
		std::string trash;

		bool end = false;
		while (!end) {
			std::string token; fileReader >> token;

			if (token == "files") {
				fileReader >> trash;
			}
			else if (token == "mesh") {
				fileReader >> token;
				utils::FileReader meshFileReader(token);
				auto rawMeshes = mMeshReader.read(meshFileReader);
				ret.rawMeshes.reserve(rawMeshes.size());
				ret.rawMeshes.insert(
					ret.rawMeshes.end(),
					std::make_move_iterator(rawMeshes.begin()),
					std::make_move_iterator(rawMeshes.end())
				);
			}
			else if (token == "material") {
				fileReader >> token;
				utils::FileReader materialFileReader(token);
				auto materials = mMaterialReader.read(materialFileReader);
				ret.materials.reserve(materials.size());
				ret.materials.insert(
					ret.materials.end(),
					std::make_move_iterator(materials.begin()),
					std::make_move_iterator(materials.end())
				);
			}
			else if (token == "}") { end = true; }
			else {
				throw std::runtime_error("Error: unexpected word \"" + token + "\" at line " + std::to_string(fileReader.getNumLines()));
			}
		}

		return ret;
	}


	std::vector<EntityReader::EntityUPtr> EntityReader::parseEntities(
		utils::FileReader& fileReader, SharedData& sharedData
	) const
	{
		std::vector<EntityUPtr> entities;
		unsigned int nEntities = 0, iEntity = 0;

		std::string token;
		while (fileReader.getValue(token) == utils::FileState::OK) {
			if (token == "num_entities") {
				if (fileReader.getValue(nEntities) == utils::FileState::OK) {
					entities.reserve(nEntities);
				}
			}
			else if (token == "entity") {
				auto curEntity = parseEntity(fileReader, sharedData);
				if (iEntity < nEntities) {
					entities.push_back(std::move(curEntity));
				}
				++iEntity;
			}
			else {
				throw std::runtime_error("Error: unexpected word \"" + token + "\" at line " + std::to_string(fileReader.getNumLines()));
			}
		}

		if (iEntity != nEntities) {
			throw std::runtime_error("Error: expected " + std::to_string(nEntities) + " entities, parsed " + std::to_string(iEntity));
		}

		return entities;
	}


	EntityReader::EntityUPtr EntityReader::parseEntity(
		utils::FileReader& fileReader, SharedData& sharedData
	) const
	{
		std::string name;
		glm::vec3 position(0.0f);
		glm::quat orientation(1.0f, glm::vec3(0.0f));
		glm::mat4 offsetMatrix(0.0f);
		std::unique_ptr<graphics::Camera> camera;
		std::unique_ptr<graphics::PointLight> pointLight;
		std::unique_ptr<graphics::Renderable3D> renderable3D;
		std::unique_ptr<physics::RigidBody> rigidBody;
		std::unique_ptr<collision::Collider> collider;

		std::string trash;
		fileReader >> name >> trash;

		bool end = false;
		while (!end) {
			std::string token; fileReader >> token;

			if (token == "name") {
				fileReader >> name;
			}
			else if (token == "position") {
				fileReader >> position.x >> position.y >> position.z;
			}
			else if (token == "orientation") {
				fileReader >> orientation.w >> orientation.x >> orientation.y >> orientation.z;
			}
			else if (token == "camera") {
				glm::vec3 cameraPosition, cameraTarget, cameraUp;
				fileReader >> cameraPosition.x	>> cameraPosition.y	>> cameraPosition.z;
				fileReader >> cameraTarget.x	>> cameraTarget.y	>> cameraTarget.z;
				fileReader >> cameraUp.x		>> cameraUp.y		>> cameraUp.z;

				camera = std::make_unique<graphics::Camera>(cameraPosition, cameraTarget, cameraUp);
			}
			else if (token == "renderable3D") {
				std::string meshName, materialName, textureName;
				fileReader >> meshName >> materialName >> textureName >>
					offsetMatrix[0][0] >> offsetMatrix[0][1] >> offsetMatrix[0][2] >> offsetMatrix[0][3] >>
					offsetMatrix[1][0] >> offsetMatrix[1][1] >> offsetMatrix[1][2] >> offsetMatrix[1][3] >>
					offsetMatrix[2][0] >> offsetMatrix[2][1] >> offsetMatrix[2][2] >> offsetMatrix[2][3] >>
					offsetMatrix[3][0] >> offsetMatrix[3][1] >> offsetMatrix[3][2] >> offsetMatrix[3][3];

				auto itMesh = std::find_if(
					sharedData.meshes.begin(), sharedData.meshes.end(),
					[&meshName](std::shared_ptr<graphics::Mesh> mesh) {
						return mesh->getName() == meshName;
					}
				);

				if (itMesh == sharedData.meshes.end()) {
					auto itRawMesh = std::find_if(
						sharedData.rawMeshes.begin(), sharedData.rawMeshes.end(),
						[&meshName](std::shared_ptr<RawMesh> rawMesh) {
							return rawMesh->name == meshName;
						}
					);

					if (itRawMesh != sharedData.rawMeshes.end()) {
						sharedData.meshes.push_back( MeshLoader::createGraphicsMesh(**itRawMesh) );
						itMesh = sharedData.meshes.end();
					}
				}

				auto itMaterial = std::find_if(
					sharedData.materials.begin(), sharedData.materials.end(),
					[&materialName](std::shared_ptr<graphics::Material> material) {
						return material->getName() == materialName;
					}
				);

				renderable3D = std::make_unique<graphics::Renderable3D>(*itMesh, *itMaterial, nullptr);
			}
			else if (token == "point_light") {
				// TODO: read light
			}
			else if (token == "physics") {
				rigidBody = std::make_unique<physics::RigidBody>();
			}
			else if (token == "collision") {
				std::string meshName;
				glm::mat4 mat;
				fileReader >> meshName >>
					mat[0][0] >> mat[0][1] >> mat[0][2] >> mat[0][3] >>
					mat[1][0] >> mat[1][1] >> mat[1][2] >> mat[1][3] >>
					mat[2][0] >> mat[2][1] >> mat[2][2] >> mat[2][3] >>
					mat[3][0] >> mat[3][1] >> mat[3][2] >> mat[3][3];

				auto itRawMesh = std::find_if(
					sharedData.rawMeshes.begin(), sharedData.rawMeshes.end(),
					[&meshName](std::shared_ptr<RawMesh> rawMesh) {
						return rawMesh->name == meshName;
					}
				);
				if (itRawMesh != sharedData.rawMeshes.end()) {
					auto heMesh = MeshLoader::createHalfEdgeMesh(**itRawMesh);
					if (heMesh) {
						// TODO: move to header
						collision::QuickHull qh(0.001f);
						qh.calculate(*heMesh);
						collider = std::make_unique<collision::ConvexPolyhedron>(qh.getMesh());
					}
				}
			}
			else if (token == "}") { end = true; }
			else {
				throw std::runtime_error("Error: unexpected word \"" + token + "\" at line " + std::to_string(fileReader.getNumLines()));
			}
		}

		auto entity = std::make_unique<app::Entity>(name);
		entity->position	= position;
		entity->orientation	= orientation;

		if (camera) {
			mGraphicsManager.addEntity(entity.get(), std::move(camera));
		}
		if (pointLight) {
			mGraphicsManager.addEntity(entity.get(), std::move(pointLight));
		}
		if (renderable3D) {
			mGraphicsManager.addEntity(entity.get(), std::move(renderable3D), offsetMatrix);
		}
		if (rigidBody) {
			mPhysicsManager.addEntity(entity.get(), std::move(rigidBody));
		}
		if (collider) {
			mCollisionManager.addEntity(entity.get(), std::move(collider));
		}

		return std::move(entity);
	}

}
