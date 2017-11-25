#include "Game.h"
#include <iostream>
#include <chrono>
#include <thread>

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Entity.h"
#include "InputManager.h"
#include "PhysicsManager.h"
#include "GraphicsManager.h"

#include "../window/WindowSystem.h"

#include "../graphics/Texture.h"
#include "../graphics/GraphicsSystem.h"
#include "../graphics/2D/Layer2D.h"
#include "../graphics/2D/Renderable2D.h"
#include "../graphics/3D/Camera.h"
#include "../graphics/3D/Lights.h"
#include "../graphics/3D/Mesh.h"
#include "../graphics/3D/Material.h"
#include "../graphics/3D/Renderable3D.h"
#include "../graphics/text/Font.h"
#include "../graphics/text/RenderableText.h"

#include "../collision/BoundingBox.h"
#include "../collision/BoundingSphere.h"
#include "../collision/MeshCollider.h"

#include "../physics/PhysicsEngine.h"
#include "../physics/PhysicsEntity.h"
#include "../physics/RigidBody.h"
#include "../physics/forces/Gravity.h"

#include "../loaders/MeshLoader.h"
#include "../loaders/MeshReader.h"
#include "../loaders/MaterialReader.h"
#include "../loaders/ImageReader.h"
#include "../loaders/FontReader.h"
#include "../loaders/TerrainLoader.h"
#include "../loaders/RawMesh.h"

#include "../utils/Image.h"
#include "../utils/Logger.h"
#include "../utils/FileReader.h"

#include <GLFW/glfw3.h>

#define GRAPHICS
#define GAME

namespace game {

// Constants definition
    const float Game::UPDATE_TIME		= 0.016f;
	const unsigned int Game::NUM_CUBES	= 50;

// Public functions
	Game::Game() : mState(GameState::STOPPED)
	{
		using namespace utils;

		// Window
		try {
			mWindowSystem = new window::WindowSystem({"< FAZE Engine >", WIDTH, HEIGHT, false, false });
			std::cout << mWindowSystem->getGLInfo() << std::endl;
		}
		catch(std::exception& e) {
			mState = GameState::ERROR;
			Logger::writeLog(LogType::ERROR, "Error initializing the window system: " + std::string(e.what()));
		}

		// Input
		try { mInputManager = new InputManager(*mWindowSystem); }
		catch (std::exception& e) {
			mState = GameState::ERROR;
			Logger::writeLog(LogType::ERROR, "Error initializing the input manager: " + std::string(e.what()));
		}

		// Physics
		try {
			mPhysicsEngine = new physics::PhysicsEngine();
			mPhysicsManager = new PhysicsManager(*mPhysicsEngine);
		}
		catch (std::exception& e) {
			mState = GameState::ERROR;
			Logger::writeLog(LogType::ERROR, "Error initializing the physics manager: " + std::string(e.what()));
		}

		// Graphics
		try {
			mGraphicsSystem = new graphics::GraphicsSystem();
			mGraphicsManager = new GraphicsManager(*mGraphicsSystem);
		}
		catch (std::exception& e) {
			mState = GameState::ERROR;
			Logger::writeLog(LogType::ERROR, "Error initializing the graphics manager: " + std::string(e.what()));
		}
	}


	Game::~Game()
	{
		delete mGraphicsManager;
		delete mGraphicsSystem;
		delete mPhysicsManager;
		delete mInputManager;
		delete mWindowSystem;
	}


