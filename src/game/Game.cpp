#include "Game.h"
#include <iostream>
#include <chrono>
#include <thread>

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
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

#include "../physics/PhysicsEntity.h"
#include "../physics/RigidBody.h"

#include "../loaders/MeshLoader.h"
#include "../loaders/MeshReader.h"
#include "../loaders/MaterialReader.h"
#include "../loaders/ImageReader.h"
#include "../loaders/FontReader.h"
#include "../loaders/TerrainLoader.h"

#include "../utils/Image.h"
#include "../utils/Logger.h"
#include "../utils/FileReader.h"

#include <GLFW/glfw3.h>

#define GRAPHICS
#define GAME

namespace game {

// Static variables definition
    const float Game::UPDATE_TIME		= 0.016f;
	const unsigned int Game::NUM_CUBES	= 50;

// Public functions
	Game::Game() : mState(GameState::STOPPED)
	{
		using namespace utils;

		// Window
		try {
			mWindowSystem = new window::WindowSystem("< FAZE >", { WIDTH, HEIGHT, false, false });
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
		try { mPhysicsManager = new PhysicsManager(); }
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
		loaders::MeshReader meshReader(meshLoader);
		loaders::MaterialReader materialReader;
		loaders::ImageReader imageReader;
		loaders::FontReader fontReader(imageReader);
		loaders::TerrainLoader terrainLoader(meshLoader, *mGraphicsManager);

		std::shared_ptr<graphics::Mesh> mesh1 = nullptr, mesh2 = nullptr;
		std::vector<std::shared_ptr<graphics::Mesh>> fileMeshes;
		std::vector<std::shared_ptr<graphics::Material>> fileMaterials;
		std::shared_ptr<utils::Image> heightMap1 = nullptr;
		std::shared_ptr<graphics::Texture> texture1 = nullptr, texture2 = nullptr;
		std::unique_ptr<graphics::Camera> camera1 = nullptr;
		std::unique_ptr<graphics::PointLight> pointLight1 = nullptr, pointLight2 = nullptr;
		std::shared_ptr<graphics::Font> arial = nullptr;
		try {
			// Meshes
			std::vector<GLfloat> positions1 = {
				-0.5f,	-0.5f,	-0.5f,
				-0.5f,	-0.5f,	0.5f,
				-0.5f,	0.5f,	-0.5f,
				-0.5f,	0.5f,	0.5f,
				0.5f,	-0.5f,	-0.5f,
				0.5f,	-0.5f,	0.5f,
				0.5f,	0.5f,	-0.5f,
				0.5f,	0.5f,	0.5f
			};
			std::vector<GLushort> indices1 = {
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
			std::vector<GLfloat> normals1 = meshReader.calculateNormals(positions1, indices1);
			std::vector<GLfloat> uvs1(16);
			mesh1 = std::move(meshLoader.createMesh("Cubo", positions1, normals1, uvs1, indices1));

			std::vector<GLfloat> positions2 = {
				-0.5f,	-0.5f,	0.0f,
				 0.5f,	-0.5f,	0.0f,
				-0.5f,	0.5f,	0.0f,
				 0.5f,	0.5f,	0.0f
			};
			std::vector<GLfloat> normals2 = {
				0.0f,	0.0f,	1.0f,
				0.0f,	0.0f,	1.0f,
				0.0f,	0.0f,	1.0f,
				0.0f,	0.0f,	1.0f
			};
			std::vector<GLfloat> uvs2 = {
				0.0f,	0.0f,
				1.0f,	0.0f,
				0.0f,	1.0f,
				1.0f,	1.0f
			};
			std::vector<GLushort> indices2 = {
				0, 1, 2,
				1, 3, 2,
			};
			mesh2 = std::move(meshLoader.createMesh("plane", positions2, normals2, uvs2, indices2));

			FileReader fileReader1("res/meshes/test.fzmsh");
			auto loadedMeshes = std::move( meshReader.load(fileReader1) );
			for (std::unique_ptr<graphics::Mesh>& meshUPtr : loadedMeshes) {
				fileMeshes.push_back(std::move(meshUPtr));
			}

			// Materials
			FileReader fileReader2("res/materials/game_materials.fzmat");
			auto loadedMaterials = materialReader.load(fileReader2);
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
			graphics::Attenuation attenuation1{ 0.5f, 0.25f, 0.2f };
			pointLight1 = std::make_unique<graphics::PointLight>(baseLight1, attenuation1, glm::vec3());
			pointLight2 = std::make_unique<graphics::PointLight>(baseLight1, attenuation1, glm::vec3());

			FileReader fileReader3("res/fonts/arial.fnt");
			arial = std::move(fontReader.load(fileReader3));
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
       	
		mInputManager->addEntity(player.get());
		mPhysicsManager->addEntity(
			player.get(),
			std::make_unique<physics::PhysicsEntity>(
       			physics::RigidBody(
       				40.0f, 0.01f,
       				2.0f / 5.0f * 10.0f * glm::pow(2.0f,2.0f) * glm::mat3(), 0.01f
       			),
				//std::make_unique<collision::BoundingBox>(glm::vec3(1,1,1)), glm::mat4()
				std::make_unique<collision::BoundingSphere>(0.5f), glm::mat4()
       		)
		);
		mGraphicsManager->addEntity(player.get(), std::move(camera1));

		mEntities.push_back(std::move(player));

		// Terrain
		mEntities.push_back( terrainLoader.createTerrain("terrain", 100.0f, *heightMap1, 10.0f) );

		// Plane
		auto plane = std::make_unique<Entity>("Plane");
		plane->mPosition = glm::vec3(-5.0f, 1.0f, -5.0f);

		auto renderable3D1 = std::make_unique<graphics::Renderable3D>(mesh2, fileMaterials[4], texture2);
		mGraphicsManager->addEntity(plane.get(), std::move(renderable3D1));

		mEntities.push_back(std::move(plane));
 
		// Cubes
		glm::vec3 cubePositions[5] = { glm::vec3(2, 0, -10), glm::vec3(0, 2, -10), glm::vec3(0, 0, -8), glm::vec3(0, 0, -10), glm::vec3(10, 0, -10) };
		for (size_t i = 0; i < 5; ++i) {
			auto cube = std::make_unique<Entity>("random cube");
			cube->mPosition = cubePositions[i];
			
			auto physicsEntityCube = std::make_unique<physics::PhysicsEntity>(
				physics::RigidBody(
					20.0f, 1.0f,
					2.0f / 5.0f * 10.0f * glm::pow(2.0f, 2.0f) * glm::mat3(), 0.25f
				),
				//std::make_unique<collision::BoundingBox>(glm::vec3(1,1,1)), glm::mat4()
				std::make_unique<collision::BoundingSphere>(0.5f), glm::mat4()
			);
			if (i == 3) { physicsEntityCube->getRigidBody()->mAngularVelocity += glm::vec3(0, 10, 0); }
			if (i == 4) { physicsEntityCube->getRigidBody()->mLinearVelocity += glm::vec3(-1, 0, 0); }
			mPhysicsManager->addEntity(cube.get(), std::move(physicsEntityCube));
			
			auto renderable3D2 = std::make_unique<graphics::Renderable3D>(mesh1, fileMaterials[i], nullptr);
			mGraphicsManager->addEntity(cube.get(), std::move(renderable3D2));

			mEntities.push_back(std::move(cube));
        }
		
		for (size_t i = 0; i < NUM_CUBES; ++i) {
			auto cube = std::make_unique<Entity>("random cube");
			cube->mPosition = glm::ballRand(50.0f);
			
			mPhysicsManager->addEntity(
				cube.get(),
				std::make_unique<physics::PhysicsEntity>(
					physics::RigidBody(
						10.0f, 1.0f,
						2.0f / 5.0f * 10.0f * glm::pow(2.0f, 2.0f) * glm::mat3(), 0.25f
					),
					//std::make_unique<collision::BoundingBox>(glm::vec3(1,1,1)), glm::mat4()
					std::make_unique<collision::BoundingSphere>(0.5f), glm::mat4()
				)
			);
			
			auto renderable3D2 = std::make_unique<graphics::Renderable3D>(mesh1, fileMaterials[4], nullptr);
			mGraphicsManager->addEntity(cube.get(), std::move(renderable3D2));

			mEntities.push_back(std::move(cube));
		}

		// Buildings
		for (auto it = fileMeshes.begin(); it != fileMeshes.end(); ++it) {
			auto building = std::make_unique<Entity>("building");
			building->mOrientation = glm::normalize(glm::quat(-1, glm::vec3(1, 0, 0)));

			auto tmpMaterial = std::make_shared<graphics::Material>(
				"tmp_material",
				graphics::RGBColor( glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f) ),
				graphics::RGBColor( glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f) ),
				graphics::RGBColor( glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f) ),
				0.2f
			);
			
			auto renderable3D2 = std::make_unique<graphics::Renderable3D>(*it, tmpMaterial, nullptr);
			mGraphicsManager->addEntity(building.get(), std::move(renderable3D2));
			
			mEntities.push_back(std::move(building));
		}

		// Lights
		auto eL1 = std::make_unique<Entity>("PointLight1");
		eL1->mPosition = glm::vec3(2, 1, 5);
		mGraphicsManager->addEntity(eL1.get(), std::move(pointLight1));
		mEntities.push_back(std::move(eL1));

		auto eL2 = std::make_unique<Entity>("PointLight2");
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
				mInputManager->update(deltaTime);
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
