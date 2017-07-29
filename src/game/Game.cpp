#include "Game.h"
#include <iostream>
#include <chrono>
#include <thread>

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/quaternion.hpp>

#include "InputComponent.h"
#include "PhysicsComponent.h"
#include "GraphicsComponent.h"

#include "../utils/Logger.h"
#include "../utils/FileReader.h"

#include "../window/WindowSystem.h"

#include "../graphics/GraphicsSystem.h"
#include "../graphics/Texture.h"
#include "../graphics/2D/Renderable2D.h"
#include "../graphics/text/Font.h"
#include "../graphics/text/RenderableText.h"
#include "../graphics/3D/Mesh.h"
#include "../graphics/3D/Material.h"
#include "../graphics/3D/Camera.h"
#include "../graphics/3D/Lights.h"
#include "../graphics/3D/Renderable3D.h"

#include "../loaders/MeshLoader.h"
#include "../loaders/FontLoader.h"

#include "../physics/PhysicsEngine.h"
#include "../physics/PhysicsEntity.h"
#include "../physics/RigidBody.h"
#include "../physics/collision/BoundingBox.h"
#include "../physics/collision/BoundingSphere.h"

#define GRAPHICS
#define GAME

namespace game {

// Static variables definition
    const float Game::UPDATE_TIME = 0.016f;

// Public functions
	Game::Game() : mEnd(false)
	{
		using namespace utils;

		// Window
		if (!(mWindowSystem = new window::WindowSystem("< FAZE >", WIDTH, HEIGHT))) {
			Logger::writeLog(LogType::ERROR, "Error initializing the window system");
			return;
		}
		mWindowSystem->setMousePosition(WIDTH / float(2), HEIGHT / float(2));
		std::cout << mWindowSystem->getGLInfo() << std::endl;

		// Graphics
		if (!(mGraphicsSystem = new graphics::GraphicsSystem())) {
			Logger::writeLog(LogType::ERROR, "Error initializing the graphics system");
			return;
		}

		// Physics
		if (!(mPhysicsEngine = new physics::PhysicsEngine())) {
			Logger::writeLog(LogType::ERROR, "Error initializing the physics engine");
			return;
		}
	}


	Game::~Game()
	{
		delete mPhysicsEngine;
		delete mGraphicsSystem;
		delete mWindowSystem;
	}


	bool Game::run()
	{
		using namespace utils;

#ifdef GRAPHICS
		/*********************************************************************
		 * GRAPHICS DATA
		 *********************************************************************/
		// Loaders
		loaders::MeshLoader meshLoader;
		loaders::FontLoader fontLoader;

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
		std::vector<GLfloat> normals = meshLoader.calculateNormals(positions, indices);
		std::shared_ptr<graphics::Mesh> mesh1 = std::move(meshLoader.createMesh("Cubo", positions, normals, std::vector<GLfloat>(16), indices));

		std::vector<std::shared_ptr<graphics::Mesh>> fileMeshes;
		try {
			FileReader fileReader1("res/models/test.fzmsh");
			auto loadedMeshes = meshLoader.load(fileReader1);
			for (std::unique_ptr<graphics::Mesh>& meshUPtr : loadedMeshes) {
				fileMeshes.push_back(std::move(meshUPtr));
			}
		}
		catch (std::exception& e) {
			Logger::writeLog(LogType::ERROR, e.what());
		}

		// Material
		auto material_red = std::make_shared<graphics::Material>(
			"material_red",
			graphics::RGBColor(1.0f, 0.2f, 0.2f),
			graphics::RGBColor(1.0f, 0.2f, 0.2f),
			graphics::RGBColor(1.0f, 0.1f, 0.1f),
			0.1f
		);

		auto material_green = std::make_shared<graphics::Material>(
			"material_green",
			graphics::RGBColor(0.2f, 1.0f, 0.2f),
			graphics::RGBColor(0.2f, 1.0f, 0.2f),
			graphics::RGBColor(0.1f, 1.0f, 0.1f),
			0.1f
		);

		auto material_blue = std::make_shared<graphics::Material>(
			"material_blue",
			graphics::RGBColor(0.2f, 0.2f, 1.0f),
			graphics::RGBColor(0.2f, 0.2f, 1.0f),
			graphics::RGBColor(0.1f, 0.1f, 1.0f),
			0.1f
		);

		auto material_white = std::make_shared<graphics::Material>(
			"material_white",
			graphics::RGBColor(1.0f, 1.0f, 1.0f),
			graphics::RGBColor(1.0f, 1.0f, 1.0f),
			graphics::RGBColor(1.0f, 1.0f, 1.0f),
			0.1f
		);

		auto material_orange = std::make_shared<graphics::Material>(
			"material_orange",
			graphics::RGBColor(1.0f, 0.5f, 0.1f),
			graphics::RGBColor(0.2f, 0.2f, 0.1f),
			graphics::RGBColor(0.1f, 0.1f, 0.2f),
			0.1f
		);

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
			arial = std::move(fontLoader.load(fileReader2));
		}
		catch (std::exception& e) {
			Logger::writeLog(LogType::ERROR, e.what());
		}
		graphics::RenderableText renderableText1("First try rendering text", arial, 10, glm::vec2());

