#include "Game.h"

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

#include <fe/app/Entity.h>
#include <fe/app/InputManager.h>
#include <fe/app/PhysicsManager.h>
#include <fe/app/GraphicsManager.h>

#include <fe/graphics/Texture.h>
#include <fe/graphics/GraphicsSystem.h>
#include <fe/graphics/3D/Camera.h>
#include <fe/graphics/3D/Lights.h>
#include <fe/graphics/3D/Mesh.h>
#include <fe/graphics/3D/Material.h>
#include <fe/graphics/3D/Renderable3D.h>
#include <fe/graphics/text/Font.h>
#include <fe/graphics/text/RenderableText.h>

#include <fe/collision/BoundingBox.h>
#include <fe/collision/BoundingSphere.h>
#include <fe/collision/MeshCollider.h>

#include <fe/physics/PhysicsEngine.h>
#include <fe/physics/PhysicsEntity.h>
#include <fe/physics/RigidBody.h>
#include <fe/physics/forces/Gravity.h>

#include <fe/loaders/MeshLoader.h>
#include <fe/loaders/MeshReader.h>
#include <fe/loaders/MaterialReader.h>
#include <fe/loaders/ImageReader.h>
#include <fe/loaders/FontReader.h>
#include <fe/loaders/TerrainLoader.h>
#include <fe/loaders/RawMesh.h>

#include <fe/utils/Image.h>
#include <fe/utils/Logger.h>
#include <fe/utils/FileReader.h>

namespace game {

// static variables definition
	const std::string Game::sTitle		= "< FAZE >";
	const unsigned int Game::sWidth		= 1280;
	const unsigned int Game::sHeight	= 720;
	const float Game::sUpdateTime		= 0.016f;
	const unsigned int Game::sNumCubes	= 50;

// Public functions
	Game::Game() : fe::app::Application(sTitle, sWidth, sHeight, sUpdateTime)
	{
		mGraphicsSystem->addLayer(&mLayer2D);
	}


	Game::~Game()
	{
		mGraphicsSystem->removeLayer(&mLayer2D);
	}


