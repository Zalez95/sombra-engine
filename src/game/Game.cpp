#include "Game.h"
#include <iostream>
#include <chrono>
#include <thread>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>

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
#include "../physics/collision/BoundingSphere.h"
#include "../physics/collision/BoundingBox.h"

namespace game {

// Static variables definition
    const float Game::FOV       	= 45.0f;
    const float Game::Z_NEAR		= 1.0f;
    const float Game::Z_FAR			= 500.0f;
    const float Game::UPDATE_TIME	= 0.016f;

// Public functions
	Game::Game() : mEnd(false)
	{
		// Window
		if (!(mWindowSystem = new window::WindowSystem("< FAZE >", WIDTH, HEIGHT))) {
			Logger::writeLog(LogType::ERROR, "Error initializing the window system");
			return;
		}
		mWindowSystem->setMousePosition(WIDTH / (float)2, HEIGHT / (float)2);
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
		delete mWindowSystem;
		delete mGraphicsSystem;
	}


	bool Game::run()
	{
		/*********************************************************************
		 * LOADERS
		 *********************************************************************/
		graphics::MeshLoader meshLoader;
		graphics::FontLoader fontLoader;


		/*********************************************************************
		 * GRAPHICS DATA
		 *********************************************************************/
		// Graphics Primitives
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
		std::shared_ptr<graphics::Font> arial = nullptr;
		try {
			FileReader fileReader1("res/models/test.fzmsh");
			auto tmp = meshLoader.load(&fileReader1);

			for (auto it = tmp.begin(); it != tmp.end(); ++it) {
				fileMeshes.push_back(std::move(*it));
			}

			FileReader fileReader2("res/fonts/arial.fnt");
			arial = std::move(fontLoader.load(&fileReader2));
		}
		catch (std::exception& e) {
			Logger::writeLog(LogType::ERROR, e.what());
		}

		auto material_green = std::make_shared<graphics::Material>(
			"material_green",
			graphics::RGBColor{ 0.2f, 1.0f, 0.2f },
			graphics::RGBColor{ 0.2f, 1.0f, 0.2f },
			graphics::RGBColor{ 0.1f, 1.0f, 0.1f },
			0.1f
		);

		auto material_blue = std::make_shared<graphics::Material>(
			"material_blue",
			graphics::RGBColor{ 0.2f, 0.2f, 1.0f },
			graphics::RGBColor{ 0.2f, 0.2f, 1.0f },
			graphics::RGBColor{ 0.1f, 0.1f, 1.0f },
			0.1f
		);

		auto material_red = std::make_shared<graphics::Material>(
			"material_red",
			graphics::RGBColor{ 1.0f, 0.2f, 0.2f },
			graphics::RGBColor{ 1.0f, 0.2f, 0.2f },
			graphics::RGBColor{ 1.0f, 0.1f, 0.1f },
			0.1f
		);

		auto material_white = std::make_shared<graphics::Material>(
			"material_white",
			graphics::RGBColor{ 1.0f, 1.0f, 1.0f },
			graphics::RGBColor{ 1.0f, 1.0f, 1.0f },
			graphics::RGBColor{ 1.0f, 1.0f, 1.0f },
			0.1f
		);

		auto material_orange = std::make_shared<graphics::Material>(
			"material_orange",
			graphics::RGBColor{ 1.0f, 0.5f, 0.1f },
			graphics::RGBColor{ 0.2f, 0.2f, 0.1f },
			graphics::RGBColor{ 0.1f, 0.1f, 0.2f },
			0.1f
		);

		auto texture1 = std::make_shared<graphics::Texture>("res/images/test.png", GL_TEXTURE_2D);

		graphics::BaseLight baseLight1(4.0f, 24.0f);
		graphics::Attenuation attenuation1{ 0.5f, 0.25f, 0.2f };

		// Cameras
		graphics::Camera camera1(glm::vec3(0), glm::vec3(0,0,-1), glm::vec3(0,1,0));

		// Lights
		std::vector<const graphics::PointLight*> pointLights;
		graphics::PointLight pointLight1(baseLight1, attenuation1, glm::vec3(2, 1, 5));
		graphics::PointLight pointLight2(baseLight1, attenuation1, glm::vec3(-3, 1, 5));
		pointLights.push_back(&pointLight1);
		pointLights.push_back(&pointLight2);

		// RenderableTexts
		std::vector<const graphics::RenderableText*> renderableTexts;
		graphics::RenderableText renderableText1("First try rendering text", arial, 10, glm::vec2());
		renderableTexts.push_back(&renderableText1);

		// Renderable2Ds
		std::vector<const graphics::Renderable2D*> renderable2Ds;
		graphics::Renderable2D renderable2D1(glm::vec2(/*80.0f, 75.0f*/), glm::vec2(0.15f, 0.2f), texture1);
		renderable2Ds.push_back(&renderable2D1);

		// Renderable3Ds
		std::vector<const graphics::Renderable3D*> renderable3Ds;
		for (auto it = fileMeshes.begin(); it != fileMeshes.end(); ++it) {
			auto tmpMaterial = std::make_shared<graphics::Material>(
				"tmp_material",
				graphics::RGBColor{ static_cast<float>(rand()) / RAND_MAX, static_cast<float>(rand()) / RAND_MAX, static_cast<float>(rand()) / RAND_MAX },
				graphics::RGBColor{ static_cast<float>(rand()) / RAND_MAX, static_cast<float>(rand()) / RAND_MAX, static_cast<float>(rand()) / RAND_MAX },
				graphics::RGBColor{ static_cast<float>(rand()) / RAND_MAX, static_cast<float>(rand()) / RAND_MAX, static_cast<float>(rand()) / RAND_MAX },
				0.2f
			);
			graphics::Renderable3D* renderable3D1 = new graphics::Renderable3D(*it, tmpMaterial, nullptr, false);
			renderable3D1->setModelMatrix( glm::rotate(glm::mat4(), -glm::half_pi<float>(), glm::vec3(1,0,0)) );
			renderable3Ds.push_back(renderable3D1);
		}


		/*********************************************************************
		 * GAME DATA
		 *********************************************************************/
		std::vector<physics::PhysicsEntity*> physicsEntities;

		// Player
       	physics::PhysicsEntity* physicsEntityPlayer = new physics::PhysicsEntity(
       		std::make_unique<physics::RigidBody>(
       			40.0f, 0.01f,
       			2.0f / 5.0f * 10.0f * glm::pow(2.0f,2.0f) * glm::mat3(), 0.01f,
       			glm::vec3(0, 0, 10), glm::quat()
       		),
			std::make_unique<physics::BoundingBox>(glm::vec3(1,1,1)), glm::mat4()
       	);
		physicsEntities.push_back(physicsEntityPlayer);
		mPhysicsEngine->addPhysicsEntity(physicsEntityPlayer);

		auto player	= std::make_unique<Player>("player", physicsEntityPlayer, &camera1, nullptr, nullptr, glm::vec2(WIDTH, HEIGHT));
		mPlayer = player.get();
		mEntities.push_back(std::move(player));

		// Cubes
        glm::vec3 cubePositions[5] = { glm::vec3(-10, 0, -10), glm::vec3(0, 0, -10), glm::vec3(2, 0, -10), glm::vec3(0, 2, -10), glm::vec3(0, 0, -8) };
        std::shared_ptr<graphics::Material> materials[5] = { material_white, material_white, material_green, material_blue, material_orange };
        for (unsigned int i = 0; i < 5; ++i) {
        	physics::PhysicsEntity* physicsEntityCube = new physics::PhysicsEntity(
        		std::make_unique<physics::RigidBody>(
        			20.0f, 1.0f,
					2.0f / 5.0f * 10.0f * glm::pow(2.0f, 2.0f) * glm::mat3(), 0.5f,
        			cubePositions[i], glm::quat()
        		),
				std::make_unique<physics::BoundingBox>(glm::vec3(1,1,1)), glm::mat4()
        	);
			if (i == 0) { physicsEntityCube->getRigidBody()->addLinearVelocity(glm::vec3(1, 0, 0)); }
			if (i == 1) { physicsEntityCube->getRigidBody()->addAngularVelocity(glm::vec3(0, 10, 0)); }
        	physicsEntities.push_back(physicsEntityCube);
        	mPhysicsEngine->addPhysicsEntity(physicsEntityCube);

			graphics::Renderable3D* renderable3DCube = new graphics::Renderable3D(mesh1, materials[i], nullptr, false);
        	renderable3Ds.push_back(renderable3DCube);
        
			auto cube = std::make_unique<Entity>("not random cube", physicsEntityCube, nullptr, nullptr, renderable3DCube);
			mEntities.push_back(std::move(cube));
        }

		for (unsigned int i = 0; i < NUM_CUBES; ++i) {
			glm::vec3 randomPosition(
				100 * (static_cast<float>(rand()) / RAND_MAX) - 50,
				100 * (static_cast<float>(rand()) / RAND_MAX) - 50,
				100 * (static_cast<float>(rand()) / RAND_MAX) - 50
			);

			physics::PhysicsEntity* physicsEntityCube = new physics::PhysicsEntity(
				std::make_unique<physics::RigidBody>(
					10.0f, 1.0f,
					2.0f / 5.0f * 10.0f * glm::pow(2.0f,2.0f) * glm::mat3(), 0.05f,
					randomPosition, glm::quat()
				),
				std::make_unique<physics::BoundingBox>(glm::vec3(1,1,1)), glm::mat4()
			);
			physicsEntities.push_back(physicsEntityCube);
			mPhysicsEngine->addPhysicsEntity(physicsEntityCube);

			graphics::Renderable3D* renderable3DCube = new graphics::Renderable3D(mesh1, material_orange, nullptr, false);
			renderable3Ds.push_back(renderable3DCube);

			auto cube = std::make_unique<Entity>("random cube", physicsEntityCube, nullptr, nullptr, renderable3DCube);
			mEntities.push_back(std::move(cube));
		}


		/*********************************************************************
		 * MAIN LOOP
		 *********************************************************************/
		float lastTime = mWindowSystem->getTime();
		float timeSinceUpdate = 0;

		while ( !mEnd ) {
			// Calculate delta (elapsed time)
			float curTime	= mWindowSystem->getTime();
			float delta		= curTime - lastTime;
			lastTime		= curTime;
			timeSinceUpdate += delta;

			if (timeSinceUpdate >= UPDATE_TIME) {
				std::cout << delta << "ms\r";
			
				input(timeSinceUpdate);
				update(timeSinceUpdate);
				render(mPlayer->getCamera(), renderable2Ds, renderable3Ds, renderableTexts, pointLights);

				timeSinceUpdate = 0.0f;
			}
			else {
				std::this_thread::sleep_for( std::chrono::duration<float>(UPDATE_TIME - timeSinceUpdate) );
			}
		}

		for (const graphics::Renderable3D* r3d : renderable3Ds) {
			delete r3d;
		}

		for (const physics::PhysicsEntity* pe : physicsEntities) {
			delete pe;
		}

		return true;
	}

// Private functions
	void Game::input(float delta)
	{
		mWindowSystem->update();

		const window::InputData* inputData = mWindowSystem->getInputData();
		if (inputData->mKeys[GLFW_KEY_ESCAPE] || mWindowSystem->isClosed()) {
			mEnd = true;
		}
		else {
			if (mPlayer) { mPlayer->doInput(inputData, delta); }
			mWindowSystem->setMousePosition(WIDTH / (float)2, HEIGHT / (float)2);
		}
	}


	void Game::update(float delta)
	{
		mPhysicsEngine->update(delta);
		for (auto it = mEntities.begin(); it != mEntities.end(); ++it) {
			(*it)->synch();
		}
	}


	void Game::render(
		const graphics::Camera* camera,
		const std::vector<const graphics::Renderable2D*>& renderable2Ds,
		const std::vector<const graphics::Renderable3D*>& renderable3Ds,
		const std::vector<const graphics::RenderableText*>& renderableTexts,
		const std::vector<const graphics::PointLight*>& pointLights
	) {
		mGraphicsSystem->render(camera, renderable2Ds, renderable3Ds, renderableTexts, pointLights);
		mWindowSystem->swapBuffers();
	}

}