		// Renderable2Ds
		graphics::Renderable2D renderable2D1(glm::vec2(0.75f, 0.75f), glm::vec2(0.15f, 0.2f), texture1);
#endif		// GRAPHICS

#ifdef GAME
		/*********************************************************************
		 * GAME DATA
		 *********************************************************************/
		mGraphicsSystem->getLayer2D()->addRenderable2D(&renderable2D1);

		// Player
       	auto pCompPlayer = std::make_unique<PhysicsComponent>(
			*mPhysicsEngine,
			std::make_unique<physics::PhysicsEntity>(
       			physics::RigidBody(
       				40.0f, 0.01f,
       				2.0f / 5.0f * 10.0f * glm::pow(2.0f,2.0f) * glm::mat3(), 0.01f
       			),
				std::make_unique<physics::BoundingBox>(glm::vec3(1,1,1)), glm::mat4()
       		)
		);
		auto iCompPlayer = std::make_unique<InputComponent>(*mWindowSystem);
		auto gCompPlayer = std::make_unique<GraphicsComponent>(*mGraphicsSystem, std::move(camera1), nullptr, nullptr);
		auto player	= std::make_unique<Entity>("player", std::move(iCompPlayer), std::move(pCompPlayer), std::move(gCompPlayer));
		player->mPosition = glm::vec3(0, 1, 10);
		player->update(0.0f);
		mEntities.push_back(std::move(player));

