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
#include "../graphics/2D/Renderable2D.h"
#include "../graphics/text/Font.h"
#include "../graphics/text/RenderableText.h"
#include "../graphics/3D/Mesh.h"
#include "../graphics/3D/Material.h"
#include "../graphics/3D/Camera.h"
#include "../graphics/3D/Lights.h"
#include "../graphics/3D/Renderable3D.h"

#include "../physics/PhysicsEntity.h"
#include "../physics/RigidBody.h"
#include "../physics/collision/BoundingBox.h"
#include "../physics/collision/BoundingSphere.h"

#include "../loaders/MeshLoader.h"
#include "../loaders/MeshReader.h"
#include "../loaders/FontReader.h"
#include "../loaders/MaterialReader.h"

#include "../utils/Logger.h"
#include "../utils/FileReader.h"

#define GRAPHICS
#define GAME

namespace game {

// Static variables definition
    const float Game::UPDATE_TIME		= 0.016f;
	const unsigned int Game::NUM_CUBES	= 50;

// Public functions
	Game::Game() : mEnd(false)
	{
		using namespace utils;

		// Window
		try {
			mWindowSystem = new window::WindowSystem("< FAZE >", WIDTH, HEIGHT);
			std::cout << mWindowSystem->getGLInfo() << std::endl;
		}
		catch(std::exception& e) {
			Logger::writeLog(LogType::ERROR, "Error initializing the window system: " + std::string(e.what()));
			return;
		}

		// Input
		try { mInputManager = new InputManager(*mWindowSystem); }
		catch (std::exception& e) {
			Logger::writeLog(LogType::ERROR, "Error initializing the input manager: " + std::string(e.what()));
			return;
		}

		// Physics
		try { mPhysicsManager = new PhysicsManager(); }
		catch (std::exception& e) {
			Logger::writeLog(LogType::ERROR, "Error initializing the physics manager: " + std::string(e.what()));
			return;
		}

		// Graphics
		try { mGraphicsManager = new GraphicsManager(); }
		catch (std::exception& e) {
			Logger::writeLog(LogType::ERROR, "Error initializing the graphics manager: " + std::string(e.what()));
			return;
		}
	}


	Game::~Game()
	{
		delete mGraphicsManager;
		delete mPhysicsManager;
		delete mInputManager;
		delete mWindowSystem;
	}


	bool Game::run()
	{
		using namespace utils;

#ifdef GAME
#ifdef GRAPHICS
		/*********************************************************************
		 * GRAPHICS DATA
		 *********************************************************************/
		// Loaders
		loaders::MeshLoader meshLoader;
		loaders::MeshReader meshReader(meshLoader);
		loaders::MaterialReader materialReader;
		loaders::FontReader fontReader;

		// Meshes
		std::vector<GLfloat> positions = {
			-0.5f,	-0.5f,	-0.5f,
			-0.5f,	-0.5f,	0.5f,
			-0.5f,	0.5f,	-0.5f,
			-0.5f,	0.5f,	0.5f,
			0.5f,	-0.5f,	-0.5f,
			0.5f,	-0.5f,	0.5f,
			0.5f,	0.5f,	-0.5f,
			0.5f,	0.5f,	0.5f
		};
		std::vector<GLushort> indices = {
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
		std::vector<GLfloat> normals = meshReader.calculateNormals(positions, indices);
		std::shared_ptr<graphics::Mesh> mesh1 = std::move(meshLoader.createMesh("Cubo", positions, normals, std::vector<GLfloat>(16), indices));

		std::vector<std::shared_ptr<graphics::Mesh>> fileMeshes;
		try {
			FileReader fileReader1("res/meshes/test.fzmsh");
			auto loadedMeshes = meshReader.load(fileReader1);
			for (std::unique_ptr<graphics::Mesh>& meshUPtr : loadedMeshes) {
				fileMeshes.push_back(std::move(meshUPtr));
			}
		}
		catch (std::exception& e) {
			Logger::writeLog(LogType::ERROR, e.what());
		}

		// Materials
		std::vector<std::shared_ptr<graphics::Material>> fileMaterials;
		try {
			FileReader fileReader1("res/materials/game_materials.fzmat");
			auto loadedMaterials = materialReader.load(fileReader1);
			for (std::unique_ptr<graphics::Material>& materialUPtr : loadedMaterials) {
				fileMaterials.push_back(std::move(materialUPtr));
			}
		}
		catch (std::exception& e) {
			Logger::writeLog(LogType::ERROR, e.what());
		}

		// Textures
		auto texture1 = std::make_shared<graphics::Texture>("res/images/test.png", GL_TEXTURE_2D);

		// Cameras
		auto camera1 = std::make_unique<graphics::Camera>(glm::vec3(0,0,0), glm::vec3(0,0,-1), glm::vec3(0,1,0));

		// Lights
		graphics::BaseLight baseLight1(graphics::RGBColor(0.5f, 0.6f, 0.3f), graphics::RGBColor(0.1f, 0.5f, 0.6f));
		graphics::Attenuation attenuation1{ 0.5f, 0.25f, 0.2f };
		auto pointLight1 = std::make_unique<graphics::PointLight>(baseLight1, attenuation1, glm::vec3());
		auto pointLight2 = std::make_unique<graphics::PointLight>(baseLight1, attenuation1, glm::vec3());

		// RenderableTexts
		std::shared_ptr<graphics::Font> arial = nullptr;
		try {
			FileReader fileReader2("res/fonts/arial.fnt");
			arial = std::move(fontReader.load(fileReader2));
		}
		catch (std::exception& e) {
			Logger::writeLog(LogType::ERROR, e.what());
		}
		graphics::RenderableText renderableText1("First try rendering text", arial, 10, glm::vec2());

		// Renderable2Ds
		graphics::Renderable2D renderable2D1(glm::vec2(0.75f, 0.75f), glm::vec2(0.15f, 0.2f), texture1);
#endif		// GRAPHICS

		/*********************************************************************
		 * GAME DATA
		 *********************************************************************/
		//mGraphicsSystem->getLayer2D()->addRenderable2D(&renderable2D1);

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
				//std::make_unique<physics::BoundingBox>(glm::vec3(1,1,1)), glm::mat4()
				std::make_unique<physics::BoundingSphere>(0.5f), glm::mat4()
       		)
		);
		mGraphicsManager->addEntity(player.get(), std::move(camera1));