	bool Game::run()
	{
		using namespace utils;

		if (mState != GameState::STOPPED) {
			Logger::writeLog(LogType::ERROR, "Bad game state");
			return false;
		}

#ifdef GAME
#ifdef GRAPHICS
		/*********************************************************************
		 * GRAPHICS DATA
		 *********************************************************************/
		// Loaders
		loaders::MeshLoader meshLoader;
		loaders::MeshReader meshReader;
		loaders::MaterialReader materialReader;
		loaders::ImageReader imageReader;
		loaders::FontReader fontReader(imageReader);
		loaders::TerrainLoader terrainLoader(meshLoader, *mGraphicsManager);

		std::shared_ptr<graphics::Mesh> mesh1 = nullptr, mesh2 = nullptr;
		std::vector<std::shared_ptr<loaders::RawMesh>> fileRawMeshes;
		std::vector<std::shared_ptr<graphics::Mesh>> fileMeshes;
		std::vector<std::shared_ptr<graphics::Material>> fileMaterials;
		std::shared_ptr<utils::Image> heightMap1 = nullptr;
		std::shared_ptr<graphics::Texture> texture1 = nullptr, texture2 = nullptr;
		std::unique_ptr<graphics::Camera> camera1 = nullptr;
		std::unique_ptr<graphics::PointLight> pointLight1 = nullptr, pointLight2 = nullptr;
		std::shared_ptr<graphics::Font> arial = nullptr;
		try {
			// Meshes
			loaders::RawMesh rawMesh1("Cube");
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

			loaders::RawMesh rawMesh2("Plane");
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

			FileReader fileReader1("res/meshes/test.fzmsh");
			auto loadedRawMeshes = meshReader.read(fileReader1);
			for (auto& meshRawUPtr : loadedRawMeshes) {
				fileMeshes.push_back( meshLoader.createMesh(*meshRawUPtr) );
				fileRawMeshes.push_back( std::move(meshRawUPtr) );
			}

			// Materials
			FileReader fileReader2("res/materials/game_materials.fzmat");
			auto loadedMaterials = materialReader.read(fileReader2);
			for (std::unique_ptr<graphics::Material>& materialUPtr : loadedMaterials) {
				fileMaterials.push_back(std::move(materialUPtr));
			}

			// Images
			std::unique_ptr<utils::Image> image1( imageReader.read("res/images/test.png", utils::ImageFormat::RGBA_IMAGE) );
			heightMap1 = std::unique_ptr<utils::Image>( imageReader.read("res/images/terrain.png", utils::ImageFormat::L_IMAGE) );

			// Textures
			texture1 = std::make_shared<graphics::Texture>();
			texture1->setImage(
				image1->getPixels(), graphics::TexturePixelType::U_BYTE, graphics::TextureFormat::RGBA,
				image1->getWidth(), image1->getHeight()
			);

			float pixels[] = {
			    0.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f,
			    1.0f, 1.0f, 1.0f,   0.0f, 0.0f, 0.0f
			};
			texture2 = std::make_shared<graphics::Texture>();
			texture2->setImage(pixels, graphics::TexturePixelType::FLOAT, graphics::TextureFormat::RGB, 2, 2);

			// Cameras
			camera1 = std::make_unique<graphics::Camera>(glm::vec3(0,0,0), glm::vec3(0,0,-1), glm::vec3(0,1,0));

			// Lights
			graphics::BaseLight baseLight1(graphics::RGBColor(0.5f, 0.6f, 0.3f), graphics::RGBColor(0.1f, 0.5f, 0.6f));
			graphics::Attenuation attenuation1{ 0.25f, 0.2f, 0.1f };
			pointLight1 = std::make_unique<graphics::PointLight>(baseLight1, attenuation1, glm::vec3());
			pointLight2 = std::make_unique<graphics::PointLight>(baseLight1, attenuation1, glm::vec3());

			FileReader fileReader3("res/fonts/arial.fnt");
			arial = std::move(fontReader.read(fileReader3));
		}
		catch (std::exception& e) {
			Logger::writeLog(LogType::ERROR, e.what());
			return false;
		}

		// RenderableTexts
		graphics::RenderableText renderableText1("First try rendering text", arial, 10, glm::vec2());

		// Renderable2Ds
		graphics::Layer2D layer2D;
		mGraphicsSystem->addLayer(&layer2D);

		graphics::Renderable2D renderable2D1(glm::vec2(0.75f, 0.75f), glm::vec2(0.15f, 0.2f), texture1);
		layer2D.addRenderable2D(&renderable2D1);
#endif		// GRAPHICS

		/*********************************************************************
		 * GAME DATA
		 *********************************************************************/
		// Player
		auto player	= std::make_unique<Entity>("player");
		player->mPosition = glm::vec3(0, 1, 10);
       	
		auto physicsEntity1 = std::make_unique<physics::PhysicsEntity>(
			physics::RigidBody(
				40.0f, 0.01f,
				2.0f / 5.0f * 10.0f * glm::pow(2.0f, 2.0f) * glm::mat3(), 0.01f
			),
			//std::make_unique<collision::BoundingSphere>(0.5f), glm::mat4()
			std::make_unique<collision::BoundingBox>(glm::vec3(1, 1, 1)), glm::mat4()
		);

		mInputManager->addEntity(player.get());
		mPhysicsManager->addEntity(player.get(), std::move(physicsEntity1));
		mGraphicsManager->addEntity(player.get(), std::move(camera1));

		mEntities.push_back(std::move(player));

		// Terrain
		mEntities.push_back( terrainLoader.createTerrain("terrain", 100.0f, *heightMap1, 10.0f) );

		// Plane
		auto plane = std::make_unique<Entity>("plane");
		plane->mPosition = glm::vec3(-5.0f, 1.0f, -5.0f);

		auto renderable3D1 = std::make_unique<graphics::Renderable3D>(mesh2, fileMaterials[4], texture2);
		mGraphicsManager->addEntity(plane.get(), std::move(renderable3D1), glm::mat4());

		mEntities.push_back(std::move(plane));

		// Fixed cubes
		glm::vec3 cubePositions[5] = { glm::vec3(2, 5, -10), glm::vec3(0, 7, -10), glm::vec3(0, 5, -8), glm::vec3(0, 5, -10), glm::vec3(10, 5, -10) };
		for (size_t i = 0; i < 5; ++i) {
			auto cube = std::make_unique<Entity>("non-random-cube");
			cube->mPosition = cubePositions[i];
			
			auto physicsEntityCube2 = std::make_unique<physics::PhysicsEntity>(
				physics::RigidBody(
					20.0f, 0.95f,
					2.0f / 5.0f * 10.0f * glm::pow(2.0f, 2.0f) * glm::mat3(), 0.95f
				),
				std::make_unique<collision::BoundingBox>(glm::vec3(1,1,1)), glm::mat4()
			);
			if (i == 3) { physicsEntityCube2->getRigidBody()->mAngularVelocity += glm::vec3(0, 10, 0); }
			if (i == 4) { cube->mVelocity += glm::vec3(-1, 0, 0); }

			mPhysicsManager->addEntity(cube.get(), std::move(physicsEntityCube2));
			
			auto renderable3D2 = std::make_unique<graphics::Renderable3D>(mesh1, fileMaterials[i], nullptr);
			mGraphicsManager->addEntity(cube.get(), std::move(renderable3D2), glm::mat4());

			mEntities.push_back(std::move(cube));
        }

		// Random cubes
		for (size_t i = 0; i < NUM_CUBES; ++i) {
			auto cube = std::make_unique<Entity>("random-cube");
			cube->mPosition = glm::ballRand(50.0f);
			
			auto physicsEntityCube2 = std::make_unique<physics::PhysicsEntity>(
				physics::RigidBody(
					10.0f, 0.9f,
					2.0f / 5.0f * 10.0f * glm::pow(2.0f, 2.0f) * glm::mat3(), 0.9f
				),
				std::make_unique<collision::BoundingBox>(glm::vec3(1,1,1)), glm::mat4()
			);
			mPhysicsManager->addEntity(cube.get(), std::move(physicsEntityCube2));
			
			auto renderable3D2 = std::make_unique<graphics::Renderable3D>(mesh1, fileMaterials[4], nullptr);
			mGraphicsManager->addEntity(cube.get(), std::move(renderable3D2), glm::mat4());

			mEntities.push_back(std::move(cube));
		}

		// Buildings
		for (auto it = fileMeshes.begin(); it != fileMeshes.end(); ++it) {
			auto building = std::make_unique<Entity>("building");
			building->mOrientation = glm::normalize(glm::quat(-1, glm::vec3(1, 0, 0)));

			if (it == fileMeshes.begin()) {
				auto rawMesh = *fileRawMeshes.begin();
				
				auto physicsEntityMesh = std::make_unique<physics::PhysicsEntity>(
					physics::RigidBody(),
					std::make_unique<collision::MeshCollider>(rawMesh->mPositions, rawMesh->mFaceIndices),
					glm::mat4()
				);
				mPhysicsManager->addEntity(building.get(), std::move(physicsEntityMesh));
			}
			
			auto tmpMaterial = std::make_shared<graphics::Material>(
				"tmp_material",
				graphics::RGBColor( glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f) ),
				graphics::RGBColor( glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f) ),
				graphics::RGBColor( glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f) ),
				0.2f
			);
			
			auto renderable3D2 = std::make_unique<graphics::Renderable3D>(*it, tmpMaterial, nullptr);
			mGraphicsManager->addEntity(building.get(), std::move(renderable3D2), glm::mat4());
			
			mEntities.push_back(std::move(building));
		}

		// Lights
		auto eL1 = std::make_unique<Entity>("point-light1");
		eL1->mPosition = glm::vec3(2, 1, 5);
		mGraphicsManager->addEntity(eL1.get(), std::move(pointLight1));
		mEntities.push_back(std::move(eL1));

		auto eL2 = std::make_unique<Entity>("point-light2");
		eL2->mPosition = glm::vec3(-3, 1, 5);
		mGraphicsManager->addEntity(eL2.get(), std::move(pointLight2));
		mEntities.push_back(std::move(eL2));