		// Buildings
		for (auto it = fileMeshes.begin(); it != fileMeshes.end(); ++it) {
			auto tmpMaterial = std::make_shared<graphics::Material>(
				"tmp_material",
				graphics::RGBColor( glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f) ),
				graphics::RGBColor( glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f) ),
				graphics::RGBColor( glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f) ),
				0.2f
			);
			auto renderable3D1 = std::make_unique<graphics::Renderable3D>(*it, tmpMaterial, nullptr, false);
			auto gCompBuilding = std::make_unique<GraphicsComponent>(*mGraphicsSystem, nullptr, nullptr, std::move(renderable3D1));

			auto building = std::make_unique<Entity>("player", nullptr, nullptr, std::move(gCompBuilding));
			building->mOrientation = glm::normalize(glm::quat(-1, glm::vec3(1, 0, 0)));
			building->update(0.0f);
			mEntities.push_back(std::move(building));
		}

		// Lights
		auto gCompL1 = std::make_unique<GraphicsComponent>(*mGraphicsSystem, nullptr, std::move(pointLight1), nullptr);
		auto eL1 = std::make_unique<Entity>("PointLight1", nullptr, nullptr, std::move(gCompL1));
		eL1->mPosition = glm::vec3(2, 1, 5);
		eL1->update(0.0f);
		mEntities.push_back(std::move(eL1));

		auto gCompL2 = std::make_unique<GraphicsComponent>(*mGraphicsSystem, nullptr, std::move(pointLight2), nullptr);
		auto eL2 = std::make_unique<Entity>("PointLight2", nullptr, nullptr, std::move(gCompL2));
		eL2->mPosition = glm::vec3(-3, 1, 5);
		eL2->update(0.0f);
		mEntities.push_back(std::move(eL2));

		// Cubes
		std::shared_ptr<graphics::Material> materials[5] = { material_red, material_green, material_blue, material_white, material_orange };
		glm::vec3 cubePositions[5] = { glm::vec3(2, 0, -10), glm::vec3(0, 2, -10), glm::vec3(0, 0, -8), glm::vec3(0, 0, -10), glm::vec3(10, 0, -10) };
		for (unsigned int i = 0; i < 5; ++i) {
			auto physicsEntityCube = std::make_unique<physics::PhysicsEntity>(
        		physics::RigidBody(
        			20.0f, 1.0f,
					2.0f / 5.0f * 10.0f * glm::pow(2.0f, 2.0f) * glm::mat3(), 0.5f
        		),
				std::make_unique<physics::BoundingBox>(glm::vec3(1,1,1)), glm::mat4()
			);
			if (i == 3) { physicsEntityCube->getRigidBody()->mAngularVelocity += glm::vec3(0, 10, 0); }
			if (i == 4) { physicsEntityCube->getRigidBody()->mLinearVelocity += glm::vec3(-1, 0, 0); }
        	auto pCompCube = std::make_unique<PhysicsComponent>(*mPhysicsEngine, std::move(physicsEntityCube));
		
			auto renderable3D1 = std::make_unique<graphics::Renderable3D>(mesh1, materials[i], nullptr, false);
			auto gCompCube = std::make_unique<GraphicsComponent>( *mGraphicsSystem, nullptr, nullptr, std::move(renderable3D1) );
        
			auto cube = std::make_unique<Entity>("not random cube", nullptr, std::move(pCompCube), std::move(gCompCube));
			cube->mPosition = cubePositions[i];
			cube->update(0.0f);
			mEntities.push_back(std::move(cube));
        }
		
		for (unsigned int i = 0; i < NUM_CUBES; ++i) {
			glm::vec3 randomPosition = glm::ballRand(50.0f);
			auto pCompCube = std::make_unique<PhysicsComponent>(
				*mPhysicsEngine,
				std::make_unique<physics::PhysicsEntity>(
					physics::RigidBody(
						10.0f, 1.0f,
						2.0f / 5.0f * 10.0f * glm::pow(2.0f,2.0f) * glm::mat3(), 0.05f
					),
					std::make_unique<physics::BoundingBox>(glm::vec3(1,1,1)), glm::mat4()
				)
			);

			auto renderable3D1 = std::make_unique<graphics::Renderable3D>(mesh1, material_orange, nullptr, false);
			auto gCompCube = std::make_unique<GraphicsComponent>(*mGraphicsSystem, nullptr, nullptr, std::move(renderable3D1));
			
			auto cube = std::make_unique<Entity>("random cube", nullptr, std::move(pCompCube), std::move(gCompCube));
			cube->mPosition = randomPosition;
			cube->update(0.0f);
			mEntities.push_back(std::move(cube));
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
				if (mWindowSystem->getInputData()->mKeys[GLFW_KEY_ESCAPE]) {
					// Stop the game loop if the escape key was pressed
					mEnd = true;
				}

				mPhysicsEngine->update(deltaTime);

				// Update the entities
				for (std::unique_ptr<Entity>& entity : mEntities) {
					entity->update(deltaTime);
				}

				// Draw
				mGraphicsSystem->render();
				mWindowSystem->swapBuffers();
			}
			else {
				std::this_thread::sleep_for( std::chrono::duration<float>(UPDATE_TIME - deltaTime) );
			}
		}

		// Delete the entities
		mEntities.clear();

		return true;
	}

}