	void Game::init()
	{
		/*********************************************************************
		 * GRAPHICS DATA
		 *********************************************************************/
		fe::loaders::MeshLoader meshLoader;
		fe::loaders::TerrainLoader terrainLoader(meshLoader, *mGraphicsManager);

		std::shared_ptr<fe::graphics::Mesh> mesh1 = nullptr, mesh2 = nullptr;
		std::vector<std::shared_ptr<fe::loaders::RawMesh>> fileRawMeshes;
		std::vector<std::shared_ptr<fe::graphics::Mesh>> fileMeshes;
		std::vector<std::shared_ptr<fe::graphics::Material>> fileMaterials;
		std::shared_ptr<fe::utils::Image> heightMap1 = nullptr;
		std::shared_ptr<fe::graphics::Texture> texture1 = nullptr, texture2 = nullptr;
		std::unique_ptr<fe::graphics::Camera> camera1 = nullptr;
		std::unique_ptr<fe::graphics::PointLight> pointLight1 = nullptr, pointLight2 = nullptr;
		std::shared_ptr<fe::graphics::Font> arial = nullptr;

		try {
			// Readers
			fe::loaders::MeshReader meshReader;
			fe::loaders::MaterialReader materialReader;
			fe::loaders::ImageReader imageReader;
			fe::loaders::FontReader fontReader(imageReader);

			// Meshes
			fe::loaders::RawMesh rawMesh1("Cube");
			rawMesh1.mPositions = {
				{-0.5f,	-0.5f,	-0.5f},
				{-0.5f,	-0.5f,	 0.5f},
				{-0.5f,	 0.5f,	-0.5f},
				{-0.5f,	 0.5f,	 0.5f},
				{ 0.5f,	-0.5f,	-0.5f},
				{ 0.5f,	-0.5f,	 0.5f},
				{ 0.5f,	 0.5f,	-0.5f},
				{ 0.5f,	 0.5f,	 0.5f}
			};
			rawMesh1.mFaceIndices = {
				0, 1, 2,
				1, 3, 2,
				0, 2, 4,
				2, 6, 4,
				4, 6, 5,
				5, 6, 7,
				1, 5, 3,
				3, 5, 7,
				0, 4, 1,
				1, 4, 5,
				2, 3, 6,
				3, 7, 6
			};
			rawMesh1.mNormals = meshReader.calculateNormals(rawMesh1.mPositions, rawMesh1.mFaceIndices);
			rawMesh1.mUVs = std::vector<glm::vec2>(8);
			mesh1 = std::move(meshLoader.createMesh(rawMesh1));

			fe::loaders::RawMesh rawMesh2("Plane");
			rawMesh2.mPositions = {
				{-0.5f,	-0.5f,	0.0f},
				{ 0.5f,	-0.5f,	0.0f},
				{-0.5f,	0.5f,	0.0f},
				{ 0.5f,	0.5f,	0.0f}
			};
			rawMesh2.mNormals = {
				{0.0f,	0.0f,	1.0f},
				{0.0f,	0.0f,	1.0f},
				{0.0f,	0.0f,	1.0f},
				{0.0f,	0.0f,	1.0f}
			};
			rawMesh2.mUVs = {
				{0.0f,	0.0f},
				{1.0f,	0.0f},
				{0.0f,	1.0f},
				{1.0f,	1.0f}
			};
			rawMesh2.mFaceIndices = {
				0, 1, 2,
				1, 3, 2,
			};
			mesh2 = std::move(meshLoader.createMesh(rawMesh2));

			fe::utils::FileReader fileReader1("res/meshes/test.fzmsh");
			auto loadedRawMeshes = meshReader.read(fileReader1);
			for (auto& meshRawUPtr : loadedRawMeshes) {
				fileMeshes.push_back( meshLoader.createMesh(*meshRawUPtr) );
				fileRawMeshes.push_back( std::move(meshRawUPtr) );
			}

			// Materials
			fe::utils::FileReader fileReader2("res/materials/game_materials.fzmat");
			auto loadedMaterials = materialReader.read(fileReader2);
			for (std::unique_ptr<fe::graphics::Material>& materialUPtr : loadedMaterials) {
				fileMaterials.push_back(std::move(materialUPtr));
			}

			// Images
			std::unique_ptr<fe::utils::Image> image1( imageReader.read("res/images/test.png", fe::utils::ImageFormat::RGBA_IMAGE) );
			heightMap1 = std::unique_ptr<fe::utils::Image>( imageReader.read("res/images/terrain.png", fe::utils::ImageFormat::L_IMAGE) );

			// Textures
			texture1 = std::make_shared<fe::graphics::Texture>();
			texture1->setImage(
				image1->getPixels(), fe::graphics::TexturePixelType::U_BYTE, fe::graphics::TextureFormat::RGBA,
				image1->getWidth(), image1->getHeight()
			);

			float pixels[] = {
				0.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f,
				1.0f, 1.0f, 1.0f,   0.0f, 0.0f, 0.0f
			};
			texture2 = std::make_shared<fe::graphics::Texture>();
			texture2->setImage(pixels, fe::graphics::TexturePixelType::FLOAT, fe::graphics::TextureFormat::RGB, 2, 2);

			// Cameras
			camera1 = std::make_unique<fe::graphics::Camera>(glm::vec3(0,0,0), glm::vec3(0,0,-1), glm::vec3(0,1,0));

			// Lights
			fe::graphics::BaseLight baseLight1(fe::graphics::RGBColor(0.5f, 0.6f, 0.3f), fe::graphics::RGBColor(0.1f, 0.5f, 0.6f));
			fe::graphics::Attenuation attenuation1{ 0.25f, 0.2f, 0.1f };
			pointLight1 = std::make_unique<fe::graphics::PointLight>(baseLight1, attenuation1, glm::vec3());
			pointLight2 = std::make_unique<fe::graphics::PointLight>(baseLight1, attenuation1, glm::vec3());

			fe::utils::FileReader fileReader3("res/fonts/arial.fnt");
			arial = std::move(fontReader.read(fileReader3));
		}
		catch (std::exception& e) {
			fe::utils::Logger::writeLog(fe::utils::LogType::ERROR, e.what());
			mState = AppState::ERROR;
		}

		// RenderableTexts
		fe::graphics::RenderableText renderableText1("First try rendering text", arial, 10, glm::vec2());

		// Renderable2Ds
		mRenderable2Ds.emplace_back(glm::vec2(0.75f, 0.75f), glm::vec2(0.15f, 0.2f), texture1);
		mLayer2D.addRenderable2D(&mRenderable2Ds.back());

		/*********************************************************************
		 * GAME DATA
		 *********************************************************************/
		// Player
		auto player	= std::make_unique<fe::app::Entity>("player");
		player->mPosition = glm::vec3(0, 1, 10);

		auto physicsEntity1 = std::make_unique<fe::physics::PhysicsEntity>(
			fe::physics::RigidBody(
				40.0f, 0.01f,
				2.0f / 5.0f * 10.0f * glm::pow(2.0f, 2.0f) * glm::mat3(), 0.01f
			),
			std::make_unique<fe::collision::BoundingSphere>(0.5f), glm::mat4(1.0f)
			//std::make_unique<fe::collision::BoundingBox>(glm::vec3(1, 1, 1)), glm::mat4(1.0f)
		);

		mInputManager->addEntity(player.get());
		mPhysicsManager->addEntity(player.get(), std::move(physicsEntity1));
		mGraphicsManager->addEntity(player.get(), std::move(camera1));

		mEntities.push_back(std::move(player));

		// Terrain
		mEntities.push_back( terrainLoader.createTerrain("terrain", 100.0f, *heightMap1, 10.0f) );

		// Plane
		auto plane = std::make_unique<fe::app::Entity>("plane");
		plane->mPosition = glm::vec3(-5.0f, 1.0f, -5.0f);

		auto renderable3D1 = std::make_unique<fe::graphics::Renderable3D>(mesh2, fileMaterials[4], texture2);
		mGraphicsManager->addEntity(plane.get(), std::move(renderable3D1), glm::mat4(1.0f));

		mEntities.push_back(std::move(plane));

		// Fixed cubes
		glm::vec3 cubePositions[5] = { glm::vec3(2, 5, -10), glm::vec3(0, 7, -10), glm::vec3(0, 5, -8), glm::vec3(0, 5, -10), glm::vec3(10, 5, -10) };
		for (size_t i = 0; i < 5; ++i) {
			auto cube = std::make_unique<fe::app::Entity>("non-random-cube");
			cube->mPosition = cubePositions[i];

			auto physicsEntityCube2 = std::make_unique<fe::physics::PhysicsEntity>(
				fe::physics::RigidBody(
					20.0f, 0.95f,
					2.0f / 5.0f * 10.0f * glm::pow(2.0f, 2.0f) * glm::mat3(), 0.95f
				),
				std::make_unique<fe::collision::BoundingBox>(glm::vec3(1,1,1)), glm::mat4(1.0f)
			);
			if (i == 3) { physicsEntityCube2->getRigidBody()->mAngularVelocity += glm::vec3(0, 10, 0); }
			if (i == 4) { cube->mVelocity += glm::vec3(-1, 0, 0); }

			mPhysicsManager->addEntity(cube.get(), std::move(physicsEntityCube2));

			auto renderable3D2 = std::make_unique<fe::graphics::Renderable3D>(mesh1, fileMaterials[i], nullptr);
			mGraphicsManager->addEntity(cube.get(), std::move(renderable3D2), glm::mat4(1.0f));

			mEntities.push_back(std::move(cube));
		}

		// Random cubes
		for (size_t i = 0; i < sNumCubes; ++i) {
			auto cube = std::make_unique<fe::app::Entity>("random-cube");
			cube->mPosition = glm::ballRand(50.0f);

			auto physicsEntityCube2 = std::make_unique<fe::physics::PhysicsEntity>(
				fe::physics::RigidBody(
					10.0f, 0.9f,
					2.0f / 5.0f * 10.0f * glm::pow(2.0f, 2.0f) * glm::mat3(), 0.9f
				),
				std::make_unique<fe::collision::BoundingBox>(glm::vec3(1,1,1)), glm::mat4(1.0f)
			);
			mPhysicsManager->addEntity(cube.get(), std::move(physicsEntityCube2));

			auto renderable3D2 = std::make_unique<fe::graphics::Renderable3D>(mesh1, fileMaterials[4], nullptr);
			mGraphicsManager->addEntity(cube.get(), std::move(renderable3D2), glm::mat4(1.0f));

			mEntities.push_back(std::move(cube));
		}

		// Buildings
		for (auto it = fileMeshes.begin(); it != fileMeshes.end(); ++it) {
			auto building = std::make_unique<fe::app::Entity>("building");
			building->mOrientation = glm::normalize(glm::quat(-1, glm::vec3(1, 0, 0)));

			if (it == fileMeshes.begin()) {
				auto rawMesh = *fileRawMeshes.begin();

				auto physicsEntityMesh = std::make_unique<fe::physics::PhysicsEntity>(
					fe::physics::RigidBody(),
					std::make_unique<fe::collision::MeshCollider>(rawMesh->mPositions, rawMesh->mFaceIndices),
					glm::mat4(1.0f)
				);
				mPhysicsManager->addEntity(building.get(), std::move(physicsEntityMesh));
			}

			auto tmpMaterial = std::make_shared<fe::graphics::Material>(
				"tmp_material",
				fe::graphics::RGBColor( glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f) ),
				fe::graphics::RGBColor( glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f) ),
				fe::graphics::RGBColor( glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f) ),
				0.2f
			);

			auto renderable3D2 = std::make_unique<fe::graphics::Renderable3D>(*it, tmpMaterial, nullptr);
			mGraphicsManager->addEntity(building.get(), std::move(renderable3D2), glm::mat4(1.0f));

			mEntities.push_back(std::move(building));
		}

		// Lights
		auto eL1 = std::make_unique<fe::app::Entity>("point-light1");
		eL1->mPosition = glm::vec3(2, 1, 5);
		mGraphicsManager->addEntity(eL1.get(), std::move(pointLight1));
		mEntities.push_back(std::move(eL1));

		auto eL2 = std::make_unique<fe::app::Entity>("point-light2");
		eL2->mPosition = glm::vec3(-3, 1, 5);
		mGraphicsManager->addEntity(eL2.get(), std::move(pointLight2));
		mEntities.push_back(std::move(eL2));
	}


	void Game::end()
	{
		for (EntityUPtr& entity : mEntities) {
			mInputManager->removeEntity(entity.get());
			mPhysicsManager->removeEntity(entity.get());
			mGraphicsManager->removeEntity(entity.get());
		}
		mEntities.clear();
	}

}