#endif		// GAME

		/*********************************************************************
		 * MAIN LOOP
		 *********************************************************************/
		mState = GameState::RUNNING;
		float lastTime = mWindowSystem->getTime();
		while (mState == GameState::RUNNING) {
			// Calculate the elapsed time since the last update
			float curTime	= mWindowSystem->getTime();
			float deltaTime	= curTime - lastTime;

			if (deltaTime >= UPDATE_TIME) {
				lastTime = curTime;
				std::cout << deltaTime << "ms\r";

				// Update the Systems
				mWindowSystem->update();
				if (mWindowSystem->getInputData()->mKeys[GLFW_KEY_ESCAPE]) {
					mState = GameState::STOPPED;
				}
				mInputManager->update();
				mPhysicsManager->update(deltaTime);
				mGraphicsManager->update();

				// Draw
				mGraphicsManager->render();
				mWindowSystem->swapBuffers();
			}
			else {
				std::this_thread::sleep_for( std::chrono::duration<float>(UPDATE_TIME - deltaTime) );
			}
		}

		// Delete the entities
		for (EntityUPtr& entity : mEntities) {
			mInputManager->removeEntity(entity.get());
			mPhysicsManager->removeEntity(entity.get());
			mGraphicsManager->removeEntity(entity.get());
		}
		mEntities.clear();

		return true;
	}

}
