#include "fe/loaders/EntityReader.h"
#include <algorithm>
#include "fe/utils/FileReader.h"
#include "fe/app/Entity.h"
#include "fe/graphics/3D/Renderable3D.h"
#include "fe/graphics/3D/Mesh.h"
#include "fe/graphics/3D/Material.h"
#include "fe/physics/PhysicsEntity.h"
#include "fe/collision/MeshCollider.h"
#include "fe/loaders/RawMesh.h"

namespace fe { namespace loaders {

// Nested types definition
	struct EntityReader::SharedData
	{
		std::vector<std::shared_ptr<RawMesh>> mRawMeshes;
		std::vector<std::shared_ptr<graphics::Mesh>> mMeshes;
		std::vector<std::shared_ptr<graphics::Material>> mMaterials;
		std::vector<std::shared_ptr<graphics::Texture>> mTextures;
	};

// Static variables definition
	const std::string EntityReader::FileFormat::sFileName		= "FAZE_ENT_FILE";
	const std::string EntityReader::FileFormat::sFileExtension	= ".fzent";

// Public Functions
	std::vector<EntityReader::EntityUPtr> EntityReader::load(utils::FileReader& fileReader)
	{
		try {
			// 1. Get the input file
			if (fileReader.getState() != utils::FileState::OK) {
				throw std::runtime_error("Error reading the file\n");
			}

			// 2. Check the file header
			if (!checkHeader(fileReader)) {
				throw std::runtime_error("Error with the header of the file\n");
			}

			// 3. Load the SharedData
			SharedData sharedData = loadSharedData(fileReader);

			// 4. Parse the Entities
			return parseEntities(fileReader, sharedData);
		}
		catch (const std::exception& e) {
			throw std::runtime_error("Error parsing the Entity in the file \"" + fileReader.getFilePath() + "\":\n" + e.what());
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
				ret.mRawMeshes.reserve(rawMeshes.size());
				ret.mRawMeshes.insert(
					ret.mRawMeshes.end(),
					std::make_move_iterator(rawMeshes.begin()),
					std::make_move_iterator(rawMeshes.end())
				);
			}
			else if (token == "material") {
				fileReader >> token;
				utils::FileReader materialFileReader(token);
				auto materials = mMaterialReader.read(materialFileReader);
				ret.mMaterials.reserve(materials.size());
				ret.mMaterials.insert(
					ret.mMaterials.end(),
					std::make_move_iterator(materials.begin()),
					std::make_move_iterator(materials.end())
				);
			}
			else if (token == "}") { end = true; }
			else {
				throw std::runtime_error("Error: unexpected word \"" + token + "\" at line " + std::to_string(fileReader.getNumLines()) + '\n');
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
				throw std::runtime_error("Error: unexpected word \"" + token + "\" at line " + std::to_string(fileReader.getNumLines()) + '\n');
			}
		}

		if (iEntity != nEntities) {
			throw std::runtime_error("Error: expected " + std::to_string(nEntities) + " entities, parsed " + std::to_string(iEntity) + '\n');
		}

		return entities;
	}


	EntityReader::EntityUPtr EntityReader::parseEntity(
		utils::FileReader& fileReader, SharedData& sharedData
	) const
	{
		std::string name;
		glm::vec3 position;
		glm::quat orientation;
		glm::mat4 offsetMatrix;
		std::unique_ptr<graphics::Camera> camera;
		std::unique_ptr<graphics::PointLight> pointLight;
		std::unique_ptr<graphics::Renderable3D> renderable3D;
		std::unique_ptr<physics::PhysicsEntity> physicsEntity;

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

				auto camera = std::make_unique<graphics::Camera>(cameraPosition, cameraTarget, cameraUp);
			}
			else if (token == "renderable3D") {
				std::string meshName, materialName, textureName;
				fileReader >> meshName >> materialName >> textureName >>
					offsetMatrix[0][0] >> offsetMatrix[0][1] >> offsetMatrix[0][2] >> offsetMatrix[0][3] >>
					offsetMatrix[1][0] >> offsetMatrix[1][1] >> offsetMatrix[1][2] >> offsetMatrix[1][3] >>
					offsetMatrix[2][0] >> offsetMatrix[2][1] >> offsetMatrix[2][2] >> offsetMatrix[2][3] >>
					offsetMatrix[3][0] >> offsetMatrix[3][1] >> offsetMatrix[3][2] >> offsetMatrix[3][3];

				auto itMesh = std::find_if(
					sharedData.mMeshes.begin(), sharedData.mMeshes.end(),
					[meshName](std::shared_ptr<graphics::Mesh> mesh) {
						return mesh->getName() == meshName;
					}
				);

				if (itMesh == sharedData.mMeshes.end()) {
					auto itRawMesh = std::find_if(
						sharedData.mRawMeshes.begin(), sharedData.mRawMeshes.end(),
						[meshName](std::shared_ptr<RawMesh> rawMesh) {
							return rawMesh->mName == meshName;
						}
					);

					if (itRawMesh != sharedData.mRawMeshes.end()) {
						sharedData.mMeshes.push_back( mMeshLoader.createMesh(**itRawMesh) );
						itMesh = sharedData.mMeshes.end();
					}
				}

				auto itMaterial = std::find_if(
					sharedData.mMaterials.begin(), sharedData.mMaterials.end(),
					[materialName](std::shared_ptr<graphics::Material> material) {
						return material->getName() == materialName;
					}
				);

				renderable3D = std::make_unique<graphics::Renderable3D>(*itMesh, *itMaterial, nullptr);
			}
			else if (token == "point_light") {
				// TODO
			}
			else if (token == "physics") {
				std::string meshName;
				glm::mat4 mat;
				fileReader >> meshName >>
					mat[0][0] >> mat[0][1] >> mat[0][2] >> mat[0][3] >>
					mat[1][0] >> mat[1][1] >> mat[1][2] >> mat[1][3] >>
					mat[2][0] >> mat[2][1] >> mat[2][2] >> mat[2][3] >>
					mat[3][0] >> mat[3][1] >> mat[3][2] >> mat[3][3];

				auto itRawMesh = std::find_if(
					sharedData.mRawMeshes.begin(), sharedData.mRawMeshes.end(),
					[meshName](std::shared_ptr<RawMesh> rawMesh) {
						return rawMesh->mName == meshName;
					}
				);

				// TODO: read other colliders
				physicsEntity = std::make_unique<physics::PhysicsEntity>(
					physics::RigidBody(),
					std::make_unique<collision::MeshCollider>((*itRawMesh)->mPositions, (*itRawMesh)->mFaceIndices),
					offsetMatrix
				);
			}
			else if (token == "}") { end = true; }
			else {
				throw std::runtime_error("Error: unexpected word \"" + token + "\" at line " + std::to_string(fileReader.getNumLines()) + '\n');
			}
		}

		auto entity = std::make_unique<app::Entity>(name);
		entity->mPosition		= position;
		entity->mOrientation	= orientation;

		if (camera) {
			mGraphicsManager.addEntity(entity.get(), std::move(camera));
		}
		if (pointLight) {
			mGraphicsManager.addEntity(entity.get(), std::move(pointLight));
		}
		if (renderable3D) {
			mGraphicsManager.addEntity(entity.get(), std::move(renderable3D), offsetMatrix);
		}
		if (physicsEntity) {
			mPhysicsManager.addEntity(entity.get(), std::move(physicsEntity));
		}

		return std::move(entity);
	}

}}