		mEntities.push_back(std::move(player));
		
		// Lights
		auto eL1 = std::make_unique<Entity>("PointLight1");
		eL1->mPosition = glm::vec3(2, 1, 5);
		mGraphicsManager->addEntity(eL1.get(), std::move(pointLight1));
		mEntities.push_back(std::move(eL1));

		auto eL2 = std::make_unique<Entity>("PointLight2");
		eL2->mPosition = glm::vec3(-3, 1, 5);
		mGraphicsManager->addEntity(eL2.get(), std::move(pointLight2));
		mEntities.push_back(std::move(eL2));

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
				//std::make_unique<physics::BoundingBox>(glm::vec3(1,1,1)), glm::mat4()
				std::make_unique<physics::BoundingSphere>(0.5f), glm::mat4()
			);
			if (i == 3) { physicsEntityCube->getRigidBody()->mAngularVelocity += glm::vec3(0, 10, 0); }
			if (i == 4) { physicsEntityCube->getRigidBody()->mLinearVelocity += glm::vec3(-1, 0, 0); }
			mPhysicsManager->addEntity(cube.get(), std::move(physicsEntityCube));
			
			auto renderable3D = std::make_unique<graphics::Renderable3D>(mesh1, fileMaterials[i], nullptr, false);
			mGraphicsManager->addEntity(cube.get(), std::move(renderable3D));

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
					//std::make_unique<physics::BoundingBox>(glm::vec3(1,1,1)), glm::mat4()
					std::make_unique<physics::BoundingSphere>(0.5f), glm::mat4()
				)
			);
			
			auto renderable3D = std::make_unique<graphics::Renderable3D>(mesh1, fileMaterials[4], nullptr, false);
			mGraphicsManager->addEntity(cube.get(), std::move(renderable3D));

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
			
			auto renderable3D = std::make_unique<graphics::Renderable3D>(*it, tmpMaterial, nullptr, false);
			mGraphicsManager->addEntity(building.get(), std::move(renderable3D));
			
			mEntities.push_back(std::move(building));
		}
#endif		// GAME

		/*********************************************************************
		 * MAIN LOOP
		 *********************************************************************/
		float lastTime = mWindowSystem->getTime();
		while ( !mEnd ) {
			// Calculate the elapsed time since the last update
			float curTime	= mWindowSystem->getTime();
			float deltaTime	= curTime - lastTime;

			if (deltaTime >= UPDATE_TIME) {
				lastTime = curTime;
				std::cout << deltaTime << "ms\r";

				// Update the Systems
				mWindowSystem->update();
				mEnd = mWindowSystem->getInputData()->mKeys[GLFW_KEY_ESCAPE];
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
