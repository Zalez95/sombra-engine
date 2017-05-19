#include "Game.h"
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "../utils/Logger.h"
#include "../utils/FileReader.h"

#include "../window/WindowSystem.h"

#include "../graphics/GraphicsSystem.h"
#include "../graphics/Texture.h"
#include "../graphics/2D/Renderable2D.h"
#include "../graphics/3D/Mesh.h"
#include "../graphics/3D/Material.h"
#include "../graphics/3D/Camera.h"
#include "../graphics/3D/Lights.h"
#include "../graphics/3D/Renderable3D.h"

#include "../loaders/MeshLoader.h"

namespace game {

// Static variables definition
    const float Game::FOV       = 45.0f;
    const float Game::Z_NEAR	= 1.0f;
    const float Game::Z_FAR		= 500.0f;

// Public functions
	Game::Game() : mEnd(false)
	{
		// Window
		if (!(mWindowSystem = new window::WindowSystem("< FAZE >", WIDTH, HEIGHT))) {
			Logger::writeLog(LogType::ERROR, "Error initializing the window system");
		}
		mWindowSystem->setMousePosition(WIDTH / (float)2, HEIGHT / (float)2);

		// Graphics
		if (!(mGraphicsSystem = new graphics::GraphicsSystem())) {
			Logger::writeLog(LogType::ERROR, "Error initializing the graphics system");
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
		 * GRAPHICS DATA
		 *********************************************************************/
		// Graphics Primitives
		graphics::MeshLoader meshLoader;
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
			FileReader fileReader("res/models/test.fzmsh");
			auto tmp = meshLoader.load(&fileReader);

			for (auto it = tmp.begin(); it != tmp.end(); ++it) {
				fileMeshes.push_back(std::move(*it));
			}
		}
		catch (std::exception& e) {
			Logger::writeLog(LogType::ERROR, e.what());
		}

		auto material1 = std::make_shared<graphics::Material>(
			"material1",
			graphics::RGBColor{ 0.2f, 1.0f, 0.2f },
			graphics::RGBColor{ 0.2f, 1.0f, 0.2f },
			graphics::RGBColor{ 0.1f, 1.0f, 0.1f },
			0.1f
		);

		auto material2 = std::make_shared<graphics::Material>(
			"material2",
			graphics::RGBColor{ 0.2f, 0.2f, 1.0f },
			graphics::RGBColor{ 0.2f, 0.2f, 1.0f },
			graphics::RGBColor{ 0.1f, 0.1f, 1.0f },
			0.1f
		);

		auto material3 = std::make_shared<graphics::Material>(
			"material3",
			graphics::RGBColor{ 1.0f, 0.2f, 0.2f },
			graphics::RGBColor{ 1.0f, 0.2f, 0.2f },
			graphics::RGBColor{ 1.0f, 0.1f, 0.1f },
			0.1f
		);

		auto material4 = std::make_shared<graphics::Material>(
			"material4",
			graphics::RGBColor{ 1.0f, 1.0f, 1.0f },
			graphics::RGBColor{ 1.0f, 1.0f, 1.0f },
			graphics::RGBColor{ 1.0f, 1.0f, 1.0f },
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

		// Renderable2Ds
		std::vector<const graphics::Renderable2D*> renderable2Ds;
		graphics::Renderable2D renderable2D1(glm::vec2(0.8f, 0.75f), glm::vec2(0.15f, 0.2f), texture1);
		renderable2Ds.push_back(&renderable2D1);

		// Renderable3Ds
		std::vector<const graphics::Renderable3D*> renderable3Ds;
		for (unsigned int i = 0; i < 500; ++i) {
			auto renderable3D1 = new graphics::Renderable3D(mesh1, material1, nullptr, false);
			renderable3D1->setModelMatrix(glm::translate(glm::vec3(
				100 * (static_cast<float>(rand()) / RAND_MAX) - 50,
				100 * (static_cast<float>(rand()) / RAND_MAX) - 50,
				100 * (static_cast<float>(rand()) / RAND_MAX) - 50)
			));
			renderable3Ds.push_back(renderable3D1);
		}

		graphics::Renderable3D* renderable3D_centro = new graphics::Renderable3D(mesh1, material4, nullptr, false);
		renderable3D_centro->setModelMatrix(glm::translate(glm::mat4(), glm::vec3(0, 0, -10)));
		renderable3Ds.push_back(renderable3D_centro);

		graphics::Renderable3D* renderable3D_derecha = new graphics::Renderable3D(mesh1, material3, nullptr, false);
		renderable3D_derecha->setModelMatrix(glm::translate(glm::mat4(), glm::vec3(2, 0, -10)));
		renderable3Ds.push_back(renderable3D_derecha);

		graphics::Renderable3D* renderable3D_arriba = new graphics::Renderable3D(mesh1, material1, nullptr, false);
		renderable3D_arriba->setModelMatrix(glm::translate(glm::mat4(), glm::vec3(0, 2, -10)));
		renderable3Ds.push_back(renderable3D_arriba);

		graphics::Renderable3D* renderable3D_frente = new graphics::Renderable3D(mesh1, material2, nullptr, false);
		renderable3D_frente->setModelMatrix(glm::translate(glm::mat4(), glm::vec3(0, 0, -8)));
		renderable3Ds.push_back(renderable3D_frente);

		std::vector<graphics::Renderable3D*> fileRenderables;
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
			fileRenderables.push_back(renderable3D1);
		}


		/*********************************************************************
		 * MAIN LOOP
		 *********************************************************************/
		float lastTime = mWindowSystem->getTime(), elapsed = lastTime;
		int fps = 0;
		while ( !mEnd ) {
			// Calculate delta (elapsed time)
			float curTime = mWindowSystem->getTime();
			float delta = curTime - lastTime;
			lastTime = curTime;

			// Update the FPSs
			fps++;
			if (lastTime - elapsed >= 1.0f) {
				elapsed = lastTime;
				std::cout << "FPS: " << fps << '\r' << std::flush;
				fps = 0;
			}

			input();
			update(delta);
			render(&camera1, renderable3Ds, renderable2Ds, pointLights);
		}

		for (const graphics::Renderable3D* r3d : renderable3Ds) {
			delete r3d;
		}

		return true;
	}

// Private functions
	void Game::input()
	{
		mWindowSystem->update();

		window::InputData inputData = mWindowSystem->getInputData();
		if (inputData.mKeys[GLFW_KEY_ESCAPE] || mWindowSystem->isClosed()) { mEnd = true; }
	}


	void Game::update(float delta)
	{
	}


	void Game::render(
		const graphics::Camera* camera,
		const std::vector<const graphics::Renderable3D*>& renderable3Ds,
		const std::vector<const graphics::Renderable2D*>& renderable2Ds,
		const std::vector<const graphics::PointLight*>& pointLights
	) {
		mGraphicsSystem->render(camera, renderable3Ds, renderable2Ds, pointLights);
		mWindowSystem->swapBuffers();
	}

}
